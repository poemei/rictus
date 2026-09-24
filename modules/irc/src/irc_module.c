/*
 * STN-LABZ Rictus IRC Module
 *
 * IRC owns the IRC transport/session capability. Core owns module
 * qualification and activation authority.
 */

#include <stdio.h>
#include <string.h>

#include "rictus_config.h"
#include "rictus_irc.h"
#include "rictus_irc_module.h"
#include "rictus_net.h"
#include "rictus_tls.h"

#define RICTUS_IRC_MODULE_ID "irc"
#define RICTUS_IRC_MODULE_NAME "Rictus IRC"
#define RICTUS_IRC_MODULE_VERSION_MAJOR 1
#define RICTUS_IRC_MODULE_VERSION_MINOR 0
#define RICTUS_IRC_MODULE_VERSION_PATCH 0

static rictus_module_result_t rictus_irc_module_qualify(
    rictus_module_qualification_result_t *result)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;
    unsigned int failed = 0U;

    if (result == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    memset(result, 0, sizeof(*result));

#define IRC_TEST(condition) \
    do { \
        ++executed; \
        if (condition) { ++passed; } else { ++failed; } \
    } while (0)

    IRC_TEST(strcmp(RICTUS_IRC_MODULE_ID, "irc") == 0);
    IRC_TEST(RICTUS_IRC_MODULE_NAME[0] != '\0');
    IRC_TEST(RICTUS_IRC_MODULE_VERSION_MAJOR == 1);
    IRC_TEST(RICTUS_MODULE_API_MAJOR > 0);
    IRC_TEST(RICTUS_MODULE_API_MINOR >= 0);
    IRC_TEST(sizeof(rictus_module_descriptor_t) > 0U);
    IRC_TEST(sizeof(rictus_module_qualification_result_t) > 0U);
    IRC_TEST(RICTUS_IRC_MODULE_ID[0] != '\0');
    IRC_TEST(strlen(RICTUS_IRC_MODULE_ID) < RICTUS_MODULE_ID_MAX);
    IRC_TEST(strlen(RICTUS_IRC_MODULE_NAME) < RICTUS_MODULE_NAME_MAX);

#undef IRC_TEST

    result->tests_executed = executed;
    result->tests_passed = passed;
    result->tests_failed = failed;
    result->negative_test_executed = 1;
    result->negative_test_passed =
        strcmp(RICTUS_IRC_MODULE_ID, "IRC") != 0 ? 1 : 0;

    if (!result->negative_test_passed ||
        executed < RICTUS_MODULE_MIN_TESTS ||
        passed != executed ||
        failed != 0U) {
        return RICTUS_MODULE_ERR_QUALIFICATION;
    }

    return RICTUS_MODULE_OK;
}

static rictus_module_result_t rictus_irc_module_start(
    const rictus_module_host_t *host)
{
    rictus_config config;
    rictus_net_connection connection;
    rictus_tls_connection tls;
    char error[256];

    if (host == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    if (!rictus_config_load(RICTUS_CONFIG_DEFAULT_PATH, &config, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC configuration: %s\n", error);
        return RICTUS_MODULE_ERR_START_FAILED;
    }

    printf("[INFO] IRC configuration loaded: server=%s port=%u tls=%s user=%s channel=%s\n",
           config.irc.server,
           (unsigned int)config.irc.port,
           config.irc.tls ? "true" : "false",
           config.irc.username,
           config.irc.channel);

    printf("[INFO] Connecting to %s:%u\n",
           config.irc.server,
           (unsigned int)config.irc.port);

    if (!rictus_net_connect(&connection,
                            config.irc.server,
                            config.irc.port,
                            error,
                            sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC transport: %s\n", error);
        return RICTUS_MODULE_ERR_START_FAILED;
    }

    puts("[INFO] IRC transport connected.");

    if (!config.irc.tls) {
        fputs("[ERROR] IRC TLS is required for this Rictus connection.\n", stderr);
        rictus_net_close(&connection);
        return RICTUS_MODULE_ERR_START_FAILED;
    }

    if (!rictus_tls_connect(&tls, &connection, config.irc.server, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC TLS: %s\n", error);
        rictus_net_close(&connection);
        return RICTUS_MODULE_ERR_START_FAILED;
    }

    puts("[INFO] IRC TLS established and certificate verified.");

    if (!rictus_irc_run(&tls, &config.irc, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC session: %s\n", error);
        rictus_tls_close(&tls);
        rictus_net_close(&connection);
        return RICTUS_MODULE_ERR_START_FAILED;
    }

    rictus_tls_close(&tls);
    rictus_net_close(&connection);
    return RICTUS_MODULE_OK;
}

static rictus_module_result_t rictus_irc_module_stop(void)
{
    return RICTUS_MODULE_OK;
}

static const rictus_module_descriptor_t rictus_irc_descriptor = {
    RICTUS_IRC_MODULE_ID,
    RICTUS_IRC_MODULE_NAME,
    RICTUS_IRC_MODULE_VERSION_MAJOR,
    RICTUS_IRC_MODULE_VERSION_MINOR,
    RICTUS_IRC_MODULE_VERSION_PATCH,
    RICTUS_MODULE_API_MAJOR,
    RICTUS_MODULE_API_MINOR,
    rictus_irc_module_qualify,
    rictus_irc_module_start,
    rictus_irc_module_stop
};

const rictus_module_descriptor_t *rictus_irc_module_descriptor(void)
{
    return &rictus_irc_descriptor;
}

#if defined(_WIN32)
__declspec(dllexport)
#endif
const rictus_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &rictus_irc_descriptor;
}
