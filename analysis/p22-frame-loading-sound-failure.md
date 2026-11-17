# Actual loading crash: sound scene lookup — 2026-11-17

User ran the isolated FRAME/OWN candidate and reported a loading crash. This
is a failed actual desktop test, not accepted delivery. Original assets, OWN's
uncommitted source and its earlier desktop artifact remain preserved.

The new runtime logs load the Tutorial scene, models and weapons, then end
without a caught C++ error. `steam-0.log` records an access violation at rebuilt
application address **006E954F**, reading address0. This is a rebuilt executable
address, NOT original client function006E954F. Symbolization against the exact
archived executable resolves it to `start_sound_runtime`, inlined sector setup:
`mov eax,[esi+14]` → `mov eax,[eax]` → faulting `mov eax,[eax]`. The latter
reads through null `WorldViews.sound`. The source intentionally leaves that
listener view null before its real later owner; there is no loaded audio view
publication at this stage. No first gameplay frame was reached/accepted.

Original **004328D0**,4328D3..4328E8, instead calls **00698500**, reads the actual
game's scene18, then scene1F8 root sector. The **00432900** callback repeats that
same fresh current-game/scene query at432950..432958 AFTER attaching its metadata.
Neither function queries a camera/listener view. Research index was used before
these static instruction exports; the prior sound source contract already
identified these bodies but collapsed their scene and listener views.

The correction supplies a late `current_sound_scene()` query on the SAME
process-owned DubbingSoundServices, borrowing the existing WorldViews owner/scene
published by GameApplicationState after actual SessionOwner construction. Both
sector queries use it. Missing owner/scene remains a required exception; no
empty scene, null-success return, skipped initialization, invented listener or
readiness publication is added. Existing listener-dependent processing keeps
its distinct contract. This fixes the observed source cause; remaining live
sound listener publication is a separate required integration boundary, not
credited by the sector correction.

Native ASan/UBSan/leaks PASS **38** focused actual WorldViews/build/add/clear-sector
checks with explicit engine enumeration/root/portal endpoints: listener access
is a throwing sentinel, real nonempty metadata is bound/cleared, callbacks query
the fresh current scene, absent/unbound scenes fail, foreign unbind is ignored,
and a partial enumeration keeps its already owned prefix for cleanup. Engine
execution is not simulated as acceptance. Temporary native audit is fingerprinted
and removed. Actual production i386 O2 application/probe builds and stage-only
results are recorded in the receipt; Windows/original/DLL/Proton execution is
performed only by the user, never the agent.

[Receipt](checkpoints/p22-frame-loading-sound-20261117.json) fingerprints the
archived failed executable and three runtime logs, original instructions,
corrected sources, build/log outputs and revised staged candidate. Previous
candidate's receipt stays revision-qualified. The revised candidate must pass
another user loading run; do not mark FRAME/HUM/OWN resolved or commit the
unconfirmed interactive adapter based on this source correction.

## Actual user retest

The user reran the corrected cb657c0c candidate and reported no crash. Fresh logs
confirm sound initialization passed its old fault and the process exited with
result0 after a handled Human footprint texture failure. This verifies removal
of the observed sound crash; it does NOT verify loaded mission/gameplay or the
whole interactive adapter. [Next actual dependency and correction](p22-frame-loading-footprints-source.md)
records the original nonfatal0078A4F0 status RET and exact retained-prefix policy.
