/*
 * STN-LABZ Rictus Core
 * Linux native dynamic module loader.
 */

#include <dlfcn.h>
#include <string.h>

#include "rictus_module_loader.h"

static int descriptor_valid(const rictus_module_descriptor_t *descriptor)
{
    if (descriptor == NULL ||
        descriptor->id[0] == '\0' ||
        descriptor->name[0] == '\0' ||
        descriptor->qualify == NULL) {
        return 0;
    }

    return memchr(descriptor->id, '\0', sizeof(descriptor->id)) != NULL &&
           memchr(descriptor->name, '\0', sizeof(descriptor->name)) != NULL;
}

void stnlabz_module_loader_init(stnlabz_module_loader_t *loader)
{
    if (loader != NULL) {
        memset(loader, 0, sizeof(*loader));
    }
}

const stnlabz_loaded_module_t *stnlabz_module_loader_find(
    const stnlabz_module_loader_t *loader,
    const char *module_id)
{
    size_t index;

    if (loader == NULL || module_id == NULL) {
        return NULL;
    }

    for (index = 0U; index < loader->count; ++index) {
        if (strcmp(loader->modules[index].module_id, module_id) == 0) {
            return &loader->modules[index];
        }
    }

    return NULL;
}

stnlabz_module_loader_result_t stnlabz_module_loader_load(
    stnlabz_module_loader_t *loader,
    const char *expected_module_id,
    const char *path,
    const stnlabz_module_descriptor_t **descriptor_out)
{
    void *handle;
    void *symbol;
    stnlabz_module_get_descriptor_fn get_descriptor;
    const stnlabz_module_descriptor_t *descriptor;
    stnlabz_loaded_module_t *loaded;
    size_t id_length;
    size_t path_length;

    if (loader == NULL || expected_module_id == NULL || path == NULL ||
        descriptor_out == NULL || expected_module_id[0] == '\0' ||
        path[0] == '\0') {
        return STNLABZ_MODULE_LOADER_ERR_INVALID_ARGUMENT;
    }

    *descriptor_out = NULL;

    if (stnlabz_module_loader_find(loader, expected_module_id) != NULL) {
        return STNLABZ_MODULE_LOADER_ERR_ALREADY_LOADED;
    }

    if (loader->count >= STNLABZ_MODULE_LOADER_MAX) {
        return STNLABZ_MODULE_LOADER_ERR_FULL;
    }

    id_length = strlen(expected_module_id);
    path_length = strlen(path);
    if (id_length >= STNLABZ_MODULE_ID_MAX ||
        path_length >= STNLABZ_MODULE_LOADER_PATH_MAX) {
        return STNLABZ_MODULE_LOADER_ERR_INVALID_ARGUMENT;
    }

    handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        return STNLABZ_MODULE_LOADER_ERR_LOAD_FAILED;
    }

    symbol = dlsym(handle, STNLABZ_MODULE_DESCRIPTOR_EXPORT);
    if (symbol == NULL) {
        dlclose(handle);
        return STNLABZ_MODULE_LOADER_ERR_EXPORT_MISSING;
    }

    memcpy(&get_descriptor, &symbol, sizeof(get_descriptor));
    descriptor = get_descriptor();
    if (!descriptor_valid(descriptor)) {
        dlclose(handle);
        return STNLABZ_MODULE_LOADER_ERR_DESCRIPTOR_INVALID;
    }

    if (strcmp(descriptor->id, expected_module_id) != 0) {
        dlclose(handle);
        return STNLABZ_MODULE_LOADER_ERR_ID_MISMATCH;
    }

    loaded = &loader->modules[loader->count];
    memset(loaded, 0, sizeof(*loaded));
    loaded->handle = handle;
    memcpy(loaded->module_id, expected_module_id, id_length + 1U);
    memcpy(loaded->dll_path, path, path_length + 1U);
    loaded->descriptor = descriptor;
    ++loader->count;
    *descriptor_out = descriptor;

    return STNLABZ_MODULE_LOADER_OK;
}

stnlabz_module_loader_result_t stnlabz_module_loader_unload(
    stnlabz_module_loader_t *loader,
    const char *module_id)
{
    size_t index;

    if (loader == NULL || module_id == NULL || module_id[0] == '\0') {
        return STNLABZ_MODULE_LOADER_ERR_INVALID_ARGUMENT;
    }

    for (index = 0U; index < loader->count; ++index) {
        if (strcmp(loader->modules[index].module_id, module_id) == 0) {
            size_t move_index;
            if (loader->modules[index].handle != NULL) {
                dlclose(loader->modules[index].handle);
            }
            for (move_index = index; move_index + 1U < loader->count; ++move_index) {
                loader->modules[move_index] = loader->modules[move_index + 1U];
            }
            memset(&loader->modules[loader->count - 1U], 0,
                   sizeof(loader->modules[loader->count - 1U]));
            --loader->count;
            return STNLABZ_MODULE_LOADER_OK;
        }
    }

    return STNLABZ_MODULE_LOADER_ERR_NOT_FOUND;
}

void stnlabz_module_loader_unload_all(stnlabz_module_loader_t *loader)
{
    if (loader == NULL) {
        return;
    }

    while (loader->count > 0U) {
        size_t index = loader->count - 1U;
        if (loader->modules[index].handle != NULL) {
            dlclose(loader->modules[index].handle);
        }
        memset(&loader->modules[index], 0, sizeof(loader->modules[index]));
        --loader->count;
    }
}

const char *stnlabz_module_loader_result_string(
    stnlabz_module_loader_result_t result)
{
    switch (result) {
    case STNLABZ_MODULE_LOADER_OK: return "OK";
    case STNLABZ_MODULE_LOADER_ERR_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case STNLABZ_MODULE_LOADER_ERR_FULL: return "FULL";
    case STNLABZ_MODULE_LOADER_ERR_ALREADY_LOADED: return "ALREADY_LOADED";
    case STNLABZ_MODULE_LOADER_ERR_LOAD_FAILED: return "LOAD_FAILED";
    case STNLABZ_MODULE_LOADER_ERR_EXPORT_MISSING: return "EXPORT_MISSING";
    case STNLABZ_MODULE_LOADER_ERR_DESCRIPTOR_INVALID: return "DESCRIPTOR_INVALID";
    case STNLABZ_MODULE_LOADER_ERR_ID_MISMATCH: return "ID_MISMATCH";
    case STNLABZ_MODULE_LOADER_ERR_NOT_FOUND: return "NOT_FOUND";
    default: return "UNKNOWN";
    }
}
