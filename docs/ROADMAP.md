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

### Next increment

Build the first bounded IRC message-handling behavior above the now-validated cross-platform session foundation.

### Deferred

IRC message handling beyond session maintenance, module loading, intelligence behavior, runtime state, and logging remain future increments.
