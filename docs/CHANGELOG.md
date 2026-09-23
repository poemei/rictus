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
- Added common ISO C loading and validation for `rictus.json`.
- Added the initial IRC configuration contract for server, port, TLS, username, password, and channel.
- Added `rictus.json.example` for Libera.Chat and `#stn-labz`.

### Changed

- Removed the broken sponsor badge from the README.
- Documented Windows `build.cmd` and Linux `Makefile` as the platform build entry points.
- Documented the rule that platform-specific behavior remains behind native platform boundaries.
- Replaced the empty build staging placeholders with minimal native Windows and Linux build definitions.
- Updated both build entry points to compile the common configuration loader.
- Excluded local `rictus.json` from Git so IRC credentials are not committed.

### Validation

- Verified the documentation paths referenced by the README are present.
- Reviewed the bootstrap source and both build definitions.
- Reviewed the configuration contract, bounded configuration file size, required fields, port range, and channel validation.
- Linux build/runtime validation completed: Rictus launched, loaded the Libera.Chat configuration for user `rictus` and channel `#stn-labz`, and completed initialization.
- Windows build/runtime validation completed: Rictus launched from `build\windows`, loaded the same IRC configuration, and completed initialization.
- The bootstrap and configuration foundation therefore have direct runtime evidence on both first-class target platforms.

### Limitations

- Rictus currently loads configuration, reports non-secret IRC settings, prints its initialization message, and exits.
- The current JSON reader intentionally supports the small established configuration contract rather than serving as a general JSON library.
- Networking, TLS, IRC authentication, channel joining, platform services, modules, runtime state, and logging are not implemented.

### Deferred

- Native Windows and Linux networking boundary.
- TLS implementation/dependency decision.
- IRC registration, authentication, channel join, and session maintenance.
- Runtime-state and logging implementation.
- Module loading implementation and validation.
