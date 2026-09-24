/*
 * STN-LABZ Rictus Core
 *
 * Core policy for qualification restoration and human-controlled enablement.
 */

#include "rictus_module_lifecycle.h"

rictus_module_result_t rictus_module_lifecycle_prepare(
    rictus_module_registry_t *registry,
    rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id)
{
    const rictus_module_inventory_record_t *evidence;
    const rictus_module_record_t *record;
    rictus_module_result_t result;

    if (registry == NULL || inventory == NULL ||
        descriptor == NULL || artifact_id == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    result = rictus_module_registry_discover(registry, descriptor);
    if (result != RICTUS_MODULE_OK) {
        return result;
    }

    result = rictus_module_registry_verify(registry, descriptor->id);
    if (result != RICTUS_MODULE_OK) {
        return result;
    }

    evidence = rictus_module_inventory_find(
        inventory, descriptor, artifact_id);

    if (evidence != NULL) {
        /*
         * The ABI registry restores qualification with activation authority
         * cleared. Core never treats persisted qualification as permission.
         */
        return rictus_module_registry_restore_qualification(
            registry, descriptor->id, &evidence->qualification);
    }

    result = rictus_module_registry_qualify(registry, descriptor->id);
    if (result != RICTUS_MODULE_OK) {
        return result;
    }

    record = rictus_module_registry_find(registry, descriptor->id);
    if (record == NULL) {
        return RICTUS_MODULE_ERR_NOT_FOUND;
    }

    if (rictus_module_inventory_store(
            inventory,
            &record->descriptor,
            artifact_id,
            &record->qualification) != RICTUS_MODULE_INVENTORY_OK) {
        /*
         * Qualification that Core cannot record is not accepted as durable
         * qualification evidence.
         */
        (void)rictus_module_registry_fail(registry, descriptor->id);
        return RICTUS_MODULE_ERR_QUALIFICATION;
    }

    return RICTUS_MODULE_OK;
}

rictus_module_result_t rictus_module_lifecycle_enable(
    rictus_module_registry_t *registry,
    const char *module_id,
    rictus_module_authority_t authority)
{
    rictus_module_result_t result;

    if (registry == NULL || module_id == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    /*
     * No module-facing authority token exists. Only a human-originated Core
     * action may cross this boundary.
     */
    if (authority != RICTUS_MODULE_AUTHORITY_HUMAN) {
        return RICTUS_MODULE_ERR_NOT_AUTHORIZED;
    }

    result = rictus_module_registry_authorize_activation(
        registry, module_id);
    if (result != RICTUS_MODULE_OK) {
        return result;
    }

    return rictus_module_registry_activate(registry, module_id);
}
