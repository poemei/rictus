# Rictus Architecture

## Status

Rictus is being rebuilt as a cross-platform ISO C project targeting Windows and Linux.

## Core rule

Cross-platform is an architectural requirement, not a later porting task.

Common Rictus behavior, protocols, configuration semantics, module contracts, and other platform-independent logic will remain common. Operating-system-specific behavior will be isolated behind explicit platform boundaries.

Windows will use native Windows implementations where operating-system behavior is required. Linux will use native POSIX/Linux implementations. Common code will not depend on a compatibility layer that makes Linux imitate Windows or Windows imitate Linux.

## Build boundary

- Windows build entry point: `build.cmd`
- Linux build entry point: `Makefile`
- Language: ISO C
- Visual Studio IDE/MSBuild project workflow is not required.
- CMake is not a project requirement.

The build systems may differ where the operating systems require it, but they build the same Rictus architecture and common behavior.

## Platform boundary

Platform-independent code must not embed operating-system-specific paths, APIs, handles, socket types, dynamic-library conventions, or filesystem assumptions.

Platform-specific implementations may provide services such as:

- filesystem and path handling;
- networking;
- dynamic module loading;
- process/runtime integration;
- timing and other operating-system services.

Those implementations must expose controlled interfaces to common code. Platform differences must not change Rictus semantics unless an explicit architectural decision documents the difference.

## Modules

Rictus supports hot-loadable modules. Module behavior and contracts are platform-independent. The native module artifact may differ by platform, such as a Windows DLL or Linux shared object.

Modules must use Rictus platform interfaces for operating-system services rather than embedding platform-specific behavior in common module logic.

## External interfaces

External ABI and dependency details will be documented as they are established during the rebuild. No removed prototype is a compatibility requirement.

## Change control

Established direction, proposed architecture, and unresolved choices are tracked in [DECISIONS.md](DECISIONS.md). Meaningful implementation changes are recorded in [CHANGELOG.md](CHANGELOG.md) and [ROADMAP.md](ROADMAP.md).
