/*
 * STN-LABZ Rictus Core
 * Linux native module discovery.
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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

static int is_directory(const char *path)
{
    struct stat status;

    if (path == NULL || stat(path, &status) != 0) {
        return 0;
    }

    return S_ISDIR(status.st_mode) != 0;
}

static int prepare_candidate(
    const char *module_id,
    const char *artifact_path,
    rictus_module_candidate_t *candidate)
{
    size_t id_length;
    int written;

    if (module_id == NULL || artifact_path == NULL || candidate == NULL) {
        return 0;
    }

    id_length = strlen(module_id);
    if (id_length == 0U || id_length >= RICTUS_MODULE_ID_MAX) {
        return 0;
    }

    memset(candidate, 0, sizeof(*candidate));
    memcpy(candidate->module_id, module_id, id_length);
    candidate->module_id[id_length] = '\0';

    written = snprintf(candidate->artifact_path,
                       sizeof(candidate->artifact_path),
                       "%s", artifact_path);
    if (written < 0 ||
        (size_t)written >= sizeof(candidate->artifact_path)) {
        return 0;
    }

    if (rictus_artifact_sha256(candidate->artifact_path,
                               candidate->artifact_id,
                               sizeof(candidate->artifact_id)) !=
        RICTUS_ARTIFACT_OK) {
        return 0;
    }

    return 1;
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
        const char *name = entry->d_name;
        char entry_path[RICTUS_MODULE_DISCOVERY_PATH_MAX];
        char artifact_path[RICTUS_MODULE_DISCOVERY_PATH_MAX];
        char module_id[RICTUS_MODULE_ID_MAX];
        size_t name_length;
        int written;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        ++report->entries_examined;

        written = snprintf(entry_path,
                           sizeof(entry_path),
                           "%s/%s", modules_path, name);
        if (written < 0 || (size_t)written >= sizeof(entry_path)) {
            ++report->candidates_rejected;
            continue;
        }

        if (has_so_suffix(name)) {
            name_length = strlen(name) - 3U;
            if (name_length == 0U || name_length >= sizeof(module_id)) {
                ++report->candidates_rejected;
                continue;
            }

            memcpy(module_id, name, name_length);
            module_id[name_length] = '\0';

            if (count >= candidate_capacity ||
                !prepare_candidate(module_id,
                                   entry_path,
                                   &candidates[count])) {
                ++report->candidates_rejected;
                continue;
            }
        } else if (is_directory(entry_path)) {
            name_length = strlen(name);
            if (name_length == 0U || name_length >= sizeof(module_id)) {
                ++report->candidates_rejected;
                continue;
            }

            memcpy(module_id, name, name_length + 1U);

            written = snprintf(artifact_path,
                               sizeof(artifact_path),
                               "%s/%s.so", entry_path, module_id);
            if (written < 0 ||
                (size_t)written >= sizeof(artifact_path) ||
                count >= candidate_capacity ||
                !prepare_candidate(module_id,
                                   artifact_path,
                                   &candidates[count])) {
                ++report->candidates_rejected;
                continue;
            }
        } else {
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
