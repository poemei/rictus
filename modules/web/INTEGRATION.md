# Rictus Chat Transport Integration

Rictus Web v1.0.2 uses the **same outbound poll/reply channel as Digit**.

The previous local token-gated HTTP listener has been retired. This file
supersedes the earlier implementation notes; see [README.md](README.md).

- Chat receives `rictus: <directive>` and queues it for `agent=rictus`.
- Rictus polls the existing protected `/chat/transport` URL using the
  `poll_rictus` operation.
- Rictus invokes the existing Core Router.
- Rictus posts `reply_rictus` with its request ID and result.
- Chat persists Rictus's reply with role `rictus`.

Digit's original `poll`/`reply` operations are unchanged.

The URL is operator configured in `/opt/rictus/config/web.conf`.
No token, certificate listener, or new public port is part of this flow.
The existing transport's localhost/trusted-proxy access restriction remains
mandatory; the Rictus worker must use the same trusted network access path
as Digit.

Agent-to-agent cross-channel work orders are explicitly out of scope.
