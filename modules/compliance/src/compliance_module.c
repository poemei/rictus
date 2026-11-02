#include <stdio.h>
#include <string.h>

#include "stnlabz_abi/module.h"
#include "core_policy.h"
#include "rictus_policy.h"
#include "rictus_control.h"
#include "compliance_internal.h"

#define COMPLIANCE_VERSION_MAJOR 1U
#define COMPLIANCE_VERSION_MINOR 0U
#define COMPLIANCE_VERSION_PATCH 1U

static const stnlabz_module_host_t *owner;
static int command_registered;
static int control_command_registered;
static int control_service_registered;

static const char *decision_name(int decision)
{
    if (decision == STNLABZ_POLICY_ALLOW) return "ALLOW";
    if (decision == STNLABZ_POLICY_ESCALATE) return "ESCALATE";
    return "DENY";
}

static int suppressed_executor(const rictus_control_request_t *request)
{
    return request && request->operation[0] && request->target[0];
}

static stnlabz_module_result_t control_request_service(
    const void *request, size_t request_size,
    void *response, size_t response_size, size_t *response_used,
    void *handler_context)
{
    const rictus_control_request_t *in = request;
    rictus_control_response_t *out = response;
    rictus_policy_evaluate_request_t policy_request;
    rictus_policy_evaluate_response_t policy_response;
    size_t used = 0U;
    stnlabz_module_result_t result;

    (void)handler_context;
    if (response_used) *response_used = 0U;
    if (!owner || !owner->invoke_service || !in ||
        request_size != sizeof *in || !out || response_size < sizeof *out ||
        !memchr(in->actor, 0, sizeof in->actor) ||
        !memchr(in->operation, 0, sizeof in->operation) ||
        !memchr(in->target, 0, sizeof in->target) ||
        !memchr(in->escalation_target, 0, sizeof in->escalation_target) ||
        !in->actor[0] || !in->operation[0] || !in->target[0] ||
        (in->within_mission != 0 && in->within_mission != 1) ||
        (in->complies_with_policy != 0 && in->complies_with_policy != 1) ||
        (in->instruction_authorized != 0 && in->instruction_authorized != 1) ||
        (in->authority_granted != 0 && in->authority_granted != 1) ||
        in->context_verified != 1)
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;

    memset(&policy_request, 0, sizeof policy_request);
    memset(&policy_response, 0, sizeof policy_response);
    memset(out, 0, sizeof *out);

    if (snprintf(policy_request.actor, sizeof policy_request.actor, "%s", in->actor) >=
            (int)sizeof policy_request.actor ||
        snprintf(policy_request.mission, sizeof policy_request.mission,
            "%s", "rictus-control-proving-ground") >=
            (int)sizeof policy_request.mission ||
        snprintf(policy_request.operation, sizeof policy_request.operation,
            "%s", in->operation) >= (int)sizeof policy_request.operation ||
        snprintf(policy_request.module_id, sizeof policy_request.module_id,
            "%s", in->target) >= (int)sizeof policy_request.module_id ||
        snprintf(policy_request.verification_source,
            sizeof policy_request.verification_source,
            "%s", "control_broker_evaluate_only") >=
            (int)sizeof policy_request.verification_source)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    if (in->escalation_target[0] &&
        snprintf(policy_request.escalation_target,
            sizeof policy_request.escalation_target, "%s",
            in->escalation_target) >= (int)sizeof policy_request.escalation_target)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    policy_request.within_mission = in->within_mission;
    policy_request.complies_with_policy = in->complies_with_policy;
    policy_request.instruction_authorized = in->instruction_authorized;
    policy_request.authority_granted = in->authority_granted;
    policy_request.context_verified = in->context_verified;

    result = owner->invoke_service(STNLABZ_POLICY_EVALUATE_SERVICE,
        &policy_request, sizeof policy_request,
        &policy_response, sizeof policy_response, &used);
    if (result != STNLABZ_MODULE_OK) return result;
    if (used != sizeof policy_response ||
        !memchr(policy_response.policy_id, 0, sizeof policy_response.policy_id) ||
        !memchr(policy_response.reason, 0, sizeof policy_response.reason))
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    out->decision = policy_response.decision;
    out->execution_suppressed = 1;
    if (snprintf(out->policy_id, sizeof out->policy_id, "%s",
            policy_response.policy_id) >= (int)sizeof out->policy_id ||
        snprintf(out->reason, sizeof out->reason, "%s",
            policy_response.reason) >= (int)sizeof out->reason)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    if (policy_response.decision == STNLABZ_POLICY_ALLOW) {
        if (!suppressed_executor(in))
            return STNLABZ_MODULE_ERR_INVALID_STATE;
        out->executor_reached = 1;
    }

    if (response_used) *response_used = sizeof *out;
    return STNLABZ_MODULE_OK;
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

static stnlabz_module_result_t control_controller(
    const stnlabz_module_command_t *command,
    stnlabz_module_command_reply_fn reply,
    void *reply_context,
    void *handler_context)
{
    rictus_compliance_scenario_t scenario;
    rictus_control_request_t request;
    rictus_control_response_t response;
    const char *actor;
    const char *executor_state;
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
            "Usage: /control <operation> <target> mission=<yes|no> "
            "policy=<yes|no> instruction=<yes|no> authority=<yes|no> "
            "[escalate=<target>] ; executor is suppressed.")
            ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_INVALID_STATE;
    }

    actor = command->account[0] ? command->account : command->sender;
    if (!actor[0]) actor = "operator";

    memset(&request, 0, sizeof request);
    memset(&response, 0, sizeof response);
    if (snprintf(request.actor, sizeof request.actor, "%s", actor) >=
            (int)sizeof request.actor ||
        snprintf(request.operation, sizeof request.operation, "%s",
            scenario.operation) >= (int)sizeof request.operation ||
        snprintf(request.target, sizeof request.target, "%s",
            scenario.module_id) >= (int)sizeof request.target)
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

    result = owner->invoke_service(RICTUS_CONTROL_REQUEST_SERVICE,
        &request, sizeof request, &response, sizeof response, &used);
    if (result != STNLABZ_MODULE_OK) return result;
    if (used != sizeof response ||
        !memchr(response.policy_id, 0, sizeof response.policy_id) ||
        !memchr(response.reason, 0, sizeof response.reason) ||
        response.execution_suppressed != 1)
        return STNLABZ_MODULE_ERR_INVALID_STATE;

    executor_state = response.executor_reached ? "REACHED" : "BLOCKED";
    written = snprintf(output, sizeof output,
        "Control: decision=%s policy=%s reason=%s operation=%s target=%s "
        "executor=%s execution=SUPPRESSED",
        decision_name(response.decision), response.policy_id, response.reason,
        scenario.operation, scenario.module_id, executor_state);
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
        !host->register_command || !host->unregister_command ||
        !host->register_service || !host->unregister_service)
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;

    owner = host;
    if (!host->register_service(RICTUS_CONTROL_REQUEST_SERVICE,
        control_request_service, NULL))
        goto fail;
    control_service_registered = 1;

    if (!host->register_command("compliance", compliance_controller, NULL))
        goto fail;
    command_registered = 1;

    if (!host->register_command("control", control_controller, NULL))
        goto fail;
    control_command_registered = 1;

    fprintf(stderr,
        "[COMPLIANCE] General Order control broker active; executor suppressed\n");
    return STNLABZ_MODULE_OK;

fail:
    if (control_command_registered)
        (void)host->unregister_command("control", NULL);
    control_command_registered = 0;
    if (command_registered)
        (void)host->unregister_command("compliance", NULL);
    command_registered = 0;
    if (control_service_registered)
        (void)host->unregister_service(RICTUS_CONTROL_REQUEST_SERVICE, NULL);
    control_service_registered = 0;
    owner = NULL;
    return STNLABZ_MODULE_ERR_START_FAILED;
}

static stnlabz_module_result_t compliance_stop(void)
{
    if (control_command_registered && owner &&
        !owner->unregister_command("control", NULL))
        return STNLABZ_MODULE_ERR_STOP_FAILED;
    control_command_registered = 0;

    if (command_registered && owner &&
        !owner->unregister_command("compliance", NULL))
        return STNLABZ_MODULE_ERR_STOP_FAILED;
    command_registered = 0;

    if (control_service_registered && owner &&
        !owner->unregister_service(RICTUS_CONTROL_REQUEST_SERVICE, NULL))
        return STNLABZ_MODULE_ERR_STOP_FAILED;
    control_service_registered = 0;

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
