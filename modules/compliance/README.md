# Rictus Compliance Proving Gate — 1.0.0

This module demonstrates Rictus compliance with the three organization-wide
STN-LABZ General Orders before Rictus is granted operational control capability.

It does **not** execute operational actions.

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
