# Rictus Changelog
- Added native Linux `make install` deployment with systemd service support, installed configuration under `/etc/rictus`, persistent Core state under `/var/lib/rictus`, and modules under `/usr/local/lib/rictus/modules`.
- Linux hot deployment now safely stops, unloads, reloads, requalifies, and disables changed non-IRC module artifacts pending human authority.

## Unreleased

### Added

- Added repository architecture/documentation baseline and common ISO C bootstrap.
- Implemented the Linux `Makefile` build.
- Added common `rictus.json` loading and the Libera.Chat configuration contract.
- Added common `rictus_net` TCP lifecycle with native POSIX implementation.
- Added the common `rictus_tls` contract.
- Added Linux OpenSSL TLS.
- Added common TLS send/receive operations for OpenSSL and Schannel.
- Added common IRC registration, SASL PLAIN authentication, channel join, and persistent PING/PONG session handling.
- Added a bounded IRC line parser and common event model for channel/private messages, joins, parts, and quits.
- Added bounded message dispatch that separates explicit `!` commands from passive observations.
- Added the first executable bounded command, `!status`, plus deterministic unknown-command handling and channel/private reply routing.
- Added `!help` so the live command surface is self-describing, and bounded command argument capture for later handlers.
- Validated JOIN event semantics on Linux from the live Libera.Chat session.
- Validated private-message command dispatch and live `!help` / `!status` replies on Linux; command matching is case-insensitive.
- Added the top-level `tests/` directory for feature acceptance tests before features are treated as live.
- Restored the Rictus Core bridge to the shared sibling STN-LABZ module ABI and its Core-owned module registry contract from the prior implementation.\n- Restored and modernized the Core-owned qualification inventory contract. Qualification evidence now identifies the exact module artifact in addition to module/version/Core API metadata, so a changed binary cannot inherit prior qualification merely by retaining the same version.\n- Added Core module lifecycle acceptance criteria under `tests/`, including qualification/enablement separation and the prohibition on module self-enablement.\n- Added the Rictus-owned Core lifecycle policy above the shared ABI registry. Core now has an explicit prepare boundary for exact-artifact qualification and a separate human-authority enable boundary.\n- Added the Core qualification inventory and lifecycle sources to Linux `make all`.\n- Corrected the build dependency exposed by Linux linking: Rictus now compiles and links the required shared sibling ABI `module.c` and `module_registry.c` sources instead of consuming declarations without their implementation.\n- Restored the shared ABI module-loader bridge and established a platform-independent discovery contract. Discovery now produces bounded module candidates only; it does not load, qualify, enable, or activate them.\n- Established IRC as the first real ABI-facing Rictus module. Linux `make all` produces `build/linux/modules/irc.so`. The IRC module owns configuration, native transport, TLS, SASL, session, event, dispatch, and command runtime behavior. Core loads the IRC shared library through the native Linux loader and starts it through the ABI descriptor.

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
- Linux TCP to `irc.libera.chat:6697`: runtime validated.
- Linux TLS handshake and certificate/hostname verification: runtime validated.
- The `SEC_I_INCOMPLETE_CREDENTIALS` path now re-enters Schannel immediately instead of incorrectly waiting for another peer record; the corrected handshake is runtime validated.
- The restored module ABI/registry bridge and qualification inventory are source-integrated only; they are not part of the default executable build and have not been declared live.\n- The qualification inventory is intentionally in-memory at this stage. Persistent storage and native path handling remain a later Core boundary.\n
### Limitations

- IRC registration, SASL authentication, channel join, and persistent online session are runtime validated on Linux.
- The initial command path is implemented and runtime validated; intelligence collection/qualification, module loading, runtime state, and logging are not yet live.
- Shared module ABI headers are expected in the sibling `../ABI/includes/` tree, matching the established STN-LABZ layout.

### Deferred

- Native Linux implementation of the platform-independent module discovery contract and loader mechanics.\n- Persistent Core qualification storage after the artifact identity and lifecycle semantics are proven.
- IRC migration from executable Core path into its own module after the Core lifecycle is proven.
- Additional command authority and intelligence behavior above the new dispatch layer.
- Runtime-state and logging implementation.

- Corrected the common module-loader boundary after Linux build evidence exposed a Windows header dependency in the shared loader header. Rictus now defines a platform-neutral loader contract and keeps native handles/mechanics inside Windows/Linux implementations.

- Linux link evidence exposed two leftover IRC transport objects in the Core object list. Removed `tls_openssl.o` and `rictus_net_linux.o` from the Core executable; those implementations belong only to `irc.so` after the module ownership migration.

- Routed IRC startup through the Core module registry, qualification inventory, lifecycle preparation, and HUMAN authority gate. Core no longer calls the IRC qualification callback directly or bypasses activation authorization before module start. Artifact identity and enable/disable persistence remain intentionally temporary/in-memory pending the next Core state increment.

- Added Core-owned persistent module state for qualification evidence and human enable/disable policy. Linux startup now restores exact recorded qualification evidence through the lifecycle path and consults persisted authorization before activation. Local runtime state lives under ignored `state/`. Artifact identity remains explicitly temporary until content-derived artifact identification is implemented.

- Removed Windows build support, Win32 networking/TLS/module-loader sources, and Windows-specific build artifacts from Rictus. This deployment is now Linux-only on the retained VPS.

- Core lifecycle preparation now reports whether qualification was freshly executed or restored from persisted evidence. Runtime logging no longer makes those two paths observationally ambiguous.

- Replaced the temporary IRC artifact identity with a Core-computed SHA-256 over the exact `irc.so` bytes. Persisted qualification is now keyed to the actual module artifact, so a changed binary cannot restore qualification solely by retaining the same module ID/version.

- Implemented native Linux module-directory discovery. Core now scans `build/linux/modules`, accepts bounded `.so` candidates, derives each candidate ID from its filename, computes its exact SHA-256 artifact identity, and selects IRC from discovered candidates rather than a hardcoded artifact path.

- Added Linux `inotify` hot-deployment observation. While IRC is running, Core watches the module directory and deterministically reports newly dropped `.so` candidates or changed artifact identities. This increment detects deployment changes only; it does not yet stop/unload/requalify/activate a changed live module.
