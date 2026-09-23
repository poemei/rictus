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

- Repository baseline established.
- Core documentation baseline established.
- Cross-platform architecture documented before implementation.
- Common Rictus bootstrap source established.
- Windows MSVC command-line build implemented and runtime validated.
- Linux C17 Makefile build implemented and runtime validated.
- Common JSON configuration loading and IRC configuration validation implemented and runtime validated on Windows and Linux.
- Local credential-bearing `rictus.json` excluded from source control.
- Libera.Chat and `#stn-labz` represented in the tracked configuration example.

### Validation

- Linux: executable launched successfully, loaded `rictus.json`, reported the expected non-secret Libera.Chat settings, and completed initialization.
- Windows: executable launched successfully from `build\windows`, loaded `rictus.json`, reported the same expected settings, and completed initialization.
- Cross-platform bootstrap/configuration behavior now has direct runtime evidence on both first-class target platforms.

### Next increment

Establish Rictus's first platform service boundary for networking:

- common networking contract used by Rictus;
- native Winsock implementation on Windows;
- native POSIX socket implementation on Linux;
- TCP connection to the configured IRC server and port;
- deterministic connection success/failure reporting.

This increment stops at a verified TCP connection. TLS, IRC registration, authentication, channel joining, and persistent session behavior remain separate work so each boundary can be validated independently.

### Deferred

TLS, IRC registration/authentication/channel join, module loading, intelligence behavior, runtime state, and logging remain future increments until explicitly developed and validated.
