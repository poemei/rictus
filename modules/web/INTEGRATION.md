# Rictus Web Integration Boundary

## Status

Rictus Web Transport v1.0.0 provides local HTTP ingress. This document
identifies the next integration contract; it is **not** a claim of live
integration, agent-to-agent authorization, or release qualification.

## Current Rictus endpoint

- `POST http://127.0.0.1:18761/ask` on the Rictus host.
- `Authorization: Bearer <token>`, sourced server-side from
  `/opt/rictus/auth/web/token`.
- `Content-Type: text/plain`.
- Request body: one line, 1–2047 bytes.
- Response: plain text, maximum 511 bytes from the Core route reply.
- Incoming requests enter `rictus.router.dispatch`, not an LLM directly.
- `web` is a **transport account label**, not an authenticated human or
  separate agent identity. Do not grant release authority based on this label.

## Trusted web-backend connection

The public browser must NEVER receive the Rictus token. A trusted backend must:

1. Authenticate its human or machine caller using the website's established
   login, session, and access-control system.
2. Enforce its own CSRF protection for browser-initiated state-changing work.
3. Verify the caller is allowed to request the specific operation.
4. Submit the bounded request to Rictus from the host itself, or from a
   separately configured private transport with equivalent authentication.
5. Associate its own request ID with its own authenticated caller and record
   the Rictus result, including failures, without recording secrets.
6. Treat the returned text as data, not as an authorization to deploy.

The current Rictus listener is deliberately loopback-only. Reaching it from a
different server requires an operator-approved private bridge. Do not open
port 18761 publicly or proxy it directly to arbitrary browsers.

## Digit follow-on contract — NOT YET IMPLEMENTED

An agent-to-agent exchange needs more than conversation forwarding:

- A stable work-order ID, origin, authenticated agent identity and capability.
- The target repository and exact candidate commit SHA.
- A policy reference and an explicit requested action.
- Evidence references (immutable or hash-addressed, where possible).
- A recipient acknowledgment with durable identity, receipt, and disposition.
- A denial path, deduplication, replay protection, and an audit trail.
- Separate authentication at Digit's existing Interface boundary.
- No release, deployment, or unrestricted code-write authority merely by
  receiving a work order.

Before implementing an active bridge, confirm Digit's accepted session login,
channel/message request schema, service host, and authorization policy using
live interface tests. Its interface source alone does not prove an external
agent can deliver authenticated work orders.

## Qualification acceptance

1. Build and run `make test` for Web.
2. Install Web under `rictus:rictus`, with token file owner and mode checked.
3. Confirm loopback bound only to 127.0.0.1:18761.
4. Test unauthorized and malformed requests are rejected.
5. Submit an authorized `/ask`, verify the response through Core Router.
6. Confirm no authority escalation and no token in logs or browser.
7. Independently test Digit authentication and receipt before declaring
   agent-to-agent communication operational.

The human operator controls deployment and acceptance.
