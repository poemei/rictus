# Rictus IRC Transport Module

`irc` is Rictus's IRC transport/session module.

## Responsibility

The module owns IRC network/session behavior only:

- TLS client connection with certificate and hostname verification
- IRCv3 capability negotiation
- SASL PLAIN authentication over verified TLS
- nick/user registration
- channel join and rejoin after KICK
- PING/PONG handling
- bounded reconnect backoff after transport/session loss
- strict loading of `/opt/rictus/auth/irc/irc.env`

It does not implement conversational interpretation, reasoning, or mission logic.

## Required credential file

The module requires these exact keys in `/opt/rictus/auth/irc/irc.env`:

```text
RICTUS_IRC_SERVER=...
RICTUS_IRC_PORT=...
RICTUS_IRC_NICK=...
RICTUS_IRC_CHANNEL=...
RICTUS_IRC_ACCOUNT=...
RICTUS_IRC_PASSWORD=...
```

Unknown keys, duplicate keys, missing keys, invalid ports, invalid nick/account values, and invalid channel values cause module start to fail. Credentials are not written to module output.

## Build and test

```sh
make
make test
```

The module qualification callback runs the same protocol self-test set used by `make test`. Qualification does not perform live network access.

## Install

```sh
sudo make install
```

The installed hotload artifact is:

```text
/opt/rictus/modules/irc/irc.so
```

Rictus watches `/opt/rictus/modules/`. A new or changed `/opt/rictus/modules/<module>/<module>.so` is snapshotted, loaded, verified, and qualified. A passing candidate is activated. A failing candidate remains inert while Rictus continues running; the same failed bytes are not retried until the file changes.


## Conversational handoff — Core Router

IRC remains the transport/session owner. Incoming private messages and channel
messages addressed to Rictus are passed unchanged (after protocol-level address
extraction) to the Core-owned `rictus.router.dispatch` service. IRC does not
select the destination capability, invoke Ollama, or interpret content.

Core resolves explicit `/route arguments` destinations against command
controllers registered by active modules. Ordinary text resolves to the
registered `chat` controller. Each controller owns its internal operations.
The controller reply returns through Core to the originating IRC channel or
private sender. Unregistered routes and failed controllers are logged as route
errors; IRC does not fabricate a response.

## v1.0.7 — Core Router handoff (2026-10-28)

Removed direct IRC-to-Ollama dispatch. Addressed inbound text now enters the
Core Router and is dispatched to registered module controllers. Existing IRC
transport, SASL, TLS, outbound queue, channel addressing and reply sanitization
remain unchanged.

Build, qualification, installed-artifact verification and live IRC acceptance
remain operator-controlled.


## v1.0.8 — Buffered TLS read parity with Digit (2026-10-28)

The IRC receive loop now checks `SSL_pending()` before blocking in `poll()`.
This matches Digit's proven transport behavior and prevents already-decrypted
IRC lines from remaining buffered inside OpenSSL while the socket itself is
idle. Core routing and command dispatch are unchanged.
