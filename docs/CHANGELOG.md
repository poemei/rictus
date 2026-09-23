# Rictus Changelog

## Unreleased

### Added

- Added baseline architecture and repository documentation for the Rictus rebuild.
- Added the common ISO C Rictus bootstrap.
- Implemented Linux `Makefile` and Windows MSVC `build.cmd` build entry points.
- Added common ISO C loading and validation for `rictus.json`.
- Added the initial IRC configuration contract and Libera.Chat example for `#stn-labz`.
- Added the common `rictus_net` TCP connection contract.
- Added native Winsock networking under `platforms/windows/`.
- Added native POSIX socket networking under `platforms/linux/`.

### Changed

- Removed the broken sponsor badge from the README and documented the cross-platform architecture.
- Excluded local `rictus.json` from Git so IRC credentials are not committed.
- Updated Windows and Linux builds to compile only their native networking implementation.
- Updated the common runtime to connect to the configured IRC host/port through the platform-neutral networking contract and close the connection after success.

### Validation

- Linux bootstrap/configuration runtime validation completed successfully.
- Windows bootstrap/configuration runtime validation completed successfully.
- Networking implementation and build wiring have been reviewed, but TCP connection execution has not yet been validated on either target platform.

### Limitations

- TLS is not yet implemented. A TCP connection to port 6697 alone does not constitute a valid IRC/TLS session.
- Rictus closes the TCP connection immediately after proving transport connectivity.
- IRC registration, authentication, channel joining, session maintenance, modules, runtime state, and logging are not implemented.

### Deferred

- TLS implementation/dependency decision.
- IRC registration, authentication, channel join, and session maintenance.
- Runtime-state and logging implementation.
- Module loading implementation and validation.
