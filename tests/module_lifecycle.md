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
