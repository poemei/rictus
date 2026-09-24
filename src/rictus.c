#include <stdio.h>

#include "rictus.h"
#include "rictus_module_lifecycle.h"
#include "rictus_module_loader.h"
#include "rictus_module_registry.h"
#include "rictus_module_state.h"

#if defined(_WIN32)
#define RICTUS_IRC_MODULE_PATH "build\\windows\\modules\\irc.dll"
#else
#define RICTUS_IRC_MODULE_PATH "build/linux/modules/irc.so"
#endif

/*
 * Artifact hashing is the next boundary. This identity remains explicit and
 * temporary; persisted qualification is therefore valid only for this declared
 * identity and must not be mistaken for content-derived artifact proof.
 */
#define RICTUS_IRC_ARTIFACT_ID "runtime:irc:1.0.0"

int rictus_run(void)
{
    rictus_module_loader_t loader;
    rictus_module_registry_t registry;
    rictus_module_store_t state;
    const rictus_module_descriptor_t *descriptor = NULL;
    const rictus_module_record_t *record;
    rictus_module_loader_result_t load_result;
    rictus_module_state_result_t state_result;
    rictus_module_result_t module_result;
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

    load_result = rictus_module_loader_load(
        &loader,
        "irc",
        RICTUS_IRC_MODULE_PATH,
        &descriptor);

    if (load_result != RICTUS_MODULE_LOADER_OK) {
        fprintf(stderr, "[ERROR] IRC module load: %s\n",
                rictus_module_loader_result_string(load_result));
        return 1;
    }

    printf("[INFO] Module loaded: %s %u.%u.%u\n",
           descriptor->name,
           descriptor->version_major,
           descriptor->version_minor,
           descriptor->version_patch);

    module_result = rictus_module_lifecycle_prepare(
        &registry,
        &state.inventory,
        descriptor,
        RICTUS_IRC_ARTIFACT_ID);

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

    printf("[INFO] IRC module qualified: %u/%u tests passed.\n",
           record->qualification.tests_passed,
           record->qualification.tests_executed);

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
