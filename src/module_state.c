/*
 * STN-LABZ Rictus Core
 *
 * Small deterministic persistence for Core-owned module qualification evidence
 * and human enable/disable policy.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rictus_module_state.h"

#define RICTUS_MODULE_STATE_LINE_MAX 1024

static int text_valid(const char *text, size_t capacity)
{
    size_t length;

    if (text == NULL || capacity == 0) {
        return 0;
    }

    length = strlen(text);
    return length > 0 && length < capacity;
}

static rictus_module_authorization_record_t *find_authorization(
    rictus_module_store_t *state,
    const char *module_id)
{
    size_t index;

    for (index = 0; index < state->authorization_count; ++index) {
        if (strcmp(state->authorizations[index].module_id, module_id) == 0) {
            return &state->authorizations[index];
        }
    }

    return NULL;
}

void rictus_module_state_init(rictus_module_store_t *state)
{
    if (state != NULL) {
        memset(state, 0, sizeof(*state));
        rictus_module_inventory_init(&state->inventory);
    }
}

int rictus_module_state_enabled(
    const rictus_module_store_t *state,
    const char *module_id)
{
    size_t index;

    if (state == NULL || module_id == NULL) {
        return 0;
    }

    for (index = 0; index < state->authorization_count; ++index) {
        if (strcmp(state->authorizations[index].module_id, module_id) == 0) {
            return state->authorizations[index].enabled == 1;
        }
    }

    return 0;
}

rictus_module_state_result_t rictus_module_state_set_enabled(
    rictus_module_store_t *state,
    const char *module_id,
    int enabled)
{
    rictus_module_authorization_record_t *record;

    if (state == NULL ||
        !text_valid(module_id, RICTUS_MODULE_ID_MAX) ||
        (enabled != 0 && enabled != 1)) {
        return RICTUS_MODULE_STATE_ERR_INVALID_ARGUMENT;
    }

    record = find_authorization(state, module_id);
    if (record == NULL) {
        if (state->authorization_count >= RICTUS_MODULE_INVENTORY_MAX) {
            return RICTUS_MODULE_STATE_ERR_IO;
        }

        record = &state->authorizations[state->authorization_count++];
        memset(record, 0, sizeof(*record));
        memcpy(record->module_id, module_id, strlen(module_id) + 1);
    }

    record->enabled = enabled;
    return RICTUS_MODULE_STATE_OK;
}

rictus_module_state_result_t rictus_module_state_load(
    rictus_module_store_t *state,
    const char *path)
{
    FILE *file;
    char line[RICTUS_MODULE_STATE_LINE_MAX];

    if (state == NULL || path == NULL || path[0] == '\0') {
        return RICTUS_MODULE_STATE_ERR_INVALID_ARGUMENT;
    }

    rictus_module_state_init(state);

    file = fopen(path, "r");
    if (file == NULL) {
        return RICTUS_MODULE_STATE_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char kind[16];
        char module_id[RICTUS_MODULE_ID_MAX];
        char artifact_id[RICTUS_MODULE_ARTIFACT_ID_MAX];
        unsigned int vmaj, vmin, vpatch, amaj, amin;
        unsigned int executed, passed, failed;
        int negative_executed, negative_passed, enabled;
        rictus_module_descriptor_t descriptor;
        rictus_module_qualification_result_t qualification;

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (sscanf(line, "%15s", kind) != 1) {
            fclose(file);
            return RICTUS_MODULE_STATE_ERR_FORMAT;
        }

        if (strcmp(kind, "QUAL") == 0) {
            if (sscanf(
                    line,
                    "QUAL %63s %64s %u %u %u %u %u %u %u %u %d %d",
                    module_id, artifact_id,
                    &vmaj, &vmin, &vpatch, &amaj, &amin,
                    &executed, &passed, &failed,
                    &negative_executed, &negative_passed) != 12) {
                fclose(file);
                return RICTUS_MODULE_STATE_ERR_FORMAT;
            }

            memset(&descriptor, 0, sizeof(descriptor));
            memcpy(descriptor.id, module_id, strlen(module_id) + 1);
            descriptor.version_major = vmaj;
            descriptor.version_minor = vmin;
            descriptor.version_patch = vpatch;
            descriptor.required_core_api_major = amaj;
            descriptor.required_core_api_minor = amin;

            memset(&qualification, 0, sizeof(qualification));
            qualification.tests_executed = executed;
            qualification.tests_passed = passed;
            qualification.tests_failed = failed;
            qualification.negative_test_executed = negative_executed;
            qualification.negative_test_passed = negative_passed;

            if (rictus_module_inventory_store(
                    &state->inventory,
                    &descriptor,
                    artifact_id,
                    &qualification) != RICTUS_MODULE_INVENTORY_OK) {
                fclose(file);
                return RICTUS_MODULE_STATE_ERR_FORMAT;
            }
        } else if (strcmp(kind, "AUTH") == 0) {
            if (sscanf(line, "AUTH %63s %d", module_id, &enabled) != 2 ||
                rictus_module_state_set_enabled(
                    state, module_id, enabled) != RICTUS_MODULE_STATE_OK) {
                fclose(file);
                return RICTUS_MODULE_STATE_ERR_FORMAT;
            }
        } else {
            fclose(file);
            return RICTUS_MODULE_STATE_ERR_FORMAT;
        }
    }

    if (ferror(file)) {
        fclose(file);
        return RICTUS_MODULE_STATE_ERR_IO;
    }

    fclose(file);
    return RICTUS_MODULE_STATE_OK;
}

rictus_module_state_result_t rictus_module_state_save(
    const rictus_module_store_t *state,
    const char *path)
{
    FILE *file;
    size_t index;

    if (state == NULL || path == NULL || path[0] == '\0') {
        return RICTUS_MODULE_STATE_ERR_INVALID_ARGUMENT;
    }

    file = fopen(path, "w");
    if (file == NULL) {
        return RICTUS_MODULE_STATE_ERR_IO;
    }

    fputs("# Rictus Core module state v1\n", file);

    for (index = 0; index < state->inventory.count; ++index) {
        const rictus_module_inventory_record_t *record =
            &state->inventory.records[index];

        if (fprintf(
                file,
                "QUAL %s %s %u %u %u %u %u %u %u %u %d %d\n",
                record->module_id,
                record->artifact_id,
                record->version_major,
                record->version_minor,
                record->version_patch,
                record->core_api_major,
                record->core_api_minor,
                record->qualification.tests_executed,
                record->qualification.tests_passed,
                record->qualification.tests_failed,
                record->qualification.negative_test_executed,
                record->qualification.negative_test_passed) < 0) {
            fclose(file);
            return RICTUS_MODULE_STATE_ERR_IO;
        }
    }

    for (index = 0; index < state->authorization_count; ++index) {
        const rictus_module_authorization_record_t *record =
            &state->authorizations[index];

        if (fprintf(file, "AUTH %s %d\n",
                    record->module_id, record->enabled) < 0) {
            fclose(file);
            return RICTUS_MODULE_STATE_ERR_IO;
        }
    }

    if (fclose(file) != 0) {
        return RICTUS_MODULE_STATE_ERR_IO;
    }

    return RICTUS_MODULE_STATE_OK;
}

const char *rictus_module_state_result_string(
    rictus_module_state_result_t result)
{
    switch (result) {
        case RICTUS_MODULE_STATE_OK:
            return "OK";
        case RICTUS_MODULE_STATE_NOT_FOUND:
            return "NOT_FOUND";
        case RICTUS_MODULE_STATE_ERR_INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";
        case RICTUS_MODULE_STATE_ERR_IO:
            return "IO";
        case RICTUS_MODULE_STATE_ERR_FORMAT:
            return "FORMAT";
        default:
            return "UNKNOWN";
    }
}
