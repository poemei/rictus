#ifndef RICTUS_MODULE_DISCOVERY_H
#define RICTUS_MODULE_DISCOVERY_H

#include <stddef.h>

#include "rictus_module.h"
#include "rictus_module_loader.h"
#include "rictus_module_registry.h"

#define RICTUS_MODULE_DISCOVERY_PATH_MAX 1024
#define RICTUS_MODULE_ARTIFACT_ID_MAX 65

typedef struct rictus_module_candidate {
    char module_id[RICTUS_MODULE_ID_MAX];
    char artifact_path[RICTUS_MODULE_DISCOVERY_PATH_MAX];
    char artifact_id[RICTUS_MODULE_ARTIFACT_ID_MAX];
} rictus_module_candidate_t;

typedef struct rictus_module_discovery_report {
    size_t entries_examined;
    size_t candidates_found;
    size_t candidates_rejected;
} rictus_module_discovery_report_t;

/*
 * Platform discovery produces candidates only.
 *
 * Discovery does not load, qualify, enable, or activate a module. Core owns
 * those later lifecycle decisions.
 */
rictus_module_result_t rictus_module_discovery_scan(
    const char *modules_path,
    rictus_module_candidate_t *candidates,
    size_t candidate_capacity,
    size_t *candidate_count,
    rictus_module_discovery_report_t *report);

#endif
