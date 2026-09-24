#include <stdio.h>
#include <string.h>

#include "rictus.h"
#include "rictus_artifact.h"
#include "rictus_module_discovery.h"
#include "rictus_module_lifecycle.h"
#include "rictus_module_loader.h"
#include "rictus_module_registry.h"
#include "rictus_module_state.h"
#include "rictus_module_watch.h"

#define RICTUS_MODULES_PATH "build/linux/modules"

int rictus_run(void)
{
    rictus_module_loader_t loader;
    rictus_module_registry_t registry;
    rictus_module_store_t state;
    rictus_module_watch_t watch;
    const rictus_module_descriptor_t *descriptor = NULL;
    const rictus_module_record_t *record;
    rictus_module_candidate_t candidates[RICTUS_MODULE_LOADER_MAX];
    rictus_module_discovery_report_t discovery_report;
    const rictus_module_candidate_t *irc_candidate = NULL;
    size_t candidate_count = 0U;
    size_t candidate_index;
    rictus_module_loader_result_t load_result;
    rictus_module_state_result_t state_result;
    rictus_module_result_t module_result;
    rictus_module_prepare_action_t prepare_action;
    rictus_module_host_t host = {0};
    int state_existed;

    puts("STN-LABZ Rictus");

    rictus_module_loader_init(&loader);
    rictus_module_registry_init(&registry);
    rictus_module_state_init(&state);

    state_result = rictus_module_state_load(
        &state, RICTUS_MODULE_STATE_PATH);
    state_existed = state_result == RICTUS_MODULE_STATE_OK;

    if (state_result != RICTUS_MODULE_STATE_OK &&
        state_result != RICTUS_MODULE_STATE_NOT_FOUND) {
        fprintf(stderr, "[ERROR] Module state load: %s\n",
                rictus_module_state_result_string(state_result));
        return 1;
    }

    module_result = rictus_module_discovery_scan(
        RICTUS_MODULES_PATH,
        candidates,
        RICTUS_MODULE_LOADER_MAX,
        &candidate_count,
        &discovery_report);

    if (module_result != RICTUS_MODULE_OK) {
        fprintf(stderr, "[ERROR] Module discovery: %s\n",
                rictus_module_result_string(module_result));
        return 1;
    }

    printf("[INFO] Module discovery: %zu candidate(s), %zu rejected.\n",
           discovery_report.candidates_found,
           discovery_report.candidates_rejected);

    for (candidate_index = 0U;
         candidate_index < candidate_count;
         ++candidate_index) {
        if (strcmp(candidates[candidate_index].module_id, "irc") == 0) {
            irc_candidate = &candidates[candidate_index];
            break;
        }
    }

    if (irc_candidate == NULL) {
        fputs("[ERROR] IRC module not discovered.\n", stderr);
        return 1;
    }

    load_result = rictus_module_loader_load(
        &loader,
        irc_candidate->module_id,
        irc_candidate->artifact_path,
        &descriptor);

    if (load_result != RICTUS_MODULE_LOADER_OK) {
        fprintf(stderr, "[ERROR] IRC module load: %s\n",
                rictus_module_loader_result_string(load_result));
        return 1;
    }

    printf("[INFO] Module loaded: %s %u.%u.%u artifact=%.*s...\n",
           descriptor->name,
           descriptor->version_major,
           descriptor->version_minor,
           descriptor->version_patch,
           12,
           irc_candidate->artifact_id);

    module_result = rictus_module_lifecycle_prepare(
        &registry,
        &state.inventory,
        descriptor,
        irc_candidate->artifact_id,
        &prepare_action);

    if (module_result != RICTUS_MODULE_OK) {
        fprintf(stderr, "[ERROR] IRC module prepare: %s\n",
                rictus_module_result_string(module_result));
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    record = rictus_module_registry_find(&registry, descriptor->id);
    if (record == NULL) {
        fputs("[ERROR] IRC module missing from Core registry after prepare.\n",
              stderr);
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    if (prepare_action == RICTUS_MODULE_PREPARE_RESTORED) {
        printf("[INFO] IRC module qualification restored: %u/%u tests passed.\n",
               record->qualification.tests_passed,
               record->qualification.tests_executed);
    } else {
        printf("[INFO] IRC module qualified: %u/%u tests passed.\n",
               record->qualification.tests_passed,
               record->qualification.tests_executed);
    }

    /*
     * First state creation preserves the currently established operator
     * decision: starting Rictus means IRC is enabled. After that point the
     * persisted Core-owned authorization record is authoritative.
     */
    if (!state_existed) {
        state_result = rictus_module_state_set_enabled(
            &state, descriptor->id, 1);
        if (state_result != RICTUS_MODULE_STATE_OK) {
            fprintf(stderr, "[ERROR] IRC authorization state: %s\n",
                    rictus_module_state_result_string(state_result));
            rictus_module_loader_unload_all(&loader);
            return 1;
        }
    }

    state_result = rictus_module_state_save(
        &state, RICTUS_MODULE_STATE_PATH);
    if (state_result != RICTUS_MODULE_STATE_OK) {
        fprintf(stderr, "[ERROR] Module state save: %s\n",
                rictus_module_state_result_string(state_result));
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    if (!rictus_module_state_enabled(&state, descriptor->id)) {
        puts("[INFO] IRC module qualified and disabled by human Core policy.");
        rictus_module_loader_unload_all(&loader);
        return 0;
    }

    module_result = rictus_module_lifecycle_enable(
        &registry,
        descriptor->id,
        RICTUS_MODULE_AUTHORITY_HUMAN);

    if (module_result != RICTUS_MODULE_OK) {
        fprintf(stderr, "[ERROR] IRC module enable: %s\n",
                rictus_module_result_string(module_result));
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    puts("[INFO] IRC module enabled by persisted human Core authority.");

    if (!rictus_module_watch_start(
            &watch,
            RICTUS_MODULES_PATH,
            candidates,
            candidate_count,
            &loader,
            &registry,
            &state)) {
        fputs("[ERROR] Module hot-deployment watch failed.\n", stderr);
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    module_result = descriptor->start(&host);
    if (module_result != RICTUS_MODULE_OK) {
        fprintf(stderr, "[ERROR] IRC module start: %s\n",
                rictus_module_result_string(module_result));
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    rictus_module_loader_unload_all(&loader);
    return 0;
}
