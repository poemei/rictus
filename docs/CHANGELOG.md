# Rictus Changelog

## Unreleased

### Added

- Added baseline architecture documentation for the Rictus rebuild.
- Added configuration, installation, policy, decisions, roadmap, contributing, and changelog documents.
- Added explicit cross-platform Windows/Linux architecture and platform badges to the README.
- Added README documentation links for decisions and roadmap.

### Changed

- Removed the broken sponsor badge from the README.
- Documented Windows `build.cmd` and Linux `Makefile` as the platform build entry points.
- Documented the rule that platform-specific behavior remains behind native platform boundaries.

### Validation

- Verified the documentation paths referenced by the README are present after this increment.
- No build or runtime validation is claimed; `build.cmd` and `Makefile` remain staging placeholders.

### Limitations

- Rictus implementation is not yet established in this rebuild.
- Build commands, dependencies, configuration, and runtime behavior remain pending.

### Deferred

- Platform interface implementation.
- Windows and Linux build implementation.
- Configuration and runtime-state implementation.
- Module loading implementation and validation.
