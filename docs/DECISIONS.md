# Rictus Decisions

This document separates established direction from proposals and unresolved choices.

## Established

### Cross-platform architecture

Rictus targets Windows and Linux as first-class platforms. Common behavior remains platform-independent. Operating-system-specific behavior uses explicit native platform implementations behind controlled interfaces.

### Language and source baseline

Rictus is implemented in ISO C. Common executable behavior uses `src/` and public declarations use `include/`. Native implementations live beneath `platforms/windows/` and `platforms/linux/` when required.

### Platform networking

Common Rictus code uses the `rictus_net` interface for TCP lifecycle.

- Windows uses native Winsock.
- Linux uses native POSIX sockets.
- Native socket APIs do not enter common Rictus behavior.

TCP connectivity to `irc.libera.chat:6697` has direct runtime evidence on Windows and Linux.

### TLS

Common Rictus code uses the `rictus_tls` interface and does not implement cryptography itself.

- Windows uses native Schannel through the Windows SDK and system certificate infrastructure.
- Linux uses OpenSSL.
- Windows does not require an OpenSSL development installation.
- A successful TCP connection is not treated as TLS success.

Linux OpenSSL TLS has direct runtime evidence including successful certificate and hostname verification. Windows Schannel TLS has direct runtime evidence of a successful handshake and certificate verification against Libera.Chat.

### IRC session

Rictus uses IRCv3 capability negotiation and SASL PLAIN over the already verified TLS transport. Authentication occurs before `CAP END`; successful registration is followed by joining the configured channel. The live session responds to IRC `PING` messages with `PONG`. Credential material is not written to runtime logs. Authenticated SASL registration and joining `#stn-labz` have direct runtime evidence on both Windows and Linux.

### Event boundary

Raw IRC protocol input is parsed into bounded, platform-independent Rictus events before higher-level behavior consumes it. JOIN event semantics from the live Libera.Chat session have direct runtime evidence on Windows and Linux.

### Command boundary

Private IRC messages are deterministically classified before command parsing. `!help` and `!status` have direct live runtime evidence on Windows and Linux, including reply routing back to the private-message sender. Command-name matching is case-insensitive; mixed-case `!staTUS` has direct Linux runtime evidence.

### Configuration

Rictus uses `rictus.json` as its local runtime configuration file with common semantics on Windows and Linux. The local credential-bearing file is not tracked by Git; `rictus.json.example` documents the contract.

### Build entry points

- Windows: `build.cmd`
- Linux: `Makefile`

Windows uses MSVC command-line tools and Windows SDK libraries. Linux uses a C17 compiler selected through `CC`. CMake and the Visual Studio IDE/MSBuild workflow are not required.

### Modules

Rictus supports hot-loadable modules. Platform-native artifacts may differ while module contracts and behavior remain common.

### Historical implementation

The removed prototype and prior implementation are not compatibility requirements for the rebuild.

## Proposed

None currently recorded.

## Open choices

- runtime/state and logging locations;
- module binary/loading contract details;
- final build output layout beyond the current targets.

Open choices remain open until explicitly decided and documented.
