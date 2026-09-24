/*
 * STN-LABZ Rictus Core
 *
 * Core-owned in-memory module qualification evidence.
 */

#include <string.h>

#include "rictus_module_inventory.h"

static int rictus_module_inventory_text_valid(const char *text, size_t capacity)
{
    size_t length;

    if (text == NULL || capacity == 0) {
        return 0;
    }

    length = strlen(text);
    return length > 0 && length < capacity;
}

static int rictus_module_inventory_qualification_valid(
    const rictus_module_qualification_result_t *qualification)
{
    if (qualification == NULL) {
        return 0;
    }

    return qualification->tests_executed >= RICTUS_MODULE_MIN_TESTS &&
           qualification->tests_passed == qualification->tests_executed &&
           qualification->tests_failed == 0 &&
           qualification->tests_passed + qualification->tests_failed ==
               qualification->tests_executed &&
           qualification->negative_test_executed &&
           qualification->negative_test_passed;
}

void rictus_module_inventory_init(rictus_module_inventory_t *inventory)
{
    if (inventory != NULL) {
        memset(inventory, 0, sizeof(*inventory));
    }
}

rictus_module_inventory_result_t rictus_module_inventory_store(
    rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id,
    const rictus_module_qualification_result_t *qualification)
{
    rictus_module_inventory_record_t *record = NULL;
    size_t index;

    if (inventory == NULL || descriptor == NULL ||
        !rictus_module_inventory_text_valid(
            descriptor->id, sizeof(descriptor->id)) ||
        !rictus_module_inventory_text_valid(
            artifact_id, RICTUS_MODULE_ARTIFACT_ID_MAX) ||
        !rictus_module_inventory_qualification_valid(qualification)) {
        return RICTUS_MODULE_INVENTORY_ERR_INVALID_ARGUMENT;
    }

    for (index = 0; index < inventory->count; ++index) {
        if (strcmp(inventory->records[index].module_id, descriptor->id) == 0) {
            record = &inventory->records[index];
            break;
        }
    }

    if (record == NULL) {
        if (inventory->count >= RICTUS_MODULE_INVENTORY_MAX) {
            return RICTUS_MODULE_INVENTORY_ERR_FULL;
        }

        record = &inventory->records[inventory->count++];
    }

    memset(record, 0, sizeof(*record));

    memcpy(record->module_id, descriptor->id, strlen(descriptor->id) + 1);
    memcpy(record->artifact_id, artifact_id, strlen(artifact_id) + 1);

    record->version_major = descriptor->version_major;
    record->version_minor = descriptor->version_minor;
    record->version_patch = descriptor->version_patch;
    record->core_api_major = descriptor->required_core_api_major;
    record->core_api_minor = descriptor->required_core_api_minor;
    record->qualification = *qualification;

    return RICTUS_MODULE_INVENTORY_OK;
}

const rictus_module_inventory_record_t *rictus_module_inventory_find(
    const rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id)
{
    size_t index;

    if (inventory == NULL || descriptor == NULL ||
        !rictus_module_inventory_text_valid(
            artifact_id, RICTUS_MODULE_ARTIFACT_ID_MAX)) {
        return NULL;
    }

    for (index = 0; index < inventory->count; ++index) {
        const rictus_module_inventory_record_t *record =
            &inventory->records[index];

        if (strcmp(record->module_id, descriptor->id) != 0) {
            continue;
        }

        if (strcmp(record->artifact_id, artifact_id) != 0 ||
            record->version_major != descriptor->version_major ||
            record->version_minor != descriptor->version_minor ||
            record->version_patch != descriptor->version_patch ||
            record->core_api_major != descriptor->required_core_api_major ||
            record->core_api_minor != descriptor->required_core_api_minor ||
            !rictus_module_inventory_qualification_valid(
                &record->qualification)) {
            return NULL;
        }

        return record;
    }

    return NULL;
}

const char *rictus_module_inventory_result_string(
    rictus_module_inventory_result_t result)
{
    switch (result) {
        case RICTUS_MODULE_INVENTORY_OK:
            return "OK";
        case RICTUS_MODULE_INVENTORY_ERR_INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";
        case RICTUS_MODULE_INVENTORY_ERR_INVALID_FORMAT:
            return "INVALID_FORMAT";
        case RICTUS_MODULE_INVENTORY_ERR_FULL:
            return "FULL";
        default:
            return "UNKNOWN";
    }
}
