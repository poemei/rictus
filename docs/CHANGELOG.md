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
- Added common TLS send/receive operations for OpenSSL and Schannel.
- Added common IRC registration, SASL PLAIN authentication, channel join, and persistent PING/PONG session handling.
- Added a bounded IRC line parser and common event model for channel/private messages, joins, parts, and quits.
- Added bounded message dispatch that separates explicit `!` commands from passive observations.
- Added the first executable bounded command, `!status`, plus deterministic unknown-command handling and channel/private reply routing.
- Validated identical JOIN event semantics on Windows and Linux from the live Libera.Chat session.

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
- Windows Schannel TLS reached Libera.Chat but runtime validation exposed `SEC_I_INCOMPLETE_CREDENTIALS` during the handshake.
- Windows Schannel now disables automatic default client credentials and preserves the same security context across every `InitializeSecurityContext` handshake leg.
- The `SEC_I_INCOMPLETE_CREDENTIALS` path now re-enters Schannel immediately instead of incorrectly waiting for another peer record; the corrected handshake is runtime validated.

### Limitations

- IRC registration, SASL authentication, channel join, and persistent online session are runtime validated on Windows and Linux.
- Dispatch currently classifies message events only; command execution, intelligence behavior, modules, runtime state, and logging are not implemented.

### Deferred

- Additional command authority and intelligence behavior above the new dispatch layer.
- Runtime-state and logging implementation.
- Module loading implementation and validation.
