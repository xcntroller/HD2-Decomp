# World sound and subtitle source — 2026-11-09

P2.2 remains open. These required receivers extend the existing shared SOUND,
DubbingSound and game+60 MissionText owners. The host still declines loading;
no actual mission/world/desktop delivery is claimed.

`frontend/sound_portal_routing.h` reconstructs00432BE0 using actual SoundServices
sector/portal endpoints. Missing sectors resolve from copied positions. Equal
sectors return distance directly; root crossing invokes existing00433330 and
multiplies its returned distance by the output attenuation as well. Other
sectors inspect side0 then side1 portals in reverse. A direct link wins;
otherwise the first shared neighbour found by reverse lookup wins. This is
at most two hops, with no shortest-path search. Closed direct links scale.5.
On a closed second edge the original repeats the FIRST edge weight, yielding
.125 for two closed edges. No match returns MAXFLOAT and scale1; range is unused.

Same/direct/first-side shared paths spill XYZ differences to FLOAT and sum
(X²+Z²)+Y²; the second-side shared branch keeps differences extended and sums
(X²+Y²)+Z². All square values spill to FLOAT before tiny/near-one gates. Same
sector returns extended sqrt; portal hits spill the result to FLOAT. Root
crossing retains the existing00433330 contract. The host spatial adapter now
calls this source receiver instead of requiring a missing World routing method.
Human sector/current camera/special-mode consumers remain containing-world work.

`game/mission_subtitles_loading.h` reconstructs0048EB70/0048F7D0/0048F420 against
the live Files resolver and actual Text\\<language>\\Dialogs.bin prefix. Root9C4
contains version9C5 (must equal1) and record9C6: FIRST identity9C7/DWORD, THEN
text9C8/NUL string within999 bytes. Unknown root/chunks are skipped as native;
unsupported/truncated or unsafe string access rejects. Each entry starts cursor
and idFFFFFFFF and owns duration/text pairs; loader duration is byte length*200.
Records retain native order/duplicates. Version/record failure leaves preceding
entries owned, and prevents renderer activation. No movie subtitle format is used.

Initialization uses the SAME GameOwnerBackend.make_hud_text shared font factory,
then style8 or26 and enabled1. Actual containing activation and wide glyph
rendering remain separate; existing renderer coverage is byte-font source.
0048EE50 uses the original signed dimensions/QWORD coefficients, full-viewport
choice, seven geometry DWORDs and dirty flags (00644B80). Show always resets
selected index.0048F080 auto-shows and selects the FIRST matching entry, minimum
1000ms.0048F180 speech selection does NOT auto-show; case-sensitive NOSUB clears
presentation. A supplied unsigned sound duration uses1.2000000476837158f then
ftol low32 and signed1500ms minimum; FFFFFFFF keeps the entry duration.

0048F2D0 decrements with DWORD wrapping, advances ONE row when signed timer<=0,
discards overshoot and does not reapply the first-row minimum. End resets actual
selection/renderer records. The current shared string survives independently
of renderer/entry ownership.0048F390 gates enabled/visible/index/renderer/profile
subtitles and restores graphics state0 after real text drawing. Strict actual
GameOwnerBackend/GameFonts/TextState/ChunkFiles instantiation passes; this does
not publish a live world, wire DubbingSound's containing World, or prove desktop
text rendering.

Original exports: ignored durable analysis/work/mission-world-20261103/
spatial-sound-dependencies-v2/00432be0.{c,json}, sound-portal-routing.asm,
mission-subtitles-required/, mission-subtitle-records-required/ and the
mission-subtitle-*.asm excerpts. Original client SHA256 remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Captured English Dialogs.bin is183120 bytes/3250 records, SHA256
90cd8cd8416b38917c2630f983366467445621938b074fad25a8ba0fdcd8fff0;
read from the existing isolated startup staging tree, without altering assets.

Native ASan/UBSan/LSan checks cover directed/duplicate/closed/unreachable routes,
actual spatial hearing delivery, captured subtitle parsing, failed/missing file
close, retained partial entries, shared font lifetime, select/speech/NOSUB,
one-row timing, geometry and draw-state failure restoration. Engine/sector/Files/
glyph endpoints are explicit doubles. Actual-host i386 composition and app link
pass; no Windows fixture or original game/DLL/Proton is executed. The combined
[receipt](checkpoints/p2.2-world-presentation-20261109.json) also records script
rotation verification and temporary fixture hashes/removal.

Continue FULL P2.2 with actual containing world/session/Human/FPV/HUD/SOUND
composition, required actor/physics families1/3/4/6, missionA8/zone37 consumers
and needed first-update script receivers. Retained save-only deferred animation
queue append0042607B belongs to004257B0 save restoration; it is not an unfilled
initial-loading producer. Actual world rendering/failure/repeat/user acceptance
remain required. Do not implement successful empty services for those boundaries.
