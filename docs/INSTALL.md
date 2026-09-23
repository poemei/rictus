# Rictus Installation

## Status

Rictus is in active development. Installation procedures are not yet finalized.

## Supported development targets

### Windows

Rictus will build from an ordinary Command Prompt using:

```text
build.cmd
```

Standalone MSVC C++ Build Tools and the Windows SDK are sufficient. The Visual Studio IDE and MSBuild project workflow are not required.

### Linux

Rictus will build using:

```text
make
```

The root `Makefile` is the Linux build entry point.

## Current limitation

The build entry-point files currently exist as staging placeholders. Build commands, dependencies, output layout, and installation paths will be documented when those increments are implemented and validated.

Do not treat an unimplemented build or installation step as supported merely because it is described as planned.
