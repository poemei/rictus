#include <stdio.h>

#include "rictus.h"
#include "rictus_module_loader.h"

#if defined(_WIN32)
#define RICTUS_IRC_MODULE_PATH "build\\windows\\modules\\irc.dll"
#else
#define RICTUS_IRC_MODULE_PATH "build/linux/modules/irc.so"
#endif

int rictus_run(void)
{
    rictus_module_loader_t loader;
    const rictus_module_descriptor_t *descriptor = NULL;
    rictus_module_qualification_result_t qualification;
    rictus_module_loader_result_t load_result;
    rictus_module_result_t module_result;
    rictus_module_host_t host = {0};

    puts("STN-LABZ Rictus");

    rictus_module_loader_init(&loader);

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

    module_result = descriptor->qualify(&qualification);
    if (module_result != RICTUS_MODULE_OK) {
        fprintf(stderr,
                "[ERROR] IRC module qualification: %s (%u/%u tests passed)\n",
                rictus_module_result_string(module_result),
                qualification.tests_passed,
                qualification.tests_executed);
        rictus_module_loader_unload_all(&loader);
        return 1;
    }

    printf("[INFO] IRC module qualified: %u/%u tests passed.\n",
           qualification.tests_passed,
           qualification.tests_executed);

    /*
     * Temporary bootstrap authority:
     * this call represents the operator starting Rictus with IRC enabled.
     * Persistent human enable/disable state will replace this bootstrap once
     * Core state persistence is established.
     */
    puts("[INFO] IRC module enabled by operator startup.");

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
