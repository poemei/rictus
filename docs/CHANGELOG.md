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
- Added `!help` so the live command surface is self-describing, and bounded command argument capture for later handlers.
- Validated identical JOIN event semantics on Windows and Linux from the live Libera.Chat session.
- Validated private-message command dispatch and live `!help` / `!status` replies on Windows and Linux; command matching is case-insensitive.
- Added the top-level `tests/` directory for feature acceptance tests before features are treated as live.
- Restored the Rictus Core bridge to the shared sibling STN-LABZ module ABI and its Core-owned module registry contract from the prior implementation.\n- Restored and modernized the Core-owned qualification inventory contract. Qualification evidence now identifies the exact module artifact in addition to module/version/Core API metadata, so a changed binary cannot inherit prior qualification merely by retaining the same version.\n- Added Core module lifecycle acceptance criteria under `tests/`, including qualification/enablement separation and the prohibition on module self-enablement.

- Documented the bounded Rictus intelligence watch scope and deterministic relevance/evidence threshold.
- Explicitly excluded CISA reporting updates from the Rictus alert feed.

### Changed

- Excluded local credential-bearing `rictus.json` from Git.
- Updated common startup to require a verified TLS session when TLS is configured.
- Windows TLS no longer depends on OpenSSL development headers/libraries or `OPENSSL_ROOT_DIR`.
- Windows TLS now uses the Windows SDK Schannel/security libraries.
- Linux continues to use the already validated OpenSSL TLS implementation.
- Core modernization now reuses sound prior Core engineering where it remains compatible with current requirements rather than rewriting it without cause.

### Validation

- Linux bootstrap/configuration: runtime validated.
- Windows bootstrap/configuration: runtime validated.
- Linux TCP to `irc.libera.chat:6697`: runtime validated.
- Windows TCP to `irc.libera.chat:6697`: runtime validated.
- Linux TLS handshake and certificate/hostname verification: runtime validated.
- Windows Schannel TLS reached Libera.Chat but runtime validation exposed `SEC_I_INCOMPLETE_CREDENTIALS` during the handshake.
- Windows Schannel now disables automatic default client credentials and preserves the same security context across every `InitializeSecurityContext` handshake leg.
- The `SEC_I_INCOMPLETE_CREDENTIALS` path now re-enters Schannel immediately instead of incorrectly waiting for another peer record; the corrected handshake is runtime validated.
- The restored module ABI/registry bridge and qualification inventory are source-integrated only; they are not part of the default executable build and have not been declared live.\n- The qualification inventory is intentionally in-memory at this stage. Persistent storage and native path handling remain a later Core boundary.

### Limitations

- IRC registration, SASL authentication, channel join, and persistent online session are runtime validated on Windows and Linux.
- The initial command path is implemented and runtime validated; intelligence collection/qualification, module loading, runtime state, and logging are not yet live.
- Shared module ABI headers are expected in the sibling `../ABI/includes/` tree, matching the established STN-LABZ layout.

### Deferred

- Core module discovery/loading and native Windows/Linux loader boundaries.\n- Persistent Core qualification storage after the artifact identity and lifecycle semantics are proven.
- IRC migration from executable Core path into its own module after the Core lifecycle is proven.
- Additional command authority and intelligence behavior above the new dispatch layer.
- Runtime-state and logging implementation.
