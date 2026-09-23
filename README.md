![Language](https://img.shields.io/badge/Language-ISO%20C-blue)
![Status](https://img.shields.io/badge/Status-Active%20Development-orange)
![Architecture](https://img.shields.io/badge/Architecture-Cross--Platform-blue)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux-blue)
![Core Qualification](https://img.shields.io/badge/Core%20Qualification-00%2F20-warning)
![Modules](https://img.shields.io/badge/Modules-Hot--Load-success)

**STN-LABZ Communications and External Intelligence Coordination Agent**

Rictus provides the controlled communications and command-and-control
boundary between STN-LABZ operators, approved external intelligence
sources, and internal STN-LABZ systems.

> What does STN-LABZ need to know before it becomes important?

## Architecture

Rictus is a cross-platform ISO C project targeting Windows and Linux.
Common behavior remains platform-independent. Operating-system-specific
behavior belongs behind explicit platform boundaries rather than compatibility
shims that make one operating system imitate another.

Windows builds use `build.cmd`. Linux builds use `Makefile`.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Configuration](docs/CONFIGURATION.md)
- [Installation](docs/INSTALL.md)
- [Policy](docs/POLICY.md)
- [Decisions](docs/DECISIONS.md)
- [Roadmap](docs/ROADMAP.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Changelog](docs/CHANGELOG.md)
- **License:** Proprietary — Copyright © 2026 STN-LABZ. All rights reserved.
