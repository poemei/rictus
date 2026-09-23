# Rictus Changelog

## Unreleased

### Added

- Added baseline architecture documentation for the Rictus rebuild.
- Added configuration, installation, policy, decisions, roadmap, contributing, and changelog documents.
- Added explicit cross-platform Windows/Linux architecture and platform badges to the README.
- Added README documentation links for decisions and roadmap.
- Added the first common ISO C Rictus bootstrap in `src/main.c` and `src/rictus.c`.
- Added the public bootstrap declaration in `include/rictus.h`.
- Implemented the Linux `Makefile` build entry point.
- Implemented the Windows MSVC command-line `build.cmd` build entry point.

### Changed

- Removed the broken sponsor badge from the README.
- Documented Windows `build.cmd` and Linux `Makefile` as the platform build entry points.
- Documented the rule that platform-specific behavior remains behind native platform boundaries.
- Replaced the empty build staging placeholders with minimal native Windows and Linux build definitions.

### Validation

- Verified the documentation paths referenced by the README are present.
- Reviewed the bootstrap source and both build definitions for this increment.
- Target-platform compilation and runtime execution have not yet been performed and are not claimed.

### Limitations

- The bootstrap currently prints the Rictus banner and initialization message, then exits successfully.
- Configuration, platform services, modules, networking, runtime state, and logging are not implemented.

### Deferred

- Platform service interface implementation.
- Configuration and runtime-state implementation.
- Module loading implementation and validation.
- Networking and communications implementation.
