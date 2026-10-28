# Rictus Roadmap

This roadmap preserves continuity. It is **not authorization** to implement a future item.

Human direction determines what moves from planned to active work.

## Current architecture

Rictus is the STN-LABZ Control Automated Agent.

Current message path:

`transport -> Core router -> registered command controller -> optional registered services -> reply/action`

Core owns routing truth, module lifecycle, policy/authority evaluation, logging, hotload, inventory, and DSD diagnostics.

Mission capabilities remain independent modules.

Shared ABI: STN-LABZ Module ABI 1.5.

## Current proven capabilities

### Core Router

- transport-neutral ingress: `rictus.router.dispatch`
- explicit `/route arguments` command dispatch
- ordinary text defaults to `chat`
- commands/services are registered by modules
- Core does not hardcode module-specific routes

### Core DSD

Current foundation:

- bounded last-request trace per sender/account
- route/controller ownership
- nested service path/result capture
- dynamic source correlation against the current source tree
- repo-relative `Code: file:line` evidence when provable
- no SQLite dependency

Operator-observed DSD path:

`foo -> chat -> ollama -> Code: modules/ollama/src/ollama_module.c:135`

### IRC transport

- TLS
- SASL
- channel join/rejoin
- PING/PONG
- Core Router handoff
- channel-send service
- private-send service
- buffered TLS handling via `SSL_pending()`

### Ollama

- local `llama3.2:1b`
- `rictus.llm.generate`
- `chat` controller
- Rictus identity anchoring
- peer-address intent
- peer actions can be delivered independently of the originating DM

### Intelligence

- Threat API polling
- relevance-driven candidate selection
- 60-second polling
- intelligence evidence log for Digit consumption

## Active proving direction

### 1. DSD diagnostic depth

Goal:

`request -> exact runtime path -> failure boundary -> current source location -> repair-ready evidence`

Desired properties:

- survives source/file movement
- follows current registration and runtime evidence
- drills into Core or module code as appropriate
- never invents a source line when correlation cannot be proven
- useful to operator, ChatGPT/Codex, and future repair capability

Possible future diagnostic evidence may include:

- more than the last two service hops in rendered output
- explicit failing symbol/function name
- transport disposition
- module activation/version evidence
- associated Core/module log event references
- structured machine-readable repair handoff

These are continuity items only until separately authorized.

### 2. Rictus supervisory-agent behavior

Direction established by the operator:

Rictus should be able to observe Digit behavior, identify drift/policy problems, and intervene directly.

Conceptual flow:

`observe -> evaluate -> intervene -> verify correction`

Rictus-to-Digit communication should be operational action, not merely conversational text.

Private coordination may be used so public channels show operational results rather than plumbing.

### 3. Conversation/context awareness

Desired agent context model:

- known operator/participant
- conversation identity
- source transport
- current topic/task
- recent relevant context
- known peer agents
- reply destination
- action destination

The purpose is to separate:

`where a request came from`

from:

`where the requested action must go`

Rictus is the proving ground before comparable behavior is considered for Digit.

### 4. Digit parity after Rictus proof

Once Rictus behavior is proven and explicitly authorized for transfer, corresponding Ollama/context behavior may be adapted to Digit.

No Digit change is authorized by this roadmap alone.

### 5. Ezra

Ezra remains intentionally small and compliant.

Rich context awareness does not imply conversational verbosity.

No expansion of Ezra behavior is authorized by this roadmap.

## Deferred / not currently selected

### SQLite persistence

Not selected.

Current DSD design intentionally uses bounded in-memory state.

Persistence should only be introduced if historical request diagnostics become an operator requirement.

### Automated repair

Not implemented.

DSD should first produce repair-ready evidence. A future repair feature may consume that evidence if separately designed and authorized.

### Historical trace database

Not implemented.

### Unbounded agent-to-agent conversation

Not implemented.

Any future autonomous peer conversation should have explicit operational purpose and bounded behavior.

## Acceptance discipline

Each implementation remains open until the operator performs the selected end-to-end test and explicitly accepts it.

Passing builds, module qualification, hotload activation, or a successful isolated test do not by themselves authorize the next roadmap item.
