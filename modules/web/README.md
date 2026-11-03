# Rictus Web Transport Module

Web is an independently owned **transport**, not a mission controller.
It routes authenticated, bounded web input through the existing
`rictus.router.dispatch` service (the same Core boundary used by IRC).
No Core or ABI 1.5 changes are required.

## Security boundary

- Listens exclusively on IPv4 loopback `127.0.0.1:18761`.
- Accepts only `POST /ask` with `Content-Type: text/plain` (or no content type).
- Requires `Authorization: Bearer <token>` on every request.
- Expects a 32-128 character lowercase hexadecimal token at
  `/opt/rictus/auth/web/token`, owned by `rictus`, with permissions `0600`.
- Maximum request body: 2047 bytes. No chunked encoding, multiline commands,
  or alternate HTTP methods.
- The Core route sender/account is currently the fixed `web` identity.
  **Do not treat this shared gateway token as an end-user identity.**
- Does not expose an external listener, a browser UI, or any new execution,
  release, or operational authority.
- Deployment and qualification remain human-controlled.

## Build and qualification

```sh
cd modules/web
make
make test
sudo make install
```

Rictus watches `/opt/rictus/modules` and qualifies new module artifacts.
The qualification callback tests request parsing, authentication failures,
invalid methods, duplicate headers, unsupported transfer encoding, and limits.
Those tests do **not** establish live HTTP or release-qualification acceptance.

## Token provisioning

On the Rictus host, as an authorized operator:

```sh
sudo install -d -o rictus -g rictus -m 0700 /opt/rictus/auth/web
openssl rand -hex 32 | sudo tee /opt/rictus/auth/web/token >/dev/null
sudo chown rictus:rictus /opt/rictus/auth/web/token
sudo chmod 0600 /opt/rictus/auth/web/token
```

Keep the token out of the repository, browser JavaScript, URLs and logs.

## Request example (on Rictus host only)

```sh
TOKEN=$(sudo cat /opt/rictus/auth/web/token)
curl --fail-with-body -sS http://127.0.0.1:18761/ask \
  -H "Host: 127.0.0.1:18761" \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: text/plain" \
  --data-binary 'diagnose my last request'
unset TOKEN
```

A trusted web backend can make this request server-side, but should not pass
untrusted browser session identity directly to Rictus as authenticated agent
authority. Agent-to-agent work orders and independent release qualification
require separate authorization and evidence contracts; they are **not**
implemented by this transport module.
