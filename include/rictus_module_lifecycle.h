#ifndef RICTUS_MODULE_LIFECYCLE_H
#define RICTUS_MODULE_LIFECYCLE_H

#include "rictus_module_inventory.h"
#include "rictus_module_registry.h"

/*
 * Rictus-owned policy above the shared ABI registry.
 *
 * The ABI defines module mechanics. Core decides when qualification evidence
 * may be restored and when human authorization may be translated into ABI
 * activation authority.
 */

typedef enum rictus_module_authority {
    RICTUS_MODULE_AUTHORITY_NONE = 0,
    RICTUS_MODULE_AUTHORITY_HUMAN
} rictus_module_authority_t;

typedef enum rictus_module_prepare_action {
    RICTUS_MODULE_PREPARE_NONE = 0,
    RICTUS_MODULE_PREPARE_QUALIFIED,
    RICTUS_MODULE_PREPARE_RESTORED
} rictus_module_prepare_action_t;

rictus_module_result_t rictus_module_lifecycle_prepare(
    rictus_module_registry_t *registry,
    rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id,
    rictus_module_prepare_action_t *action);

rictus_module_result_t rictus_module_lifecycle_enable(
    rictus_module_registry_t *registry,
    const char *module_id,
    rictus_module_authority_t authority);

#endif
