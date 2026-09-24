/*
 * STN-LABZ Rictus Core
 * Linux native hot-deployment observation.
 */

#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "rictus_module_watch.h"

#define RICTUS_WATCH_BUFFER 4096

static const rictus_module_candidate_t *find_candidate(
    const rictus_module_candidate_t *candidates,
    size_t count,
    const char *module_id)
{
    size_t index;

    for (index = 0U; index < count; ++index) {
        if (strcmp(candidates[index].module_id, module_id) == 0) {
            return &candidates[index];
        }
    }

    return NULL;
}

static void observe_scan(rictus_module_watch_t *watch)
{
    rictus_module_candidate_t current[RICTUS_MODULE_LOADER_MAX];
    rictus_module_discovery_report_t report;
    size_t current_count = 0U;
    size_t index;

    if (rictus_module_discovery_scan(
            watch->modules_path,
            current,
            RICTUS_MODULE_LOADER_MAX,
            &current_count,
            &report) != RICTUS_MODULE_OK) {
        fputs("[ERROR] Module watch rescan failed.\n", stderr);
        return;
    }

    for (index = 0U; index < current_count; ++index) {
        const rictus_module_candidate_t *previous =
            find_candidate(watch->known, watch->known_count,
                           current[index].module_id);

        if (previous == NULL) {
            printf("[INFO] Module deployment detected: %s artifact=%.*s...\n",
                   current[index].module_id, 12, current[index].artifact_id);
        } else if (strcmp(previous->artifact_id,
                          current[index].artifact_id) != 0) {
            printf("[INFO] Module artifact changed: %s artifact=%.*s...\n",
                   current[index].module_id, 12, current[index].artifact_id);
        }
    }

    memcpy(watch->known, current, sizeof(current));
    watch->known_count = current_count;
}

static void *watch_main(void *argument)
{
    rictus_module_watch_t *watch = argument;
    int descriptor;
    int watch_descriptor;
    unsigned char buffer[RICTUS_WATCH_BUFFER];

    descriptor = inotify_init1(IN_CLOEXEC);
    if (descriptor < 0) {
        fputs("[ERROR] Module watch initialization failed.\n", stderr);
        return NULL;
    }

    watch_descriptor = inotify_add_watch(
        descriptor,
        watch->modules_path,
        IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_DELETE);
    if (watch_descriptor < 0) {
        close(descriptor);
        fputs("[ERROR] Module directory watch failed.\n", stderr);
        return NULL;
    }

    puts("[INFO] Module hot-deployment watch active.");

    while (watch->running) {
        ssize_t received = read(descriptor, buffer, sizeof(buffer));

        if (received < 0) {
            break;
        }

        if (received > 0) {
            observe_scan(watch);
        }
    }

    inotify_rm_watch(descriptor, watch_descriptor);
    close(descriptor);
    return NULL;
}

int rictus_module_watch_start(
    rictus_module_watch_t *watch,
    const char *modules_path,
    const rictus_module_candidate_t *initial,
    size_t initial_count)
{
    size_t path_length;

    if (watch == NULL || modules_path == NULL || initial == NULL ||
        initial_count > RICTUS_MODULE_LOADER_MAX) {
        return 0;
    }

    path_length = strlen(modules_path);
    if (path_length == 0U || path_length >= sizeof(watch->modules_path)) {
        return 0;
    }

    memset(watch, 0, sizeof(*watch));
    memcpy(watch->modules_path, modules_path, path_length + 1U);
    memcpy(watch->known, initial,
           initial_count * sizeof(rictus_module_candidate_t));
    watch->known_count = initial_count;
    watch->running = 1;

    if (pthread_create(&watch->thread, NULL, watch_main, watch) != 0) {
        watch->running = 0;
        return 0;
    }

    (void)pthread_detach(watch->thread);
    return 1;
}
