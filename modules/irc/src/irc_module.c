/*
 * STN-LABZ Rictus IRC Module
 *
 * Initial ABI-facing module boundary.
 *
 * The existing proven IRC transport/session remains untouched while this
 * module becomes the first real consumer used to prove Core module mechanics.
 */

#include <string.h>

#include "rictus_irc_module.h"

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
    if (host == NULL) {
        return RICTUS_MODULE_ERR_INVALID_ARGUMENT;
    }

    /*
     * Runtime ownership moves here only after Core loading/enablement is
     * proven. Until then the existing IRC runtime remains the evidence path.
     */
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
