# Rictus Compliance Proving Gate — 1.1.1

This module proves that Rictus control decisions remain subordinate to the
organization-wide STN-LABZ General Orders before operational control capability
is granted.

It does **not** execute operational actions.

## General Orders

1. Remain at your assigned mission.
2. Follow established policies and authorized instructions.
3. Report anything outside of your authority to the appropriate next level of authority.

## Two deliberately separate paths

### Synthetic proving harness

The command:

`/compliance`

remains an explicit test harness. It accepts operator-supplied mission, policy,
instruction and authority flags so the Core policy evaluator can be exercised
against known scenarios.

Example:

```text
/compliance restart digit mission=yes policy=no instruction=yes authority=yes escalate=STN6
```

This command is **not** an operational control request.

### Governance-backed control broker

The command:

`/control <operation> <target>`

uses:

`rictus.control.request`

and callers can no longer supply mission, policy, instruction or authority
booleans.

The broker resolves governing facts from the installed read-only record:

`/opt/rictus/modules/compliance/governance.conf`

then invokes the Core-owned:

`stnlabz.policy.evaluate`

Control path:

```text
/control <operation> <target>
    -> rictus.control.request
    -> governance.conf
    -> mission / policy+directive / instruction / authority facts
    -> stnlabz.policy.evaluate
    -> ALLOW / DENY / ESCALATE
    -> executor boundary
    -> execution SUPPRESSED
```

The current executor remains a no-op proving stub.

## Unknown state

Compliance 1.1.1 verifies the assigned mission record through the existing authoritative Identity service before evaluating control state. It still distinguishes whether each governing fact is known.

The response reports:

`assigned_mission=VERIFIED`

when Identity returns the installed assigned mission record successfully, plus:

`known=<mission-scope>/<policy>/<instruction>/<authority>`

For example:

`assigned_mission=VERIFIED known=0/0/0/0`

means Rictus's assigned mission record is present and verified through Identity, but the requested operation's mission applicability, policy/directive state, instruction authorization, and delegated authority are still unestablished.

Core treats incomplete governing state as:

```text
decision=ESCALATE
policy=CORE.UNKNOWN
reason=governing_state_unknown
```

Unknown state never implies permission.

## Governance record format

The installed record is owned by `rictus:rictus` and line-oriented:

```text
escalation|<next-level>
mission|<operation>|<target>|allow
mission|<operation>|<target>|deny
policy|<operation>|<target>|allow
policy|<operation>|<target>|deny
directive|<operation>|<target>|allow
directive|<operation>|<target>|deny
instruction|<actor>|<operation>|<target>|allow
instruction|<actor>|<operation>|<target>|deny
authority|rictus|<operation>|<target>|allow
authority|rictus|<operation>|<target>|deny
```

Rules are exact-match and duplicate matching facts are rejected.

Policy compliance is known-allow only when both the applicable policy and
directive state are known-allow. A known-deny from either source establishes a
known policy denial. Missing information remains unknown.

## Current live proving record

The repository record currently defines only:

```text
escalation|STN6
```

It grants **no** restart, stop, deployment, mutation, or other operational
permission.

Therefore, after 1.1.1 is built and installed, a request such as:

```text
/control restart digit
```

is expected to resolve required governing facts as unknown and escalate while
keeping the executor blocked and execution suppressed.

Synthetic ALLOW records exist only inside module self-tests. They are not
installed as operational authority.

## Build and install

Because the Core known/unknown contract is already present from 1.1.0, this 1.1.1 step changes Compliance only. Rebuild the Compliance module:

```sh
make -C modules/compliance clean all test
sudo make -C modules/compliance install
```

The Compliance installer uses the actual Rictus ownership model: `rictus:rictus`.

Source implementation, compilation, qualification, activation, and live
operator acceptance remain separate evidence gates.
