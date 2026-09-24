/*
 * STN-LABZ Rictus Core
 * Linux native module discovery.
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "rictus_artifact.h"
#include "rictus_module_discovery.h"

static int has_so_suffix(const char *name)
{
    size_t length;

    if (name == NULL) {
        return 0;
    }

    length = strlen(name);
    return length > 3U && strcmp(name + length - 3U, ".so") == 0;
}

rictus_module_result_t rictus_module_discovery_scan(
    const char *modules_path,
    rictus_module_candidate_t *candidates,
    size_t candidate_capacity,
    size_t *candidate_count,
    rictus_module_discovery_report_t *report)
{
    DIR *directory;
    struct dirent *entry;
    size_t count = 0U;

    if (modules_path == NULL || modules_path[0] == '\0' ||
        candidates == NULL || candidate_capacity == 0U ||
        candidate_count == NULL || report == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    memset(report, 0, sizeof(*report));
    *candidate_count = 0U;

    directory = opendir(modules_path);
    if (directory == NULL) {
        return RICTUS_MODULE_ERR_NOT_FOUND;
    }

    while ((entry = readdir(directory)) != NULL) {
        rictus_module_candidate_t *candidate;
        const char *name = entry->d_name;
        size_t name_length;
        int written;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        ++report->entries_examined;

        if (!has_so_suffix(name)) {
            ++report->candidates_rejected;
            continue;
        }

        name_length = strlen(name) - 3U;
        if (name_length == 0U || name_length >= RICTUS_MODULE_ID_MAX ||
            count >= candidate_capacity) {
            ++report->candidates_rejected;
            continue;
        }

        candidate = &candidates[count];
        memset(candidate, 0, sizeof(*candidate));
        memcpy(candidate->module_id, name, name_length);
        candidate->module_id[name_length] = '\0';

        written = snprintf(candidate->artifact_path,
                           sizeof(candidate->artifact_path),
                           "%s/%s", modules_path, name);
        if (written < 0 ||
            (size_t)written >= sizeof(candidate->artifact_path)) {
            ++report->candidates_rejected;
            continue;
        }

        if (rictus_artifact_sha256(candidate->artifact_path,
                                   candidate->artifact_id,
                                   sizeof(candidate->artifact_id)) !=
            RICTUS_ARTIFACT_OK) {
            ++report->candidates_rejected;
            continue;
        }

        ++count;
        ++report->candidates_found;
    }

    closedir(directory);
    *candidate_count = count;
    return RICTUS_MODULE_OK;
}
