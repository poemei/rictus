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

### Current staging

- Repository baseline established.
- Core documentation baseline established.
- Cross-platform architecture documented before implementation.
- Common Rictus bootstrap source established.
- Windows MSVC command-line build implemented.
- Linux C17 Makefile build implemented.

### Validation

- Repository/documentation structure reviewed.
- Source/build definitions reviewed for the bootstrap increment.
- Windows and Linux compilation and runtime execution remain pending validation on their target platforms.

### Deferred

Configuration, platform service interfaces, module loading, networking, communications, intelligence behavior, runtime state, and logging remain future increments until explicitly developed and validated.
