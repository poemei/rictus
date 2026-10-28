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


## Core DSD diagnostics

Rictus Core retains a bounded in-memory last-request diagnostic trace per sender.
The trace follows the actual runtime path through Core routing, registered command
ownership, nested service invocations, result codes, and reply/action disposition.

The natural request `diagnose my last request` is handled by Core before a new
trace is started, so the diagnostic request does not overwrite the request being
diagnosed.

Command and service ownership is captured from the module activation/registration
path without changing ABI 1.5. Source correlation is performed at diagnosis time
against the current Rictus source tree. The default source root is
`/home/stnchain/rictus`; `RICTUS_SOURCE_ROOT` may override it. Reported code
locations are repo-relative, for example:

`modules/ollama/src/ollama_module.c:143`

Source correlation follows current registration symbols to their handler
definitions when provable. No static module-to-file map and no SQLite persistence
are used.


## Documentation continuity

Project continuity is maintained under [docs/](docs/):

- [Changelog](docs/CHANGELOG.md)
- [Roadmap](docs/ROADMAP.md)
- [Module and runtime map](docs/MODULES.md)

Source and live runtime evidence remain authoritative when documentation and implementation differ.
