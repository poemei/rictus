# Core Module Lifecycle Acceptance

These acceptance cases define the Core behavior that must be demonstrated before
the module lifecycle is treated as live. They are deliberately independent of
IRC or any other operational module.

## State invariant

Core treats these as separate facts:

    PRESENT != COMPATIBLE != QUALIFIED != ENABLED

Qualification is evidence about one exact module artifact. Enablement is human
authorization and is never implied by qualification.

## Acceptance cases

1. A newly discovered compatible module has no inherited qualification.
2. A module that passes its required internal qualification package becomes
   QUALIFIED but is not activation-authorized.
3. A qualification failure cannot be enabled or activated.
4. Persisted qualification evidence is accepted only for the exact module
   identity, version, required Core API, and artifact identity.
5. Replacing an artifact invalidates qualification even when its module ID and
   semantic version are unchanged.
6. Restoring qualification does not restore activation authorization.
7. Activation without explicit human authorization fails.
8. Human authorization can be granted only to a qualified module.
9. A module cannot grant activation authority to itself or another module.
10. Disabling/stopping a module removes operational capability without
    manufacturing new qualification evidence.
11. An incompatible ABI fails before activation.
12. Malformed or corrupt module evidence is rejected deterministically.

## Evidence required

Implementation is not considered live merely because it compiles. These cases
must be exercised by executable acceptance tests once the Core lifecycle and
native loader boundaries are available to the test harness.


## Core policy surface established

The Rictus-owned lifecycle layer now defines two separate operations:

- `rictus_module_lifecycle_prepare()` performs discovery, ABI verification,
  exact-artifact qualification restoration when evidence matches, or fresh
  qualification when it does not.
- `rictus_module_lifecycle_enable()` accepts only
  `RICTUS_MODULE_AUTHORITY_HUMAN` before translating that decision into ABI
  activation authorization and activation.

This source contract is not runtime evidence. Executable acceptance remains
required before the lifecycle is declared live.

## Runtime observability

Core lifecycle preparation reports whether qualification was freshly executed or restored from exact persisted evidence. Runtime evidence must distinguish these paths; a generic qualified message is not sufficient proof of restoration.

## Artifact identity

Core computes SHA-256 over the exact module artifact bytes before lifecycle preparation. Qualification evidence is keyed to that 64-character lowercase digest. A changed artifact with the same module ID and semantic version must not inherit the previous artifact's qualification.

## Hot-deployment observation

While an operational module is running, Core watches the Linux module directory. A new `.so` must be reported as a deployment; a known module whose SHA-256 changes must be reported as an artifact change. Detection alone must not grant qualification, enablement, or activation authority.
