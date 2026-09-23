# Rictus Configuration

## Status

Configuration is not yet implemented in the rebuilt Rictus codebase.

This document records the configuration contract as it becomes established. It must not be used to invent configuration keys or runtime behavior before those choices are implemented and documented.

## Cross-platform requirement

Configuration semantics will be the same on Windows and Linux. Platform-specific filesystem locations may differ, but configuration meaning must not change merely because the operating system changes.

## Pending definition

The following remain to be established during implementation:

- configuration file format;
- configuration file discovery;
- runtime/state locations;
- log locations;
- module configuration;
- operator and communications settings;
- validation and deterministic error behavior.

Approved configuration decisions will be recorded here and, where architectural, in [DECISIONS.md](DECISIONS.md).
