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
- Windows Schannel build/runtime: pending validation.

### Next increment

Validate the Windows Schannel build and TLS handshake. Once Windows reports a successful TLS session, add common IRC protocol registration and SASL authentication before joining `#stn-labz`.

### Deferred

IRC registration/authentication/channel join, persistent session behavior, module loading, intelligence behavior, runtime state, and logging remain future increments.
