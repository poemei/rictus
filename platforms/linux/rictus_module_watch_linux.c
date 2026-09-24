/*
 * STN-LABZ Rictus Core
 * Linux native hot-deployment observation and qualification.
 */

#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "rictus_module_lifecycle.h"
#include "rictus_module_watch.h"

#define RICTUS_WATCH_BUFFER 4096

static const rictus_module_candidate_t *find_candidate(
    const rictus_module_candidate_t *candidates,
    size_t count,
    const char *module_id)
{
    size_t index;
    for (index = 0U; index < count; ++index)
        if (strcmp(candidates[index].module_id, module_id) == 0)
            return &candidates[index];
    return NULL;
}

static void prepare_deployment(
    rictus_module_watch_t *watch,
    const rictus_module_candidate_t *candidate)
{
    const rictus_module_descriptor_t *descriptor = NULL;
    const rictus_module_record_t *record;
    rictus_module_loader_result_t load_result;
    rictus_module_result_t result;
    rictus_module_prepare_action_t action;
    rictus_module_state_result_t state_result;

    load_result = rictus_module_loader_load(
        watch->loader, candidate->module_id, candidate->artifact_path,
        &descriptor);
    if (load_result != RICTUS_MODULE_LOADER_OK) {
        fprintf(stderr, "[ERROR] Module deployment load: %s result=%s\n",
                candidate->module_id,
                rictus_module_loader_result_string(load_result));
        return;
    }

    result = rictus_module_lifecycle_prepare(
        watch->registry, &watch->state->inventory, descriptor,
        candidate->artifact_id, &action);
    if (result != RICTUS_MODULE_OK) {
        fprintf(stderr, "[ERROR] Module deployment prepare: %s result=%s\n",
                candidate->module_id, rictus_module_result_string(result));
        (void)rictus_module_loader_unload(watch->loader, candidate->module_id);
        return;
    }

    record = rictus_module_registry_find(watch->registry, descriptor->id);
    if (record == NULL) {
        fprintf(stderr, "[ERROR] Module deployment registry missing: %s\n",
                candidate->module_id);
        (void)rictus_module_loader_unload(watch->loader, candidate->module_id);
        return;
    }

    state_result = rictus_module_state_set_enabled(
        watch->state, descriptor->id, 0);
    if (state_result != RICTUS_MODULE_STATE_OK) {
        fprintf(stderr, "[ERROR] Module deployment disable state: %s result=%s\n",
                candidate->module_id,
                rictus_module_state_result_string(state_result));
        (void)rictus_module_loader_unload(watch->loader, candidate->module_id);
        return;
    }

    state_result = rictus_module_state_save(
        watch->state, RICTUS_MODULE_STATE_PATH);
    if (state_result != RICTUS_MODULE_STATE_OK) {
        fprintf(stderr, "[ERROR] Module deployment state save: %s result=%s\n",
                candidate->module_id,
                rictus_module_state_result_string(state_result));
        (void)rictus_module_loader_unload(watch->loader, candidate->module_id);
        return;
    }

    if (action == RICTUS_MODULE_PREPARE_RESTORED) {
        printf("[INFO] Module qualification restored: %s %u/%u tests passed; DISABLED pending human authority.\n",
               descriptor->id, record->qualification.tests_passed,
               record->qualification.tests_executed);
    } else {
        printf("[INFO] Module qualified: %s %u/%u tests passed; DISABLED pending human authority.\n",
               descriptor->id, record->qualification.tests_passed,
               record->qualification.tests_executed);
    }
}

static void observe_scan(rictus_module_watch_t *watch)
{
    rictus_module_candidate_t current[RICTUS_MODULE_LOADER_MAX];
    rictus_module_discovery_report_t report;
    size_t current_count = 0U;
    size_t index;

    if (rictus_module_discovery_scan(
            watch->modules_path, current, RICTUS_MODULE_LOADER_MAX,
            &current_count, &report) != RICTUS_MODULE_OK) {
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
            prepare_deployment(watch, &current[index]);
        } else if (strcmp(previous->artifact_id,
                          current[index].artifact_id) != 0) {
            const rictus_loaded_module_t *loaded;
            const rictus_module_record_t *record;
            rictus_module_result_t stop_result;

            printf("[INFO] Module artifact changed: %s artifact=%.*s...\n",
                   current[index].module_id, 12, current[index].artifact_id);

            /*
             * IRC owns the transport carrying operator control and is not
             * replaced from its own watcher thread.
             */
            if (strcmp(current[index].module_id, "irc") == 0) {
                puts("[INFO] IRC artifact change deferred until Core restart.");
                continue;
            }

            loaded = rictus_module_loader_find(
                watch->loader, current[index].module_id);
            record = rictus_module_registry_find(
                watch->registry, current[index].module_id);

            if (loaded != NULL && record != NULL &&
                record->state == RICTUS_MODULE_STATE_ACTIVE) {
                if (loaded->descriptor->stop == NULL) {
                    fprintf(stderr,
                            "[ERROR] Module replacement stop unavailable: %s\n",
                            current[index].module_id);
                    continue;
                }

                stop_result = loaded->descriptor->stop();
                if (stop_result != RICTUS_MODULE_OK) {
                    fprintf(stderr,
                            "[ERROR] Module replacement stop: %s result=%s\n",
                            current[index].module_id,
                            rictus_module_result_string(stop_result));
                    continue;
                }

                stop_result = rictus_module_registry_stop(
                    watch->registry, current[index].module_id);
                if (stop_result != RICTUS_MODULE_OK) {
                    fprintf(stderr,
                            "[ERROR] Module replacement registry stop: %s result=%s\n",
                            current[index].module_id,
                            rictus_module_result_string(stop_result));
                    continue;
                }

                printf("[INFO] Module stopped for artifact replacement: %s\n",
                       current[index].module_id);
            }

            if (loaded != NULL) {
                rictus_module_loader_result_t unload_result =
                    rictus_module_loader_unload(
                        watch->loader, current[index].module_id);
                if (unload_result != RICTUS_MODULE_LOADER_OK) {
                    fprintf(stderr,
                            "[ERROR] Module replacement unload: %s result=%s\n",
                            current[index].module_id,
                            rictus_module_loader_result_string(unload_result));
                    continue;
                }
                printf("[INFO] Previous module artifact unloaded: %s\n",
                       current[index].module_id);
            }

            /*
             * prepare_deployment() loads the new image, verifies ABI,
             * performs qualification for the changed artifact, records its
             * evidence, and persists DISABLED. Replacement never restores
             * prior operational authority.
             */
            prepare_deployment(watch, &current[index]);
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
        descriptor, watch->modules_path,
        IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_DELETE);
    if (watch_descriptor < 0) {
        close(descriptor);
        fputs("[ERROR] Module directory watch failed.\n", stderr);
        return NULL;
    }

    puts("[INFO] Module hot-deployment watch active.");

    while (watch->running) {
        ssize_t received = read(descriptor, buffer, sizeof(buffer));
        if (received < 0) break;
        if (received > 0) observe_scan(watch);
    }

    inotify_rm_watch(descriptor, watch_descriptor);
    close(descriptor);
    return NULL;
}

int rictus_module_watch_start(
    rictus_module_watch_t *watch,
    const char *modules_path,
    const rictus_module_candidate_t *initial,
    size_t initial_count,
    rictus_module_loader_t *loader,
    rictus_module_registry_t *registry,
    rictus_module_store_t *state)
{
    size_t path_length;

    if (watch == NULL || modules_path == NULL || initial == NULL ||
        loader == NULL || registry == NULL || state == NULL ||
        initial_count > RICTUS_MODULE_LOADER_MAX) return 0;

    path_length = strlen(modules_path);
    if (path_length == 0U || path_length >= sizeof(watch->modules_path))
        return 0;

    memset(watch, 0, sizeof(*watch));
    memcpy(watch->modules_path, modules_path, path_length + 1U);
    memcpy(watch->known, initial,
           initial_count * sizeof(rictus_module_candidate_t));
    watch->known_count = initial_count;
    watch->loader = loader;
    watch->registry = registry;
    watch->state = state;
    watch->running = 1;

    if (pthread_create(&watch->thread, NULL, watch_main, watch) != 0) {
        watch->running = 0;
        return 0;
    }

    (void)pthread_detach(watch->thread);
    return 1;
}
