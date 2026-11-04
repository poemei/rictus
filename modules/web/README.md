# Rictus Web Transport v1.0.2

Rictus now uses the same Chat Module **poll/reply** pattern as Digit.
No Rictus-specific bearer token, no inbound public listener, no extra agent
bridge, and no changes to Core or the ABI.

## Flow

1. The authenticated ChAoS MVC user sends `rictus: your request` in Chat.
2. Chat stores the request with `agent=rictus`, state `queued`.
3. Rictus's Web module POSTs `{"type":"poll_rictus"}` to the existing
   `/chat/transport` route.
4. Rictus passes the returned request into `rictus.router.dispatch`.
5. It POSTs `{"type":"reply_rictus","id":"...","status":"ok","message":"..."}`.
6. Chat stores the response as a message from **Rictus**, not Digit.

Digit retains its existing `poll` and `reply` transport operation names and
cannot collect Rictus's jobs. The same source-address / trusted-proxy gate
already protecting Digit's Chat Transport applies to Rictus. This module does
not change or bypass that gate.

## One setting, no token

On the same VPS as Digit, set the **same reachable Chat Transport URL** that
Digit's outbound transport currently uses. Create this local file:

`/opt/rictus/config/web.conf`

Exactly one line:

```ini
transport_url=https://YOUR-CHAT-SERVER/chat/transport
```

The URL must resolve from the VPS through the existing trusted route. The
Rictus process uses HTTPS with certificate and hostname verification. Do not
point it at a public route that returns HTTP 403: that means the existing
trusted transport network path has not been established.

## Build and install

```bash
cd modules/web
make
make test
sudo make install
```

Rictus hotload calls the module's side-effect-free qualification callback,
which exercises 12 payload/encoding cases. This is not proof of a live
round-trip. Check the module status and send a real test directive in Chat.

No token files, TLS listener private keys, or new firewall openings are
required by this module.

## Scope

The module owns the web **transport** only. Rictus's Core Router makes
dispatch decisions. It does not implement cross-channel agent communication,
release approvals or qualification work orders.
