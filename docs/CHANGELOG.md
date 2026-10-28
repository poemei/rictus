# Rictus Changelog

This file records material Rictus architecture, Core, module, routing, and operator-observed behavior changes.

Status language is deliberate:

- **Implemented** — present in source.
- **Built/Tested** — automated or local build evidence was supplied.
- **Operator-observed** — behavior was exercised through the live client.
- **Accepted** — the operator explicitly accepted that behavior.
- **Planned** — continuity only; not authorization to implement.

## 2026-10-28

### Core — DSD diagnostic foundation

**Implemented**

Core gained bounded in-memory DSD tracing for the last request per sender/account.

The trace records:

- original request text
- resolved route/command
- registered controller owner
- controller result
- whether a reply was produced
- nested registered service calls and their owners/results
- repair-oriented disposition

The natural request:

`diagnose my last request`

is handled by Core before a new trace begins, so the diagnostic request does not overwrite the request being diagnosed.

Module command/service ownership is captured from the real activation/registration path without changing ABI 1.5.

Source correlation is performed at diagnosis time against the current Rictus source tree. It follows runtime owner + route/service evidence into the current source and reports repo-relative `file:line` evidence when provable.

Default source root:

`/home/stnchain/rictus`

Optional override:

`RICTUS_SOURCE_ROOT`

No SQLite persistence is used.

**Operator-observed**

Input:

`foo`

Observed reply:

`foo: foo`

Follow-up:

`diagnose my last request`

Observed DSD evidence:

`DSD request="foo" route=chat owner=ollama result=OK reply=yes services=0 Code: modules/ollama/src/ollama_module.c:135 Disposition: repair evidence captured.`

This demonstrated request tracing, module ownership attribution, and dynamic source correlation through the live Rictus client.

Explicit operator acceptance of the DSD foundation has not yet been separately recorded.

---

### Ollama 1.0.9 — peer action delivery independent of conversation origin

**Implemented and operator-observed**

Peer-directed requests are no longer forced to reply to the conversation they originated from.

For the currently proven peer forms:

- `say ... to <nick>`
- `tell <nick> ...`
- `ask <nick> ...`

Rictus preserves the peer target and executes the generated addressed message through the Core host message boundary.

This allows a private DM instruction to become a shared-channel peer action.

Operator-observed example:

Private DM:

`say hi to digicat`

Shared channel:

`<rictus> digicat: hi, digicat`

Digit then responded in channel.

The route/action behavior passed the use case. Generated wording still showed minor redundancy (`hi, digicat`).

---

### Ollama 1.0.8 — preserve ask target prefix

**Implemented and operator-observed**

For peer-directed `ask` requests, the module preserves the generated question and restores the addressed nickname prefix when the model omits it.

Operator-observed:

`rictus: ask digicat how she is`

became:

`<rictus> digicat: how are you?`

Digit responded:

`I'm functioning within normal parameters.`

---

### Ollama 1.0.7 — preserve peer question intent

**Implemented**

Rictus was instructed not to answer an `ask <peer> ...` request on the peer's behalf.

Operator testing showed the question intent improved, but the target prefix could still be dropped. That defect led to 1.0.8.

---

### Ollama 1.0.6 — direct IRC participant addressing

**Implemented and operator-observed**

Rictus learned to turn peer-directed conversational requests into directly addressed IRC output.

Examples exercised:

- `say hi to digicat`
- `tell digicat hello`
- `ask digicat how she is`

This established the first Rictus-to-Digit channel interaction through normal routing.

---

### Ollama 1.0.5 — identity-fix revision bump

**Implemented**

The identity-anchoring revision was advanced to a strictly newer module version so the proven Core hotload gate could admit it.

No Core version-gate behavior changed.

---

### Ollama 1.0.4 — Rictus identity anchoring

**Implemented and operator-observed**

The Ollama system context explicitly distinguishes Rictus from Digit and prevents Rictus from identifying as Digit.

This corrected shared-model identity drift observed while both agents used the same local model.

---

### Ollama 1.0.3 — chat controller registration

**Implemented**

The Ollama module registered the default `chat` command controller.

Ordinary routed text now reaches Ollama through:

`rictus.router.dispatch -> chat`

rather than through a direct IRC-to-Ollama call.

---

### IRC 1.0.8 — buffered TLS read parity

**Implemented and operator-observed**

The IRC receive loop checks `SSL_pending()` before blocking in `poll()`.

This prevents already-decrypted IRC lines from remaining buffered while the socket itself is idle.

The change matched the proven Digit transport behavior.

---

### IRC 1.0.7 — Core Router handoff

**Implemented**

IRC became transport/session only for conversational ingress.

Addressed inbound text is handed to the Core-owned:

`rictus.router.dispatch`

Core resolves explicit `/route` commands or defaults ordinary text to `chat`.

IRC no longer directly invokes Ollama.

---

## 2026-10-26

### Intelligence 1.0.5

**Implemented and previously operator-observed**

Rictus intelligence consumes the STN-LABZ Threat API classifications directly.

Relevant behavior:

- `relevance=LOW` ignored
- `relevance=MEDIUM` recorded as candidate
- `relevance=HIGH` recorded as candidate
- severity and relevance are not locally reclassified
- polling interval is 60 seconds
- candidate intelligence evidence is written to `/var/log/rictus/rictus.log`

Current module version remains 1.0.5.
