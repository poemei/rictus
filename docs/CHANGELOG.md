# Rictus Changelog

## Unreleased

### Added

- Added repository architecture/documentation baseline and common ISO C bootstrap.
- Implemented Linux `Makefile` and Windows MSVC `build.cmd`.
- Added common `rictus.json` loading and the Libera.Chat configuration contract.
- Added common `rictus_net` TCP lifecycle with native Winsock and POSIX implementations.
- Added the common `rictus_tls` contract.
- Added Linux OpenSSL TLS.
- Added native Windows Schannel TLS.

### Changed

- Excluded local credential-bearing `rictus.json` from Git.
- Updated common startup to require a verified TLS session when TLS is configured.
- Windows TLS no longer depends on OpenSSL development headers/libraries or `OPENSSL_ROOT_DIR`.
- Windows TLS now uses the Windows SDK Schannel/security libraries.
- Linux continues to use the already validated OpenSSL TLS implementation.

### Validation

- Linux bootstrap/configuration: runtime validated.
- Windows bootstrap/configuration: runtime validated.
- Linux TCP to `irc.libera.chat:6697`: runtime validated.
- Windows TCP to `irc.libera.chat:6697`: runtime validated.
- Linux TLS handshake and certificate/hostname verification: runtime validated.
- Windows Schannel TLS implementation/build integration: pending runtime validation.

### Limitations

- TLS currently closes immediately after a successful handshake.
- IRC registration, SASL authentication, channel joining, persistent session behavior, modules, runtime state, and logging are not implemented.

### Deferred

- IRC registration and SASL authentication.
- Join and maintain `#stn-labz`.
- Runtime-state and logging implementation.
- Module loading implementation and validation.
