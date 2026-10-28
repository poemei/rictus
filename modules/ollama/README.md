# Rictus Ollama LLM Module

Rictus-specific local LLM transport.

- Endpoint: `http://127.0.0.1:11434/api/generate`
- Model: `llama3.2:1b`
- Service: `rictus.llm.generate`\n- Command controller: `chat`
- No credential/config file is required.
- Ollama owns generation only; it does not own Rictus state, authority, IRC, or intelligence.

Build and test:

```sh
make clean all test
sudo make install
```

Installed artifact:

```text
/opt/rictus/modules/ollama/ollama.so
/opt/rictus/modules/ollama/module.conf
```


## v1.0.3 — Chat controller registration (2026-10-28)

The module now registers the default `chat` command controller with Core.
Ordinary routed text reaches this controller through `rictus.router.dispatch`;
the controller invokes the existing generation implementation and returns the
reply through Core. Ollama still owns generation only and does not own IRC or
Core routing.


## v1.0.4 — Rictus identity anchoring (2026-10-28)

The Ollama system instruction now explicitly distinguishes Rictus from Digit:
Rictus is the STN-LABZ Control Automated Agent; Digit is a separate STN-LABZ
terrestrial AI agent. The model is instructed never to identify itself as Digit
or speak as Digit. A source-level regression check verifies that these identity
anchors are present in generated Ollama requests.

Routing, IRC transport, model selection, temperature and generation limits are
unchanged.
