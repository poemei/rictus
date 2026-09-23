# Rictus Roadmap

## Unreleased

### Established direction

- Cross-platform ISO C with Windows and Linux as first-class targets.
- Common behavior remains platform-independent.
- Native operating-system behavior remains behind explicit platform boundaries.
- Windows builds through `build.cmd`; Linux builds through `Makefile`.
- Runtime configuration uses `rictus.json`.
- TLS is exposed through one common contract with native platform implementations.

### Current staging

- Bootstrap and JSON configuration are runtime validated on Windows and Linux.
- Native Winsock and POSIX networking boundary is implemented.
- TCP connectivity to Libera.Chat port 6697 is runtime validated on Windows and Linux.
- Linux TLS uses OpenSSL and is runtime validated against Libera.Chat with certificate/hostname verification.
- Windows TLS uses native Schannel and the Windows system certificate infrastructure.
- Windows build no longer requires OpenSSL headers, libraries, or `OPENSSL_ROOT_DIR`.

### Validation

- Linux TCP: validated.
- Windows TCP: validated.
- Linux OpenSSL TLS: validated.
- Windows Schannel build: validated.
- Windows Schannel TLS handshake and certificate verification against Libera.Chat: validated.
- The corrected Schannel context/client-certificate handling is runtime validated.

### Current IRC increment

- Common TLS stream I/O is implemented for Linux OpenSSL and Windows Schannel.
- IRCv3 capability negotiation requests SASL before registration completes.
- SASL PLAIN credentials are Base64 encoded in memory and never logged.
- Successful SASL completes capability negotiation, waits for welcome, and joins the configured channel.
- The persistent session answers server PING with PONG.
- Authenticated registration and join of `#stn-labz` are runtime validated on Windows and Linux.

### Current event increment

- Added a bounded, platform-independent IRC message parser.
- IRC wire input is mapped into common Rictus events rather than intelligence behavior consuming protocol text directly.
- Initial event types are channel message, private message, join, part, and quit.
- Events are observation-only in this increment; they do not trigger commands or intelligence actions.
- Identical JOIN event observation is runtime validated on Windows and Linux.

### Current dispatch increment

- Added a bounded dispatch layer above common Rictus events.
- Channel and private messages are deterministically classified as commands when their first non-space character is `!`; other messages become observations.
- Dispatch is classification-only: no command is executed and no intelligence action is authorized in this increment.
- IRC protocol parsing remains isolated below the event boundary.
- Runtime validation of dispatch remains pending.

### Current command increment

- Added the first bounded command parser above dispatch.
- `!status` is the first executable command.
- `!status` reports only established runtime state: Rictus is online with an authenticated active IRC session.
- `!help` reports the bounded command surface currently implemented.
- Unknown commands receive a deterministic unknown-command response.
- Channel commands reply to the channel; private commands reply to the sender.
- Runtime validation is pending on Windows and Linux.

### Next increment

Validate `!status`, `!help`, and unknown-command behavior on both platforms before adding operational command authority.

### Deferred

IRC message handling beyond session maintenance, module loading, intelligence behavior, runtime state, and logging remain future increments.
