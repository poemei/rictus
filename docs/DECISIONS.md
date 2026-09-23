# Rictus Decisions

This document separates established direction from proposals and unresolved choices.

## Established

### Cross-platform architecture

Rictus targets Windows and Linux as first-class platforms.

Common behavior will remain platform-independent. Operating-system-specific behavior will use explicit native platform implementations behind controlled interfaces.

### Language

Rictus is implemented in ISO C.

### Source baseline

The initial executable uses common source code under `src/` with public project declarations under `include/`. Platform-specific source directories will be introduced only when an implementation actually requires an operating-system boundary.

### Configuration

Rictus uses `rictus.json` as its local runtime configuration file. Configuration semantics are common across Windows and Linux.

The initial IRC configuration contains explicit server, port, TLS, username, password, and channel fields. The local credential-bearing file is not tracked by Git; `rictus.json.example` documents the contract.

### Build entry points

- Windows: `build.cmd`
- Linux: `Makefile`

Windows uses the MSVC command-line compiler. Linux uses a C17 compiler selected through `CC`.

The Visual Studio IDE/MSBuild project workflow is not required. CMake is not required.

### Modules

Rictus supports hot-loadable modules. Platform-native module artifacts may differ while module contracts and behavior remain common.

### Historical implementation

The removed prototype and prior implementation are not compatibility requirements for the rebuild.

## Proposed

None currently recorded.

## Open choices

Implementation details not yet established include:

- platform interface surface;
- TLS dependency;
- runtime/state and logging locations;
- module binary/loading contract details;
- final build output layout beyond the bootstrap targets.

Open choices remain open until explicitly decided and documented.
