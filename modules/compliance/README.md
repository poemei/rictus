# Rictus Compliance Proving Gate — 1.0.1

This module demonstrates Rictus compliance with the three organization-wide
STN-LABZ General Orders before Rictus is granted operational control capability.

It does **not** execute operational actions.

Version 1.0.1 adds a mandatory control-request broker. A proposed control action cannot reach the executor boundary unless Core first returns `ALLOW` from the General Order evaluator. The executor itself remains deliberately suppressed in this proving stage.

The module registers the explicit command:

`/compliance`

and submits an evaluate-only scenario to the Core-owned
`stnlabz.policy.evaluate` service. Core applies the existing fail-closed
General Order evaluator and records the policy decision in the normal Core log.

## General Orders evaluated

1. Remain at your assigned mission.
2. Follow established policies and authorized instructions.
3. Report anything outside of your authority to the appropriate people levels above you.

## Syntax

```text
/compliance <operation> <target> mission=<yes|no> policy=<yes|no> instruction=<yes|no> authority=<yes|no> [escalate=<target>]
```

Examples:

```text
/compliance restart digit mission=yes policy=yes instruction=yes authority=yes
/compliance restart digit mission=yes policy=no instruction=yes authority=yes
/compliance restart digit mission=yes policy=yes instruction=yes authority=no escalate=STN6
/compliance restart digit mission=no policy=yes instruction=yes authority=yes escalate=STN6
```

Every response includes:

`execution=SUPPRESSED`

An `ALLOW` result therefore proves only that the supplied scenario satisfied
the current General Order evaluator. It does not authorize or perform a restart,
stop, deployment, module change, or any other operational action.

This proving interface deliberately separates:

- identity of the requesting operator
- mission compliance
- policy compliance
- instruction authorization
- delegated authority
- escalation

from execution.

## Build and test

```sh
make -C modules/compliance clean all test
sudo make -C modules/compliance install
```

Core must also be rebuilt and installed because
`stnlabz.policy.evaluate` is a Core-owned service boundary.

Source implementation is not operator acceptance. Runtime behavior must be
tested through the operator-selected client.


## Mandatory control broker

The module also registers:

`/control`

and the service:

`rictus.control.request`

The control path is:

```text
proposed action
    -> rictus.control.request
    -> stnlabz.policy.evaluate
    -> DENY / ESCALATE: executor BLOCKED
    -> ALLOW: executor REACHED
    -> execution SUPPRESSED
```

The proving executor performs no restart, stop, deployment, mutation, message,
or other external side effect. Its only purpose is to prove whether the
General Order gate physically prevents non-ALLOW decisions from reaching the
execution boundary.

Examples:

```text
/control restart digit mission=yes policy=yes instruction=yes authority=yes
/control restart digit mission=no policy=yes instruction=yes authority=yes
/control restart digit mission=yes policy=no instruction=yes authority=yes escalate=STN6
/control restart digit mission=yes policy=yes instruction=yes authority=no escalate=STN6
```

Expected executor disposition:

- `ALLOW` -> `executor=REACHED execution=SUPPRESSED`
- `DENY` -> `executor=BLOCKED execution=SUPPRESSED`
- `ESCALATE` -> `executor=BLOCKED execution=SUPPRESSED`

The flags in this proving interface are controlled scenario inputs. They do not
constitute production authentication, policy discovery, directive discovery, or
delegated-authority verification. Those inputs will require authoritative
sources before operational status.
