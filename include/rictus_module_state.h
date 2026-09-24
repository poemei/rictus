#ifndef RICTUS_MODULE_STATE_H
#define RICTUS_MODULE_STATE_H

#include <stddef.h>

#include "rictus_module_inventory.h"

#define RICTUS_MODULE_STATE_PATH "/var/lib/rictus/modules.state"

typedef enum rictus_module_state_result {
    RICTUS_MODULE_STATE_OK = 0,
    RICTUS_MODULE_STATE_NOT_FOUND,
    RICTUS_MODULE_STATE_ERR_INVALID_ARGUMENT,
    RICTUS_MODULE_STATE_ERR_IO,
    RICTUS_MODULE_STATE_ERR_FORMAT
} rictus_module_state_result_t;

typedef struct rictus_module_authorization_record {
    char module_id[RICTUS_MODULE_ID_MAX];
    int enabled;
} rictus_module_authorization_record_t;

typedef struct rictus_module_store {
    rictus_module_inventory_t inventory;
    rictus_module_authorization_record_t authorizations[
        RICTUS_MODULE_INVENTORY_MAX];
    size_t authorization_count;
} rictus_module_store_t;

void rictus_module_state_init(rictus_module_store_t *state);

rictus_module_state_result_t rictus_module_state_load(
    rictus_module_store_t *state,
    const char *path);

rictus_module_state_result_t rictus_module_state_save(
    const rictus_module_store_t *state,
    const char *path);

int rictus_module_state_enabled(
    const rictus_module_store_t *state,
    const char *module_id);

rictus_module_state_result_t rictus_module_state_set_enabled(
    rictus_module_store_t *state,
    const char *module_id,
    int enabled);

const char *rictus_module_state_result_string(
    rictus_module_state_result_t result);

#endif
