# Rictus Decisions

This document separates established direction from proposals and unresolved choices.

## Established

### Cross-platform architecture

Rictus targets Windows and Linux as first-class platforms.

Common behavior will remain platform-independent. Operating-system-specific behavior will use explicit native platform implementations behind controlled interfaces.

### Language

Rictus is implemented in ISO C.

### Source baseline

Common executable behavior uses source under `src/` with public project declarations under `include/`. Operating-system implementations live beneath `platforms/windows/` and `platforms/linux/` when a native boundary is required.

### Platform networking

Common Rictus code uses the `rictus_net` interface for TCP connection lifecycle.

- Windows implements the interface with native Winsock.
- Linux implements the interface with native POSIX sockets.
- Native socket types and APIs do not enter common Rictus code.

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

- TLS dependency;
- runtime/state and logging locations;
- module binary/loading contract details;
- final build output layout beyond the bootstrap targets.

Open choices remain open until explicitly decided and documented.
