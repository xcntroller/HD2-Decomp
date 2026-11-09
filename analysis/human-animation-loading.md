# Human animation loading — source continuation

2026-11-05. P2.2 remains open. These owners have native fixture and i386 compile
verification; they are not a constructed DD0/activated Human or a world frame.
No original client, DLL, Windows fixture or Proton was executed. Ledger unchanged.

## Tracks at DD0+260

`recovered/game/human_animation_tracks.h` reconstructs007720C0/007723B0,
007733D0/00773260,007724A0/007724E0,00775260/00775970,00774DF0 and the used
00782EB0/00782FA0 selection helpers. Actual frames/clips are borrowed; containing
callback registration, player query and live0084E80C time scale are mandatory.
The original initialize argument is Human; this extracted interface takes its
actual model atHuman+20. No native Human pointer is encoded in a scalar word.

Original initialization writes SEVEN metadata records but resets EIGHT engine
tracks, and requires the actual model parent. It finds parent/own `a` withFFFF;
own marker registers00774D80 with1000. Clear removes it before eight null track
assignments. CD0 activation initializes this same model twice without an intervening
clear; this is preserved.00782FA0 has four stack arguments/RET16 but writes only
the first three words. Selection ping-pongs each group, preserves original
same-selection reuse/flags2, remaining-time flag4 and normalized-time flag8.

007724E0's first DWORD is unused. Normal updates reset three event counts and
advance eight tracks before transitions. The original casts unsigned elapsed
to float, uses live scale per track and rates only for tracks0..5, then zero
extends the low DWORD of __ftol before reloading it. Actual1.4f gives1399 for1000.
Transition-only updates skip engine advancement; original x87 blend evaluation,
pause/stop/fourth-group fade and sentinel cleanup are preserved.

00774DF0 retains three five-DWORD queues, p/p2 pauses and s/s1/s2's live-scale
debounce. k/u intentionally index queue0 with queue1's count while incrementing
queue0's count. Undefined native overflow rejects instead of writing past capacity.
The separate marker dispatcher below supplies00774D80's sound routing.

## CD0 state

`recovered/game/human_animation_state.h` supplies00757C20/00757DC0/00757DF0/
00757E10/00757F50,00757FB0 and both00758290 branches. The containing owner must
provide the real DD0 scalar view (including its embedded tracks), E0 properties,
shared animation tables, equipment visibility and changed-opacity module receiver.
No default successful endpoint exists. Sector lookup now has its actual host
scene+E0 endpoint inGameActorFrames. Borrowers are separate from scalar pointer slots.

00757C20 resets750/A9C as appropriate, copies actual world positions including
the original fourth word, refreshes equipment, consumes sharedrand()%1000, binds
the actual scene sector/ambient and runs opacity update10000/mode0. Its initial
position setup calls the original motion gate.00757E10 selects table0/1 for
E0+A0=1/2, compact row135 (native65C/stride12), writes clip+8=2, selects
(45,0,1)/(31+[E0+5C!=0],0)/(75,0), updates transition-only1000 then normal1000,
and only then sets byte10. Missing selection table rejects; a null row reference
retains the original no-clip behavior and provides no world readiness.

00757FB0 preserves C4's other bits and tests original animation/stance/sentinel
branches. Read-only DWORD slices at00816AB8/00816B48 and constants are preserved
in `human-animation-motion-data.json`; indexes outside those recovered slices
reject.00758290 preserves actual world position, previous/target/delta and both
distance/span ratio branches from assembly. Undefined scratch/SIMD padding words
7C/AC/BC are not fabricated as recovered values. Clear retires tracks then shadows;
deactivate also clears byte10. Containing borrowers must outlive this owner.

## Verification and next required consumers

Focused ASan/UBSan passes track selection/reuse/sync/pause, all marker codes,
debounce/cross-index/capacity, unsigned/live-scale conversions,13 initialization
and23 operation endpoint failures. CD0 composes those actual tracks in fixtures;
checks double initialization, both tables/null clip, shared random, sector/opacity/
visibility, repeated cleanup, both ratios and motion gates with64 activation
endpoint failures. One fixture initially expected opacity timer0; corrected to
10000 because the already verified update uses the previous timer for its gate.
Strict actual-host i386 instantiation and affected application compile/link pass.
The receipt records fingerprints and temporary-audit cleanup. An interrupted
execution environment removed the earlier track/CD0/spatial temporary sources
before their fingerprints could be recorded; their existing logs remain evidence.

Evidence: `analysis/work/mission-world-20261103/human-animation-loading-abi-verified/`,
`human-animation-event-dependencies/`, `human-animation-owner-binding.asm`,
`human-animation-selection-record.asm`, `human-animation-selection-normalize.asm`,
`human-animation-blend-ratio.asm`, `human-animation-sync.asm`,
`human-animation-update-prefix.asm`, `human-animation-track-events.asm`,
`human-animation-position.asm`, `human-animation-motion-gate.asm` and the data JSON.
The earlier `human-animation-loading-abi/` export stopped at a missing callback
start; use only the later verified ABI export. No repeated broad audit was run.

## Marker dispatch and shared SOUND producer

`human_animation_events.h` supplies00774D80/007759D0 routing: actual parent
frame data queues the DD0 event before the unconditional sound helper. `!` parses
two integers, discards the first and calls speech00436470, passing only an actual
type3 actor. `$` parses filename/near/far and searches the callback frame's `a`;
`@` selects group11 and searches its parent's `a`. The actual005A0890/0069A4B0
view branch applies local forward times original.666f (0081681C). Missing spaces
retain the native no-call path; undefined float/overflow/short-marker reads reject.

`game_human_animation_events.h` registers the actual engine frame+28 callback,
1000 mask, stdcall four arguments/RET16, and null/zero clearing. Typed registry
borrowers do not overwrite actor frame data. Same-track double registration is
permitted; foreign owner replacement rejects. Exceptions are latched inside the
engine callback and rethrown immediately after model advancement, so CD0 cannot
publish active byte10 after a callback failure. Models/Actors/Sound/Frames outlive
this bridge.10 dispatch endpoint failures and that propagation boundary pass
focused ASan/UBSan; actual i386 host/track/bridge composition compiles.

`frontend/spatial_sound.h` supplies00433FA0 and00435B00 hearing traversal with
typed pointer slots. `SoundVoice` now holds original position/group/range/actor/
relative fields as well as the existing pooled stop/update state. Native catalogue
selection consumes the same CRT stream; relative world position is recomputed
after selection. Preserve inclusive far boundary, direct-file flags1, request
flags, live gate, special-mode.4f volume, extension substring, actual primary/
extra sample ownership and group cursor/replacement semantics. Existing empty
frame after refused open and active/no-frame after null factory remain original
branches, never mission readiness. Out-of-range pool/group/cursor/name accesses
reject. Hearing visits registrations in reverse, offsets listenerY by.25f,
preserves near-range scale bypass and calls required00432BE0 for occlusion.

`host/game_spatial_sound.h` forwards actual kind4 factory, category2 shared cache,
frame linkage/local position/range/mode/volume and callback delivery. StartupSession
passes its SAME manager/catalogue/gate/cache to `emit_game_sound`; no second sound
manager or random stream is introduced. Actual actor/module/sector/occlusion World
receivers remain mandatory and uncomposed. The initial strict i386 compile caught
Windows near/far macros; local names were corrected. Focused ASan/UBSan passes
20 endpoint failures plus pool/cache/flags/range/group/hearing cases; strict actual
host i386 instantiation passes. The first catalogue fixture appended after the14
real default groups; corrected to populate group0. No original runtime execution.

SOUND+50 is00436470: it uses16 shared dubbing channels, distinct from emission
+54 and Human's existing20-group sound-frame pool. Continue its actual playback/
close/update dependencies0071DEF0/0071DF30/0071E460/0071E490/0071E7F0/0071E860/
0071ECF0, actual callback actors/world/00432BE0, C00/DD0 composition/shared
presentation and inline98 activation, remaining derived actors/VM/modules, world
cleanup and real first-frame rendering. No speech success stub is installed.

The initial spatial-sound-dependencies export forced two getters to void and
therefore erased their reads; do not use those decompilations. Assembly confirms
005A6420 reads(Human+B0)+50 bit18 and005AC4C0 reads(inline98+8)+CE4. Corrected
return types are in the v2 export. This is new dependency research, not a renewed
audit of previously verified source.

Latest source continuation: actual sixteen-channel dubbing and DD0/AA0/C00/shared presentation/inline98/230 now have focused native/i386 verification. [Current composition contracts and remaining actual World consumers](human-control-loading.md). Earlier next-work statements above describe the previous source boundary; containing Human/world and first frame remain open.
