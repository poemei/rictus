# Rictus Roadmap

## Unreleased

### Established direction

- ISO C targeting Linux on the retained VPS.
- Linux builds through `Makefile`.
- Runtime configuration uses `rictus.json`.
- TLS is exposed through one common contract with native platform implementations.
- Core is completed and feature-tested before operational capabilities are moved into modules.
- IRC will become the first operational module and will be used to prove Core discovery, qualification, human enablement, activation, and runtime behavior.
- Qualification proves compatibility/health; it does not grant operational authority. A qualified module remains disabled until a human enables it.
- No module has authority to enable itself or another module.
- Feature acceptance tests live directly under `tests/`.

### Current staging

- Bootstrap and JSON configuration are runtime validated on Linux.
- Native POSIX networking is implemented.
- TCP connectivity to Libera.Chat port 6697 is runtime validated on Linux.
- Linux TLS uses OpenSSL and is runtime validated against Libera.Chat with certificate/hostname verification.
- Core modernization has begun by restoring the shared STN-LABZ ABI module and registry bridge from the prior Rictus implementation.\n- Core-owned qualification inventory has been modernized around exact artifact identity, preventing changed binaries from inheriting qualification solely from an unchanged semantic version.\n- Module lifecycle acceptance criteria are established directly under `tests/`; executable evidence remains pending.\n- Rictus-owned lifecycle policy now separates preparation/qualification from human-authorized enablement above the shared ABI registry.\n- Linux `make all` compiles the established Core qualification inventory and lifecycle sources and the required sibling ABI module/registry implementation.\n- The restored ABI bridge expects the established sibling `../ABI/includes/` layout and is not yet wired into the default executable build.

### Validation

- Linux TCP: validated.
- Linux OpenSSL TLS: validated.
- The corrected Schannel context/client-certificate handling is runtime validated.
- New Core module work remains pre-live until its feature tests and platform builds provide evidence.

### Current IRC increment

- TLS stream I/O is implemented with Linux OpenSSL.
- IRCv3 capability negotiation requests SASL before registration completes.
- SASL PLAIN credentials are Base64 encoded in memory and never logged.
- Successful SASL completes capability negotiation, waits for welcome, and joins the configured channel.
- The persistent session answers server PING with PONG.
- Authenticated registration and join of `#stn-labz` are runtime validated on Linux.
- This proven IRC implementation is scheduled to move out of executable Core behavior and become the first Core qualification/activation module.

### Current event increment

- Added a bounded, platform-independent IRC message parser.
- IRC wire input is mapped into common Rictus events rather than intelligence behavior consuming protocol text directly.
- Initial event types are channel message, private message, join, part, and quit.
- JOIN event observation is runtime validated on Linux.

### Current dispatch increment

- Added a bounded dispatch layer above common Rictus events.
- Channel and private messages are deterministically classified as commands when their first non-space character is `!`; other messages become observations.
- IRC protocol parsing remains isolated below the event boundary.
- Private-message command dispatch is runtime validated on Linux.

### Current command increment

- Added the first bounded command parser above dispatch.
- `!status` reports only established runtime state.
- `!help` reports the bounded command surface currently implemented.
- Unknown commands receive a deterministic unknown-command response.
- Channel commands reply to the channel; private commands reply to the sender.
- `!help`, `!status`, and mixed-case `!staTUS` private-message replies are runtime validated on Linux.

### Intelligence policy increment

- Established the bounded intelligence watch scope.
- Excluded CISA reporting updates from the Rictus alert feed.
- Established a deterministic relevance/evidence threshold between observation and intelligence candidacy.
- Below-threshold material is ignored and produces no operator report.
- Intelligence collection and qualification implementation remain pending.

### Next increment

Use the IRC module as the first real Core integration fixture. The direct Core IRC startup path has been replaced by native module loading and ABI qualification/start. Next prove this end-to-end path on Linux, then replace the temporary operator-startup enable assumption with persisted Core-owned human enable/disable state and lifecycle enforcement.

### Deferred

IRC module migration remains pending until the Core lifecycle is proven. Intelligence behavior, runtime state, and logging remain future increments.

- Build evidence exposed and removed a Windows-only dependency from the common loader contract. Re-prove Linux Core/module startup before advancing lifecycle persistence.

- Core/IRC ownership cleanup: Linux Core no longer links IRC TLS or network implementations. Re-run the clean build and then exercise module-driven startup.

- IRC now exercises the established Core lifecycle path: load -> discover -> verify -> qualify/restore -> human authority -> activate -> module start. Next: replace temporary artifact identity and process-start human authorization with Core-owned persistent artifact qualification and enable/disable state.

- Core module state persistence is now implemented for qualification evidence and human enable/disable policy. Next prove first-run state creation and second-run qualification restoration on Linux, then replace the temporary declared artifact identity with deterministic content-derived artifact identity so changed `.so` artifacts force requalification.

- Windows support has been removed. Rictus is now developed and deployed as a Linux system on the retained VPS; Windows build parity is no longer a project requirement.

- Qualification-path observability is established: Core explicitly reports fresh qualification versus restored qualification. Next runtime evidence should demonstrate the restored path after restart before artifact identity work begins.

- Core now derives module artifact identity from SHA-256 of the exact shared-object bytes. Next runtime evidence must show: current artifact performs one fresh qualification because the old temporary identity cannot match; immediate restart restores qualification; rebuilding/changing `irc.so` produces a different identity and forces fresh qualification.

- Linux module discovery is now executable Core behavior. Startup scans the modules directory and produces bounded candidates without granting lifecycle authority. IRC is selected from discovery and then follows the existing load/verify/qualify-or-restore/human-enable path. Next prove this discovery-driven startup at runtime before adding change polling/hot deployment.

- Linux hot-deployment observation is now source-integrated using `inotify`. Core establishes the startup candidate snapshot, starts the watcher, and can report new or changed module artifacts while IRC remains online. Next prove runtime detection, then connect detected changes to the lifecycle without granting automatic enable authority.

- [x] Live replacement of changed non-IRC Linux module artifacts: stop, unload, reload, requalify, persist disabled.

- [x] Native Linux installation and systemd daemon deployment (`make install`).
