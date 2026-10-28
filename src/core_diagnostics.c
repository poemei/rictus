#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdatomic.h>
#include <stdio.h>\n#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "core_diagnostics.h"

typedef struct {
    char name[STNLABZ_MODULE_SERVICE_NAME_MAX];
    char owner[STNLABZ_MODULE_ID_MAX];
    stnlabz_module_result_t result;
} rictus_diag_service_t;

typedef struct {
    int valid;
    char sender[STNLABZ_MODULE_COMMAND_SENDER_MAX];
    char account[STNLABZ_MODULE_COMMAND_ACCOUNT_MAX];
    char text[RICTUS_DIAG_TEXT_MAX];
    char command[STNLABZ_MODULE_COMMAND_NAME_MAX];
    char command_owner[STNLABZ_MODULE_ID_MAX];
    stnlabz_module_result_t command_result;
    int command_seen;
    int replied;
    rictus_diag_service_t services[RICTUS_DIAG_SERVICE_MAX];
    size_t service_count;
} rictus_diag_trace_t;

static rictus_diag_trace_t trace;
static atomic_flag trace_lock = ATOMIC_FLAG_INIT;

static void lock_trace(void)
{
    while (atomic_flag_test_and_set_explicit(&trace_lock, memory_order_acquire)) {}
}

static void unlock_trace(void)
{
    atomic_flag_clear_explicit(&trace_lock, memory_order_release);
}

static void copy_text(char *dst, size_t dst_size, const char *src)
{
    size_t length;
    if (!dst || !dst_size) return;
    dst[0] = '\0';
    if (!src) return;
    length = strlen(src);
    if (length >= dst_size) length = dst_size - 1U;
    memcpy(dst, src, length);
    dst[length] = '\0';
}

void rictus_diag_begin(const char *sender, const char *account, const char *text)
{
    lock_trace();
    memset(&trace, 0, sizeof trace);
    trace.valid = 1;
    copy_text(trace.sender, sizeof trace.sender, sender);
    copy_text(trace.account, sizeof trace.account, account);
    copy_text(trace.text, sizeof trace.text, text);
    unlock_trace();
}

void rictus_diag_route(const char *command)
{
    lock_trace();
    if (trace.valid) copy_text(trace.command, sizeof trace.command, command);
    unlock_trace();
}

void rictus_diag_command(const char *command, const char *owner,
                         stnlabz_module_result_t result, int replied)
{
    lock_trace();
    if (trace.valid) {
        if (command && *command)
            copy_text(trace.command, sizeof trace.command, command);
        copy_text(trace.command_owner, sizeof trace.command_owner,
                  owner && *owner ? owner : "core/unknown");
        trace.command_result = result;
        trace.command_seen = 1;
        trace.replied = replied ? 1 : 0;
    }
    unlock_trace();
}

void rictus_diag_service(const char *service, const char *owner,
                         stnlabz_module_result_t result)
{
    rictus_diag_service_t *item;
    lock_trace();
    if (!trace.valid || trace.service_count >= RICTUS_DIAG_SERVICE_MAX) {
        unlock_trace();
        return;
    }
    item = &trace.services[trace.service_count++];
    copy_text(item->name, sizeof item->name, service);
    copy_text(item->owner, sizeof item->owner,
              owner && *owner ? owner : "core/unknown");
    item->result = result;
    unlock_trace();
}

static int appendf(char *output, size_t output_size, size_t *used,
                   const char *format, ...)
{
    va_list ap;
    int written;
    if (!output || !used || *used >= output_size) return 0;
    va_start(ap, format);
    written = vsnprintf(output + *used, output_size - *used, format, ap);
    va_end(ap);
    if (written < 0 || (size_t)written >= output_size - *used) return 0;
    *used += (size_t)written;
    return 1;
}

static int source_registration(const char *owner, const char *needle,
                               char *path, size_t path_size,
                               unsigned long *line_out)
{
    const char *root = getenv("RICTUS_SOURCE_ROOT");
    char directory_path[512];
    DIR *directory;
    struct dirent *entry;
    if (!owner || !*owner || !needle || !*needle || !path || !line_out)
        return 0;
    if (!root || !*root) root = "/home/stnchain/rictus";
    if (snprintf(directory_path, sizeof directory_path, "%s/modules/%s/src",
                 root, owner) >= (int)sizeof directory_path)
        return 0;
    directory = opendir(directory_path);
    if (!directory) return 0;

    while ((entry = readdir(directory)) != NULL) {
        char full_path[768];
        FILE *file;
        char line[2048];
        unsigned long number = 0UL;
        size_t name_length = strlen(entry->d_name);
        if (entry->d_name[0] == '.' || name_length < 3U ||
            strcmp(entry->d_name + name_length - 2U, ".c") != 0)
            continue;
        if (snprintf(full_path, sizeof full_path, "%s/%s",
                     directory_path, entry->d_name) >= (int)sizeof full_path)
            continue;
        file = fopen(full_path, "r");
        if (!file) continue;
        while (fgets(line, sizeof line, file)) {
            ++number;
            if (strstr(line, needle)) {
                if (snprintf(path, path_size, "modules/%s/src/%s",
                             owner, entry->d_name) < (int)path_size) {
                    *line_out = number;
                    fclose(file);
                    closedir(directory);
                    return 1;
                }
            }
        }
        fclose(file);
    }
    closedir(directory);
    return 0;
}

int rictus_diag_render(char *output, size_t output_size)
{
    rictus_diag_trace_t snapshot;
    size_t used = 0U;
    size_t i;
    char source_path[512];
    unsigned long source_line = 0UL;
    const char *source_needle = NULL;

    if (!output || output_size < 2U) return 0;
    output[0] = '\0';

    lock_trace();
    snapshot = trace;
    unlock_trace();

    if (!snapshot.valid)
        return appendf(output, output_size, &used,
                       "DSD: no prior request trace is available.");

    if (!appendf(output, output_size, &used,
                 "DSD last request\n"
                 "Request: %s\n"
                 "Sender: %s\n"
                 "Route: %s\n",
                 snapshot.text[0] ? snapshot.text : "(empty)",
                 snapshot.sender[0] ? snapshot.sender : "(unknown)",
                 snapshot.command[0] ? snapshot.command : "(unresolved)"))
        return 0;

    if (snapshot.command_seen) {
        if (!appendf(output, output_size, &used,
                     "Controller owner: %s\n"
                     "Controller result: %s\n"
                     "Reply produced: %s\n",
                     snapshot.command_owner[0] ? snapshot.command_owner : "(unknown)",
                     stnlabz_module_result_string(snapshot.command_result),
                     snapshot.replied ? "yes" : "no"))
            return 0;
    } else {
        if (!appendf(output, output_size, &used,
                     "Controller: not reached\n"))
            return 0;
    }

    for (i = 0U; i < snapshot.service_count; ++i) {
        if (!appendf(output, output_size, &used,
                     "Service[%zu]: %s owner=%s result=%s\n",
                     i + 1U,
                     snapshot.services[i].name,
                     snapshot.services[i].owner,
                     stnlabz_module_result_string(snapshot.services[i].result)))
            return 0;
    }

    if (snapshot.command_owner[0] &&
        strcmp(snapshot.command_owner, "core/unknown") != 0 &&
        snapshot.command[0]) {
        char literal[STNLABZ_MODULE_COMMAND_NAME_MAX + 3U];
        if (snprintf(literal, sizeof literal, "\"%s\"", snapshot.command) <
            (int)sizeof literal)
            source_needle = literal;
        if (source_needle &&
            source_registration(snapshot.command_owner, source_needle,
                                source_path, sizeof source_path, &source_line)) {
            if (!appendf(output, output_size, &used,
                         "Code: %s:%lu (route registration evidence)\n",
                         source_path, source_line))
                return 0;
        }
    }

    if (!snapshot.command_seen ||
        snapshot.command_result != STNLABZ_MODULE_OK) {
        if (!appendf(output, output_size, &used,
                     "Disposition: failed before successful controller completion."))
            return 0;
    } else if (!snapshot.replied && snapshot.service_count == 0U) {
        if (!appendf(output, output_size, &used,
                     "Disposition: controller completed without reply or recorded service action."))
            return 0;
    } else {
        if (!appendf(output, output_size, &used,
                     "Disposition: request reached its controller; inspect recorded services/results above."))
            return 0;
    }
    return 1;
}
