#ifndef RICTUS_ARTIFACT_H
#define RICTUS_ARTIFACT_H

#include <stddef.h>

typedef enum rictus_artifact_result {
    RICTUS_ARTIFACT_OK = 0,
    RICTUS_ARTIFACT_ERR_INVALID_ARGUMENT,
    RICTUS_ARTIFACT_ERR_OPEN,
    RICTUS_ARTIFACT_ERR_READ,
    RICTUS_ARTIFACT_ERR_HASH
} rictus_artifact_result_t;

/*
 * Compute the SHA-256 identity of the exact artifact bytes.
 * output must provide at least 65 bytes for 64 lowercase hex characters
 * plus the terminating NUL.
 */
rictus_artifact_result_t rictus_artifact_sha256(
    const char *path,
    char *output,
    size_t output_size);

const char *rictus_artifact_result_string(rictus_artifact_result_t result);

#endif
