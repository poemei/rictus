#include <stdio.h>

#include "rictus.h"
#include "rictus_module_inventory.h"
#include "rictus_module_lifecycle.h"
#include "rictus_module_loader.h"
#include "rictus_module_registry.h"

#if defined(_WIN32)
#define RICTUS_IRC_MODULE_PATH "build\\windows\\modules\\irc.dll"
#else
#define RICTUS_IRC_MODULE_PATH "build/linux/modules/irc.so"
#endif

/*
 * Temporary artifact identity used only until Core-owned artifact hashing and
 * persisted qualification evidence are established. Because the inventory is
 * currently in-memory, this does not survive process restart.
 */
#define RICTUS_IRC_ARTIFACT_ID "runtime:irc:1.0.0"

int rictus_run(void)
{
    rictus_module_loader_t loader;
    rictus_module_registry_t registry;
    rictus_module_inventory_t inventory;
    const rictus_module_descriptor_t *descriptor = NULL;
    const rictus_module_record_t *record;
    rictus_module_loader_result_t load_result;
    rictus_module_result_t module_result;
    rictus_module_host_t host = {0};

    puts("STN-LABZ Rictus");

    rictus_module_loader_init(&loader);
    rictus_module_registry_init(&registry);
    rictus_module_inventory_init(&inventory);

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
        &inventory,
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
     * The startup invocation is currently the human-originated enable action.
     * Unlike the previous bootstrap, this now crosses the Core lifecycle
     * authority boundary. A module has no path to manufacture HUMAN authority.
     *
     * Persistent human enable/disable policy remains the next Core state step.
     */
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

    puts("[INFO] IRC module enabled by human Core authority.");

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
