# Rictus Changelog

## Unreleased

### Added

- Added repository architecture/documentation baseline and common ISO C bootstrap.
- Implemented Linux `Makefile` and Windows MSVC `build.cmd`.
- Added common `rictus.json` loading and the Libera.Chat configuration contract.
- Added common `rictus_net` TCP lifecycle with native Winsock and POSIX implementations.
- Added an OpenSSL TLS client layer with peer verification, SNI, and hostname certificate verification.

### Changed

- Excluded local credential-bearing `rictus.json` from Git.
- Updated common startup to establish TCP and, when configured, require a verified TLS session.
- Linux now links OpenSSL through `libssl` and `libcrypto`.
- Windows now requires `OPENSSL_ROOT_DIR` and links the installed OpenSSL libraries.

### Validation

- Linux bootstrap/configuration: runtime validated.
- Windows bootstrap/configuration: runtime validated.
- Linux TCP connection to `irc.libera.chat:6697`: runtime validated.
- Windows TCP connection to `irc.libera.chat:6697`: runtime validated.
- TLS implementation/build integration has been reviewed; TLS runtime success is not yet claimed.

### Limitations

- TLS currently closes immediately after a successful verified handshake.
- Windows requires a usable OpenSSL development installation to compile this increment.
- IRC registration, SASL authentication, channel joining, persistent session behavior, modules, runtime state, and logging are not implemented.

### Deferred

- IRC registration and SASL authentication.
- Join and maintain `#stn-labz`.
- Runtime-state and logging implementation.
- Module loading implementation and validation.
