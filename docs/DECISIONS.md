# Rictus Decisions

This document separates established direction from proposals and unresolved choices.

## Established

### Cross-platform architecture

Rictus targets Windows and Linux as first-class platforms.

Common behavior will remain platform-independent. Operating-system-specific behavior will use explicit native platform implementations behind controlled interfaces.

### Language

Rictus is implemented in ISO C.

### Build entry points

- Windows: `build.cmd`
- Linux: `Makefile`

The Visual Studio IDE/MSBuild project workflow is not required. CMake is not required.

### Modules

Rictus supports hot-loadable modules. Platform-native module artifacts may differ while module contracts and behavior remain common.

### Historical implementation

The removed prototype and prior implementation are not compatibility requirements for the rebuild.

## Proposed

None currently recorded.

## Open choices

Implementation details not yet established include:

- final source-tree layout;
- platform interface surface;
- dependency set;
- configuration format and discovery;
- runtime/state and logging locations;
- module binary/loading contract details;
- build output layout.

Open choices remain open until explicitly decided and documented.
