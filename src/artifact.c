/*
 * STN-LABZ Rictus Core
 *
 * Deterministic artifact identity from exact file bytes.
 */

#include <stdio.h>

#include <openssl/evp.h>

#include "rictus_artifact.h"

rictus_artifact_result_t rictus_artifact_sha256(
    const char *path,
    char *output,
    size_t output_size)
{
    FILE *file;
    EVP_MD_CTX *context;
    unsigned char buffer[8192];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0;
    size_t count;
    size_t index;

    if (path == NULL || path[0] == '\0' ||
        output == NULL || output_size < 65) {
        return RICTUS_ARTIFACT_ERR_INVALID_ARGUMENT;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return RICTUS_ARTIFACT_ERR_OPEN;
    }

    context = EVP_MD_CTX_new();
    if (context == NULL) {
        fclose(file);
        return RICTUS_ARTIFACT_ERR_HASH;
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(context);
        fclose(file);
        return RICTUS_ARTIFACT_ERR_HASH;
    }

    while ((count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (EVP_DigestUpdate(context, buffer, count) != 1) {
            EVP_MD_CTX_free(context);
            fclose(file);
            return RICTUS_ARTIFACT_ERR_HASH;
        }
    }

    if (ferror(file)) {
        EVP_MD_CTX_free(context);
        fclose(file);
        return RICTUS_ARTIFACT_ERR_READ;
    }

    fclose(file);

    if (EVP_DigestFinal_ex(context, digest, &digest_length) != 1 ||
        digest_length != 32) {
        EVP_MD_CTX_free(context);
        return RICTUS_ARTIFACT_ERR_HASH;
    }

    EVP_MD_CTX_free(context);

    for (index = 0; index < digest_length; ++index) {
        static const char hex[] = "0123456789abcdef";
        output[index * 2] = hex[digest[index] >> 4];
        output[index * 2 + 1] = hex[digest[index] & 0x0f];
    }

    output[64] = '\0';
    return RICTUS_ARTIFACT_OK;
}

const char *rictus_artifact_result_string(rictus_artifact_result_t result)
{
    switch (result) {
        case RICTUS_ARTIFACT_OK:
            return "OK";
        case RICTUS_ARTIFACT_ERR_INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";
        case RICTUS_ARTIFACT_ERR_OPEN:
            return "OPEN";
        case RICTUS_ARTIFACT_ERR_READ:
            return "READ";
        case RICTUS_ARTIFACT_ERR_HASH:
            return "HASH";
        default:
            return "UNKNOWN";
    }
}
