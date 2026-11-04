# Rictus Web Transport v1.0.1

Web owns Rictus's authenticated HTTPS transport; Core continues to own
routing through `rictus.router.dispatch`. No Core or ABI changes.

## Endpoint

- HTTPS listener on port **18761**, IPv4 `0.0.0.0` (like Digit's
  Interface on 8081). Host firewall MUST restrict inbound access to the
  trusted website server(s). This port is not meant for unrestricted access.
- `POST /ask`, `Content-Type: text/plain`.
- `Authorization: Bearer <token>`, present on every request.
- Body length 1–2047 bytes, one line. Plain-text replies.
- The fixed `web` account label is a transport identity only.
- Only the existing Rictus Core Router decides the conversational destination.

## Server-side TLS configuration

Web **fails closed** if its TLS certificate, key, or token is absent or
invalid. It has no plaintext fallback. Requires:

- `/opt/rictus/auth/web/cert.pem`: PEM certificate chain, with a SAN
  matching the public hostname used by the Chat Module.
- `/opt/rictus/auth/web/key.pem`: matching private key, regular file,
  not symlinked, owner root or Rictus, mode `0600`.
- `/opt/rictus/auth/web/token`: 32–128 lowercase hex characters
  (64 recommended), owner Rictus, mode `0600`.

Use your existing certificate issuance process; do not copy Digit's key
unless the certificate actually covers the requested Rictus hostname.
The Chat Module verifies both certificate trust and hostname.

## Build, qualify, and install on Rictus VPS

```sh
cd modules/web
make
make test
sudo make install
```

The Rictus module hotloader detects and qualifies `web.so`. Test/build
success does not prove the live TLS endpoint works. Check Rictus's
module state and TLS logs before declaring it operational.

## Request from a trusted host

```sh
curl --fail-with-body -sS \
  'https://YOUR-RICTUS-HOST:18761/ask' \
  -H "Authorization: Bearer $RICTUS_WEB_TOKEN" \
  -H 'Content-Type: text/plain' \
  --data-binary 'get in here'
```

Never disable certificate verification, include credentials in browser
JavaScript, or put tokens into GitHub. The ChAoS MVC Chat Module must have
the matching secret and endpoint on its *PHP backend*, using environment
settings or private `chat/config/rictus.local.php`.

## Acceptance checks

- Valid certificate and key: listener starts on TCP 18761.
- Invalid/missing certificate or key: module does not activate.
- Unauthenticated request: rejected.
- Authorized request: reaches Core Router and returns Rictus's response.
- Browser: displays the response as Rictus, never Digit.
- Firewall: only intended backend source addresses can connect.
