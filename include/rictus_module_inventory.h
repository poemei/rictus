#ifndef RICTUS_MODULE_INVENTORY_H
#define RICTUS_MODULE_INVENTORY_H

#include <stddef.h>

#include "rictus_module.h"

#define RICTUS_MODULE_INVENTORY_MAX 32
#define RICTUS_MODULE_ARTIFACT_ID_MAX 65

typedef enum {
    RICTUS_MODULE_INVENTORY_OK = 0,
    RICTUS_MODULE_INVENTORY_ERR_INVALID_ARGUMENT,
    RICTUS_MODULE_INVENTORY_ERR_INVALID_FORMAT,
    RICTUS_MODULE_INVENTORY_ERR_FULL
} rictus_module_inventory_result_t;

/*
 * Core-owned qualification evidence.
 *
 * Qualification belongs to one exact artifact.  A module may describe
 * itself, but it cannot create, replace, or authorize this record.
 */
typedef struct rictus_module_inventory_record {
    char module_id[RICTUS_MODULE_ID_MAX];
    char artifact_id[RICTUS_MODULE_ARTIFACT_ID_MAX];

    unsigned int version_major;
    unsigned int version_minor;
    unsigned int version_patch;

    unsigned int core_api_major;
    unsigned int core_api_minor;

    rictus_module_qualification_result_t qualification;
} rictus_module_inventory_record_t;

typedef struct rictus_module_inventory {
    rictus_module_inventory_record_t records[RICTUS_MODULE_INVENTORY_MAX];
    size_t count;
} rictus_module_inventory_t;

void rictus_module_inventory_init(rictus_module_inventory_t *inventory);

rictus_module_inventory_result_t rictus_module_inventory_store(
    rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id,
    const rictus_module_qualification_result_t *qualification);

const rictus_module_inventory_record_t *rictus_module_inventory_find(
    const rictus_module_inventory_t *inventory,
    const rictus_module_descriptor_t *descriptor,
    const char *artifact_id);

const char *rictus_module_inventory_result_string(
    rictus_module_inventory_result_t result);

#endif
