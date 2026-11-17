# Reached script FadeIn/FadeOut — 2026-11-17

Source-only correction after d3f1fe4. Actual revised user run b4708c75 passes
Door dispatch and the loaded1/status5 activation, Scene78 render, fixed-update,
input and full-update prefix. A later required command160 CSC_FadeIn exits with
handled result1. This is not stable game/scene/control/presentation acceptance.
Exact executable and three logs:WORK/first-frame-script160-return.
[Verification receipt](checkpoints/p22-frame-script-fade-20261117.json).

Original00416E10 translates opcodeA0 through byte47 at0041CE5B to0041BFD1.
Paired opcodeA1 translates byte48 to0041BFF7. Both read operand0's integerA0
DWORD as signed duration, obtain actual GEffects00486510, and call virtual8C
00486B30 with null colour. FadeIn pushes targetfalse; FadeOut targettrue.
Both converge at0041C8BC, then guarded00416CF0 at0041C8C6 and returntrue.
They configure immediately and advance; no added blocking, timer or duration
conversion. Negative and zero durations preserve existing native fade semantics.

The existing render-command bridge now dispatches A0/A1 to the SAME actual
EffectsFade owner already borrowed by process/update/render/shutdown. There is
no copied state, synthetic successful service or new presentation owner. Actual
00487240 update and00486C10 render remain unchanged. The source retains authority
and guarded-PC behavior, rejects malformed operands and does not advance after a
required receiver throws. AD/BB behavior stays unchanged.

The shared-tail assembly also disproved the prior Door null/wrong-actor claim.
004179A2/B0/BA/C4 branch to0041C8C6: they skip mutation AND integer evaluation,
then guarded-advance and returntrue. The previous source returnedfalse; that was
an interpretation error. Corrected source preserves this exact skip/advance and
only returns nullopt for another opcode. Actual type6/linked/key/packet work is
unchanged. The earlier Door receipt remains revision-qualified; this checkpoint
supersedes its missing/wrong actor expectation. No default unknown-command success.

54 native ASan/UBSan/leak checks use actual MissionScriptContext, EffectsFade,
render_loading_effects_fade and actual Door/packet owners, with explicit frame/
reference leaves. They cover both opcode targets, signed/zero durations, colour
XYZ reset and unspecified retainedW, same-state update/render, completion sentinel,
startup fade precedence, authority/guarded-PC/failure ordering and Door missing/
wrong-target skip without reading an intentionally invalid integer operand.
The actual i386 O2/Werror application/probe build and stage-only candidate are
fingerprinted in the receipt. Temporary fixtures are fingerprinted then removed.
OWN dirty sources/old artifact remain untouched; recoverable full review patch
includes tracked and untracked headers. Interactive overlay stays uncommitted.
No agent execution of original game/DLLs/Proton/Windows fixtures.

Research index preceded reads. Existing typed00416E10 and00486B30/00486C10
exports were reused; raw dispatch/translation and shared-tail evidence are under
WORK=analysis/work/p22-frame-closure-20261117. Original PE SHA256:
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
No new inventory entries/promotions;00416E10 and FRAME/HUM/OWN remain partial.
Next is actual revised desktop continuation and its first concrete failure.
Existing captured Tutorial scripts call CSC_RunCutscene after delay; that owner
remains required (index lookup retained), not silently bypassed or claimed ready.
