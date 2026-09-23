# Rictus Roadmap

## Unreleased

### Established direction

- Cross-platform ISO C with Windows and Linux as first-class targets.
- Common behavior remains platform-independent.
- Native operating-system behavior remains behind explicit platform boundaries.
- Windows builds through `build.cmd`; Linux builds through `Makefile`.
- Runtime configuration uses `rictus.json`.
- TLS uses a mature external implementation rather than custom cryptography.

### Current staging

- Bootstrap and JSON configuration are runtime validated on Windows and Linux.
- Native Winsock and POSIX networking boundary is implemented.
- TCP connectivity to Libera.Chat port 6697 is runtime validated on Windows and Linux.
- OpenSSL TLS client layer is implemented above the transport.
- TLS performs peer verification, SNI, and configured-hostname certificate verification.
- Linux links OpenSSL through `-lssl -lcrypto`.
- Windows build consumes an OpenSSL installation identified by `OPENSSL_ROOT_DIR`.

### Validation

- Linux TCP connection to `irc.libera.chat:6697`: validated.
- Windows TCP connection to `irc.libera.chat:6697`: validated.
- TLS source/build integration: implemented.
- TLS handshake and certificate verification: pending runtime validation on Windows and Linux.

### Next increment

Validate the TLS handshake on both target platforms. Once both report a verified TLS session, add common IRC protocol registration and SASL authentication before joining `#stn-labz`.

### Deferred

IRC registration/authentication/channel join, persistent session behavior, module loading, intelligence behavior, runtime state, and logging remain future increments.
