# Contributing to Rictus

Rictus is an ISO C, cross-platform Windows/Linux project.

## Before changing code

Read:

- `README.md`
- `docs/ARCHITECTURE.md`
- `docs/POLICY.md`
- `docs/DECISIONS.md`
- `AGENTS.md`

Preserve the documented distinction between common behavior and platform-specific implementation.

## Portability

Do not place Windows-specific behavior in common code and then emulate it on Linux. Do not place Linux/POSIX-specific behavior in common code and emulate it on Windows.

When operating-system behavior genuinely differs, keep the common contract stable and provide native platform implementations behind the platform boundary.

## Builds

Windows:

```text
build.cmd
```

Linux:

```text
make
```

Do not introduce a CMake requirement or require the Visual Studio IDE/MSBuild workflow.

## Documentation

Meaningful changes must update both `docs/CHANGELOG.md` and `docs/ROADMAP.md`.

Document validation evidence accurately. If something was not built or exercised, do not report it as validated.
