#include <stdio.h>
#include <string.h>

#include "stnlabz_abi/module.h"
#include "core_policy.h"
#include "rictus_policy.h"
#include "compliance_internal.h"

#define COMPLIANCE_VERSION_MAJOR 1U
#define COMPLIANCE_VERSION_MINOR 0U
#define COMPLIANCE_VERSION_PATCH 0U

static const stnlabz_module_host_t *owner;
static int command_registered;

static const char *decision_name(int decision)
{
    if (decision == STNLABZ_POLICY_ALLOW) return "ALLOW";
    if (decision == STNLABZ_POLICY_ESCALATE) return "ESCALATE";
    return "DENY";
}

static stnlabz_module_result_t compliance_controller(
    const stnlabz_module_command_t *command,
    stnlabz_module_command_reply_fn reply,
    void *reply_context,
    void *handler_context)
{
    rictus_compliance_scenario_t scenario;
    rictus_policy_evaluate_request_t request;
    rictus_policy_evaluate_response_t response;
    const char *actor;
    char output[512];
    size_t used = 0U;
    int written;
    stnlabz_module_result_t result;

    (void)handler_context;
    if (!command || !reply || !owner || !owner->invoke_service ||
        !memchr(command->arguments, 0, sizeof command->arguments))
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;

    if (!rictus_compliance_parse(command->arguments, &scenario)) {
        return reply(reply_context,
            "Usage: /compliance <operation> <target> mission=<yes|no> "
            "policy=<yes|no> instruction=<yes|no> authority=<yes|no> "
            "[escalate=<target>] ; execution is always suppressed.")
            ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_INVALID_STATE;
    }

    actor = command->account[0] ? command->account : command->sender;
    if (!actor[0]) actor = "operator";

    memset(&request, 0, sizeof request);
    memset(&response, 0, sizeof response);

    if (snprintf(request.actor, sizeof request.actor, "%s", actor) >=
            (int)sizeof request.actor ||
        snprintf(request.mission, sizeof request.mission,
            "%s", "rictus-control-proving-ground") >= (int)sizeof request.mission ||
        snprintf(request.operation, sizeof request.operation,
            "%s", scenario.operation) >= (int)sizeof request.operation ||
        snprintf(request.module_id, sizeof request.module_id,
            "%s", scenario.module_id) >= (int)sizeof request.module_id ||
        snprintf(request.verification_source, sizeof request.verification_source,
            "%s", "operator_evaluate_only") >=
            (int)sizeof request.verification_source)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    if (scenario.escalation_target[0] &&
        snprintf(request.escalation_target, sizeof request.escalation_target,
            "%s", scenario.escalation_target) >=
            (int)sizeof request.escalation_target)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    request.within_mission = scenario.within_mission;
    request.complies_with_policy = scenario.complies_with_policy;
    request.instruction_authorized = scenario.instruction_authorized;
    request.authority_granted = scenario.authority_granted;
    request.context_verified = 1;

    result = owner->invoke_service(STNLABZ_POLICY_EVALUATE_SERVICE,
        &request, sizeof request, &response, sizeof response, &used);
    if (result != STNLABZ_MODULE_OK) return result;
    if (used != sizeof response ||
        !memchr(response.policy_id, 0, sizeof response.policy_id) ||
        !memchr(response.reason, 0, sizeof response.reason))
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    written = snprintf(output, sizeof output,
        "Compliance: decision=%s policy=%s reason=%s "
        "operation=%s target=%s execution=SUPPRESSED",
        decision_name(response.decision), response.policy_id, response.reason,
        scenario.operation, scenario.module_id);
    if (written < 0 || (size_t)written >= sizeof output)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    return reply(reply_context, output)
        ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_INVALID_STATE;
}

static stnlabz_module_result_t compliance_qualify(
    stnlabz_module_qualification_result_t *qualification)
{
    unsigned executed = 0U, passed = 0U;
    int negative_executed = 0, negative_passed = 0;
    int ok;

    if (!qualification) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    memset(qualification, 0, sizeof *qualification);
    ok = rictus_compliance_selftest(&executed, &passed,
        &negative_executed, &negative_passed);
    qualification->tests_executed = executed;
    qualification->tests_passed = passed;
    qualification->tests_failed = executed - passed;
    qualification->negative_test_executed = negative_executed;
    qualification->negative_test_passed = negative_passed;
    return ok ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_QUALIFICATION;
}

static stnlabz_module_result_t compliance_start(const stnlabz_module_host_t *host)
{
    if (!host || !host->invoke_service ||
        !host->register_command || !host->unregister_command)
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    owner = host;
    if (!host->register_command("compliance", compliance_controller, NULL)) {
        owner = NULL;
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    command_registered = 1;
    fprintf(stderr, "[COMPLIANCE] evaluate-only General Order proving gate active\n");
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t compliance_stop(void)
{
    if (command_registered && owner &&
        !owner->unregister_command("compliance", NULL))
        return STNLABZ_MODULE_ERR_STOP_FAILED;
    command_registered = 0;
    owner = NULL;
    return STNLABZ_MODULE_OK;
}

static const stnlabz_module_descriptor_t descriptor = {
    "compliance",
    "Rictus Compliance Proving Gate",
    COMPLIANCE_VERSION_MAJOR,
    COMPLIANCE_VERSION_MINOR,
    COMPLIANCE_VERSION_PATCH,
    STNLABZ_MODULE_API_MAJOR,
    STNLABZ_MODULE_API_MINOR,
    compliance_qualify,
    compliance_start,
    compliance_stop
};

const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &descriptor;
}
