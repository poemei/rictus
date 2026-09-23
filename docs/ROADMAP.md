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
- Runtime validation is pending on Windows and Linux.

### Next increment

Validate identical event observation on Windows and Linux, then place bounded command/intelligence dispatch above the event contract.

### Deferred

IRC message handling beyond session maintenance, module loading, intelligence behavior, runtime state, and logging remain future increments.
