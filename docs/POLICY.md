# Rictus Development Policy

## Scope

This document records repository-level engineering policy for Rictus.

## Engineering baseline

Rictus is a cross-platform ISO C project targeting Windows and Linux.

Cross-platform behavior is designed into the architecture. Common code must remain portable. Operating-system-specific behavior belongs behind explicit platform boundaries.

The project favors small, deterministic, understandable implementations and evidence from actual builds and runtime behavior over assumptions.

## Build policy

- Windows uses `build.cmd`.
- Linux uses `Makefile`.
- Visual Studio IDE/MSBuild workflow is not required.
- CMake will not be introduced as a requirement.
- Platform-specific build mechanics must not create different application semantics.

## Change tracking

Every meaningful code, documentation, build, or configuration change must update both:

- `docs/CHANGELOG.md`
- `docs/ROADMAP.md`

Unreleased work remains under **Unreleased** until a release is explicitly established.

## Architectural decisions

Established direction, proposals, and open choices must remain distinguishable. Architectural decisions and unresolved choices are tracked in `docs/DECISIONS.md`.

## Validation

Claims that something builds, runs, connects, loads, or otherwise works require corresponding evidence. Unsupported functionality must be described as pending rather than implied to exist.
