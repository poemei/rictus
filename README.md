# Project Rictus

**Rictus — Control Automated Agent**  
**Mission:** mission operations and control


## Identity and lineage

Rictus is a distinct agent derived from Digit's Core foundation. In their agent relationship, **Rictus is Digit's boss**. Rictus's mission is the Mission; he does not inherit Digit's mission-specific modules.

## Architecture and ownership

- **STN-LABZ owns the Core.** Core-owned features, methods, and public APIs use the `stnlabz_` namespace.
- **Rictus owns his mission-specific modules.** Those modules extend the Core through its defined interfaces; they do not govern, replace, or modify Core.
- **Core responsibilities** include the agent foundation, identity validation, lifecycle/runtime, policy and authority evaluation, logging, and generic module infrastructure such as discovery, verification, qualification, activation, and inventory.
- **Mission capabilities** belong in independent Rictus modules, not as hardcoded service-specific behavior within Core.
- Agent architecture is intended to preserve its semantics across platform-specific implementations. Cross-platform implementation and spacecraft qualification remain unproven.



Discussion, research, and speculative planning are not authorization to implement. Human direction determines requirements, changes, validation, and acceptance. Proposed designs should be checked against previously established decisions and revalidated before implementation.
