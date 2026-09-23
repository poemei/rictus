# Rictus Roadmap

## Unreleased

### Established direction

- Rebuild Rictus as a cross-platform ISO C project.
- Treat Windows and Linux as first-class targets.
- Keep common behavior platform-independent.
- Isolate operating-system behavior behind native platform boundaries.
- Use `build.cmd` as the Windows build entry point.
- Use `Makefile` as the Linux build entry point.
- Preserve hot-loadable module architecture.
- Use `rictus.json` for common runtime configuration.

### Current staging

- Repository and documentation baseline established.
- Common Rictus bootstrap established and runtime validated on Windows and Linux.
- Common JSON configuration loading established and runtime validated on Windows and Linux.
- Native networking platform boundary implemented.
- Windows networking implementation uses Winsock.
- Linux networking implementation uses POSIX sockets.
- Common runtime now attempts a TCP connection to the configured IRC server and closes it cleanly after success.

### Validation

- Bootstrap/configuration behavior has direct runtime evidence on Windows and Linux.
- Networking source and build integration are implemented.
- TCP connection behavior is pending target-platform runtime validation.

### Next increment

After Windows and Linux both demonstrate a successful TCP connection, establish TLS over the common transport. TLS dependency selection remains an explicit decision and must use a mature implementation rather than a custom cryptographic implementation.

IRC registration, authentication, channel joining, and persistent session behavior remain separate work after TLS is validated.

### Deferred

TLS, IRC registration/authentication/channel join, module loading, intelligence behavior, runtime state, and logging remain future increments until explicitly developed and validated.
