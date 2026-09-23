# Rictus Roadmap

## Unreleased

### Established direction

- Cross-platform ISO C with Windows and Linux as first-class targets.
- Common behavior remains platform-independent.
- Native operating-system behavior remains behind explicit platform boundaries.
- Windows builds through `build.cmd`; Linux builds through `Makefile`.
- Runtime configuration uses `rictus.json`.
- TLS is exposed through one common contract with native platform implementations.
- Core is completed and feature-tested before operational capabilities are moved into modules.
- IRC will become the first operational module and will be used to prove Core discovery, qualification, human enablement, activation, and runtime behavior.
- Qualification proves compatibility/health; it does not grant operational authority. A qualified module remains disabled until a human enables it.
- No module has authority to enable itself or another module.
- Feature acceptance tests live directly under `tests/`.

### Current staging

- Bootstrap and JSON configuration are runtime validated on Windows and Linux.
- Native Winsock and POSIX networking boundary is implemented.
- TCP connectivity to Libera.Chat port 6697 is runtime validated on Windows and Linux.
- Linux TLS uses OpenSSL and is runtime validated against Libera.Chat with certificate/hostname verification.
- Windows TLS uses native Schannel and the Windows system certificate infrastructure.
- Windows build no longer requires OpenSSL headers, libraries, or `OPENSSL_ROOT_DIR`.
- Core modernization has begun by restoring the shared STN-LABZ ABI module and registry bridge from the prior Rictus implementation.
- The restored ABI bridge expects the established sibling `../ABI/includes/` layout and is not yet wired into the default executable build.

### Validation

- Linux TCP: validated.
- Windows TCP: validated.
- Linux OpenSSL TLS: validated.
- Windows Schannel build: validated.
- Windows Schannel TLS handshake and certificate verification against Libera.Chat: validated.
- The corrected Schannel context/client-certificate handling is runtime validated.
- New Core module work remains pre-live until its feature tests and platform builds provide evidence.

### Current IRC increment

- Common TLS stream I/O is implemented for Linux OpenSSL and Windows Schannel.
- IRCv3 capability negotiation requests SASL before registration completes.
- SASL PLAIN credentials are Base64 encoded in memory and never logged.
- Successful SASL completes capability negotiation, waits for welcome, and joins the configured channel.
- The persistent session answers server PING with PONG.
- Authenticated registration and join of `#stn-labz` are runtime validated on Windows and Linux.
- This proven IRC implementation is scheduled to move out of executable Core behavior and become the first Core qualification/activation module.

### Current event increment

- Added a bounded, platform-independent IRC message parser.
- IRC wire input is mapped into common Rictus events rather than intelligence behavior consuming protocol text directly.
- Initial event types are channel message, private message, join, part, and quit.
- Identical JOIN event observation is runtime validated on Windows and Linux.

### Current dispatch increment

- Added a bounded dispatch layer above common Rictus events.
- Channel and private messages are deterministically classified as commands when their first non-space character is `!`; other messages become observations.
- IRC protocol parsing remains isolated below the event boundary.
- Private-message command dispatch is runtime validated on Windows and Linux.

### Current command increment

- Added the first bounded command parser above dispatch.
- `!status` reports only established runtime state.
- `!help` reports the bounded command surface currently implemented.
- Unknown commands receive a deterministic unknown-command response.
- Channel commands reply to the channel; private commands reply to the sender.
- `!help` and `!status` private-message replies are runtime validated on Windows and Linux; mixed-case `!staTUS` is also validated on Linux.

### Intelligence policy increment

- Established the bounded intelligence watch scope.
- Excluded CISA reporting updates from the Rictus alert feed.
- Established a deterministic relevance/evidence threshold between observation and intelligence candidacy.
- Below-threshold material is ignored and produces no operator report.
- Intelligence collection and qualification implementation remain pending.

### Next increment

Continue Core modernization from the prior implementation: module lifecycle, qualification evidence, explicit human enablement, discovery/change recognition, and native Windows/Linux loading boundaries. Prove each feature under `tests/` before treating it as live.

### Deferred

IRC module migration remains pending until the Core lifecycle is proven. Intelligence behavior, runtime state, and logging remain future increments.
