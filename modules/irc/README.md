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

The installed layout matches Core discovery:

```text
/opt/rictus/bin/modules/irc/module.conf
/opt/rictus/bin/modules/irc/bin/irc.so
```

Installation does not grant activation authority.

## Current integration boundary

The current Rictus Core startup path discovers, verifies, and qualifies modules but intentionally activates none. Therefore this module can be built, installed, discovered, and qualified without connecting to IRC until Core receives separately authorized activation wiring.
