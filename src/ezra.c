#include <stdio.h>
#include "agent_root.h"
#include "ezra.h"
#include "logging.h"
#include "core_discovery.h"
#include "core_activation.h"
#include "core_inventory.h"
#include "stnlabz_abi/module_registry.h"
#include "stnlabz_abi/module_loader.h"
#include "runtime.h"

/* Core identity and operations never depend on installed capability modules. */
int stnlabz_initialize(void)
{
    stnlabz_runtime_t runtime;
    stnlabz_module_registry_t modules;
    stnlabz_module_loader_t loader;
    stnlabz_module_discovery_report_t discovery;
    char modules_path[1024];
    const stnlabz_agent_root_t *root;

    /* Logging is mandatory; inability to record Core activity denies startup. */
    stnlabz_module_registry_init(&modules);
    stnlabz_module_loader_init(&loader);
    if (!stnlabz_log_open()) {
        fprintf(stderr, "[CORE] Cannot open mandatory log: %s\n", STNLABZ_LOG_PATH);
        return 1;
    }
    if (!stnlabz_log_event("CORE", "START", "status=initiated")) goto log_failure;
    root = stnlabz_agent_root_identity();
    if (!stnlabz_agent_root_validate(root)) {
        (void)stnlabz_log_event("CORE", "ROOT_REJECTED", "startup_denied=true");
        fputs("[CORE] Agent Root validation failed. Startup denied.\n", stderr);
        stnlabz_log_close();
        return 1;
    }
    if (!stnlabz_log_event("CORE", "ROOT_VERIFIED", "agent=ezra revision=1")) goto log_failure;
    stnlabz_runtime_init(&runtime);
    if (stnlabz_module_discovery_get_path(modules_path, sizeof modules_path) != STNLABZ_MODULE_OK)
        goto log_failure;
    if (!stnlabz_core_discover(&modules, &loader, modules_path, &discovery))
        goto log_failure;
    if (!stnlabz_core_verify(&modules)) goto log_failure;
    if (!stnlabz_core_qualify(&modules)) goto log_failure;
    if (!stnlabz_core_inventory_log(&modules)) goto log_failure;
    if (!stnlabz_log_event("CORE", "MODULE_MANAGER_READY", "status=discovery_complete")) goto log_failure;

    printf("%s\n", EZRA_NAME);
    printf("[CORE] Agent Root verified: %s (revision %u)\n",
           root->agent_name, (unsigned)root->revision);
    printf("[CORE] Module manager ready: %zu registered modules.\n", modules.count);
    puts("[CORE] Qualification complete; no modules activated. SIGINT/SIGTERM to stop.");
    fflush(stdout);
    if (!stnlabz_log_event("CORE", "RUNNING", "verification=complete qualification=complete activation=disabled") || !stnlabz_log_sync()) goto log_failure;
    if (!stnlabz_runtime_run(&runtime)) {
        (void)stnlabz_log_event("CORE", "RUNTIME_FAILED", NULL);
        if (!stnlabz_core_stop_all(&modules)) goto stop_failure;
        stnlabz_core_discovery_cleanup(&modules, &loader);
        (void)stnlabz_log_event("MODULE", "MANAGER_SHUTDOWN", "id=unknown result=0");
        stnlabz_log_close();
        fputs("[CORE] Runtime failed.\n", stderr);
        return 1;
    }
    if (!stnlabz_core_stop_all(&modules)) goto stop_failure;
    if (!stnlabz_core_inventory_log(&modules)) goto log_failure;
    stnlabz_core_discovery_cleanup(&modules, &loader);
    (void)stnlabz_log_event("MODULE", "MANAGER_SHUTDOWN", "id=unknown result=0");
    if (!stnlabz_log_event("CORE", "STOPPED", "reason=signal") || !stnlabz_log_sync()) goto log_failure;
    stnlabz_log_close();
    printf("[CORE] Stopped. Shutdown recorded in %s\n", STNLABZ_LOG_PATH);
    return 0;

stop_failure:
    (void)stnlabz_log_event("CORE", "STOP_FAILED", "reason=module_stop_failure_or_audit");
    stnlabz_log_close();
    fputs("[CORE] Module stop failed; shared library handles retained.\n", stderr);
    return 1;

log_failure:
    if (!stnlabz_core_stop_all(&modules)) goto stop_failure;
    stnlabz_core_discovery_cleanup(&modules, &loader);
    (void)stnlabz_log_event("MODULE", "MANAGER_SHUTDOWN", "id=unknown result=0");
    stnlabz_log_close();
    fputs("[CORE] Mandatory logging failed.\n", stderr);
    return 1;
}
