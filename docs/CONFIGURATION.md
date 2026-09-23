# Rictus Configuration

## Status

Rictus now loads its initial runtime configuration from `rictus.json` in the working directory.

The local `rictus.json` file is excluded from Git because it contains the IRC account credential. A credential-free `rictus.json.example` is tracked as the configuration template.

## IRC configuration

The current contract is:

```json
{
  "irc": {
    "server": "irc.libera.chat",
    "port": 6697,
    "tls": true,
    "username": "Rictus",
    "password": "",
    "channel": "#stn-labz"
  }
}
```

Fields:

- `server`: IRC server hostname.
- `port`: IRC server TCP port, 1 through 65535.
- `tls`: whether the IRC connection requires TLS.
- `username`: Rictus IRC account/nickname used by the future IRC session.
- `password`: IRC authentication credential. Rictus does not print this field during normal configuration reporting.
- `channel`: channel Rictus will join. The current validator requires a channel beginning with `#`.

All fields are required in this first configuration contract. The password must be non-empty in the local runtime configuration.

## Cross-platform requirement

Configuration syntax and meaning are identical on Windows and Linux. Configuration loading is common ISO C code and does not use a platform compatibility shim.

## Current scope

This increment loads and validates configuration only. It does not establish a network connection, TLS session, IRC registration, authentication, or channel join.

Runtime/state locations, log locations, module configuration, and additional deterministic validation remain to be established as their corresponding capabilities are implemented.
