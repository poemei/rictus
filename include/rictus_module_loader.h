#ifndef RICTUS_MODULE_LOADER_H
#define RICTUS_MODULE_LOADER_H

#include <stddef.h>

#include "rictus_module.h"

/*
 * Rictus module-loader contract.
 *
 * The shared ABI descriptor remains authoritative. Native library handles,
 * paths, and loader mechanics belong to Rictus platform implementations and
 * must not leak Windows headers into the common Core contract.
 */

#define RICTUS_MODULE_LOADER_MAX 32
#define RICTUS_MODULE_LOADER_PATH_MAX 1024
#define RICTUS_MODULE_DESCRIPTOR_EXPORT "stnlabz_module_get_descriptor"

typedef enum rictus_module_loader_result {
    RICTUS_MODULE_LOADER_OK = 0,
    RICTUS_MODULE_LOADER_ERR_INVALID_ARGUMENT,
    RICTUS_MODULE_LOADER_ERR_FULL,
    RICTUS_MODULE_LOADER_ERR_ALREADY_LOADED,
    RICTUS_MODULE_LOADER_ERR_LOAD_FAILED,
    RICTUS_MODULE_LOADER_ERR_EXPORT_MISSING,
    RICTUS_MODULE_LOADER_ERR_DESCRIPTOR_INVALID,
    RICTUS_MODULE_LOADER_ERR_ID_MISMATCH,
    RICTUS_MODULE_LOADER_ERR_NOT_FOUND
} rictus_module_loader_result_t;

typedef const rictus_module_descriptor_t *(*rictus_module_get_descriptor_fn)(void);

typedef struct rictus_loaded_module {
    void *handle;
    char module_id[RICTUS_MODULE_ID_MAX];
    char artifact_path[RICTUS_MODULE_LOADER_PATH_MAX];
    const rictus_module_descriptor_t *descriptor;
} rictus_loaded_module_t;

typedef struct rictus_module_loader {
    rictus_loaded_module_t modules[RICTUS_MODULE_LOADER_MAX];
    size_t count;
} rictus_module_loader_t;

void rictus_module_loader_init(rictus_module_loader_t *loader);

rictus_module_loader_result_t rictus_module_loader_load(
    rictus_module_loader_t *loader,
    const char *expected_module_id,
    const char *artifact_path,
    const rictus_module_descriptor_t **descriptor_out);

rictus_module_loader_result_t rictus_module_loader_unload(
    rictus_module_loader_t *loader,
    const char *module_id);

void rictus_module_loader_unload_all(rictus_module_loader_t *loader);

const rictus_loaded_module_t *rictus_module_loader_find(
    const rictus_module_loader_t *loader,
    const char *module_id);

const char *rictus_module_loader_result_string(
    rictus_module_loader_result_t result);

#endif
