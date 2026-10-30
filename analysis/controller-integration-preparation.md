# Next delivery: concrete application states and controller integration

Implementation follow-up: [controller/state delivery](controller-integration.md)
is now user desktop-confirmed. This document preserves the preceding research
and the remaining playback/subtitle contracts; statements below about missing
native wiring describe the preparation checkpoint.

Research checkpoint, 2026-10-30, after desktop acceptance and commit `47e768d`.
The first milestone0 item is complete for the bounded startup executable. This
document prepares the second item; it does not mark it implemented or tested.
No Windows executable, DLL or Proton was executed by the agent.

## Recommended implementation boundary

Connect the real startup session to `ApplicationController`, with owning video
and front-end states. One executable must progress from Logo1 through Intro into
the existing compiled Main/Exit menu, with controller calls owning input, fixed
update, update, rendering and replacement. A second host loop or a state that
merely reports success while the old preview runs does not meet this boundary.

Reuse the existing desktop-confirmed menu products and engine movie adapter.
The full original eager fourteen-screen registry, additional menu actions,
original class ABI, registry/setup.exe wrapper and integrity VM retain their
separate documented limitations. Do not claim that wrapping the two-screen menu
recovers the whole original front-end initializer. Unavailable builders must
continue to reject; do not install success callbacks for their original hooks.

Music/subtitles/profile persistence are real calls inside the video owner and
menu state, not optional successful services. Their contracts and dependencies
are recorded below so the third delivery item can follow without rediscovery.
If the second item uses the already-confirmed ordinary playback subset, identify
that scope explicitly; credit no omitted initializer/shutdown body as complete.

## First fix: the empty input buffers have a concrete cause

The desktop startup log had bindings0 and input-words0/0. This is a native
constructor omission, not evidence that the original launches without bindings.

- Application constructor006B4A60 calls006A81C0 at006B4BC9 while constructing
  profile+EC's binding configuration.
- Constructor006A81C0 clears its vector and explicitly writes mode2 at006A81D4.
  Native `BindingConfiguration::mode` currently defaults to0. The owning
  `ApplicationStartupData` does not restore the original value.
- Default reset006A8BE0 and loaded-record application006A86C0 deliberately refuse
  installation outside modes2/3. Their recovered behavior is correct.
- Input query006C72B0 returns-1 for an empty selected vector. Controller006C31D0
  queries both views and calls state+0C only if both results are nonnegative.
  Video input00688BB0 reads IGraph+ A0 itself, but is never reached through this
  controller if either binding buffer is empty.

Restore mode2 at the actual native constructor/owner boundary before profile
loading; do not add dummy buffer entries or call the state input out of band.
Keep explicit mode0 tests where they exercise the existing invalid-mode gate.
Fresh layout0/mode2 creates63 manager words in each buffer, including Escape at
binding46; mode3 creates64. A temporary ASan/UBSan audit reproduced the current
zero-output case and verified the63/63 result using the original constructor
value, real recovered default installer and controller input getter.

## Startup and shared owners

`StartupSession::after_devices()` currently performs both device/resource setup
and prepare/reset. Split those phases without changing their order: controller
006C0820 must call real preparation006B65D0 once, then reset006B6620 once, then
initial-video006C0CA0. Do not call `after_devices()` in full and repeat these
operations through controller services. Preserve early game.cfg construction,
gate/locks, archive registration, directory trees, input/text setup, then ascending
module initialization006DBFB0→0048DA60 before controller start.

Expose actual borrowed session owners to the controller adapter: input manager,
profile/options, localized text/language, files, shared gate, sound manager and
its catalog/clock. Do not copy the gate or create a second sound settings owner.
Controller mouse request is actual IGraph+DC(self,1); delay expiry006C2133 loads
ECX from008AE91C before006B32A0, so it reapplies the actual profile sound volume.
It is not an arbitrary timer-completed callback.

The normal alternate-start test006C0AF0 obtains `GetCommandLineA()` and searches
case-sensitive substrings ` -cmd ` first, then ` -c`. It clears controller+14
before the normal false return. Decode the command line; reject a reached
unimplemented console branch explicitly rather than supplying always-false.
Keep launcher flags free of accidental matching substrings.

`MenuScene` currently owns/initializes `MenuDriver` internally. The startup probe
already owns an initialized driver before controller start. Add borrowed-driver
scene construction with explicit lifetime; retain the existing owning preview
constructor. Close all state frames before the single driver, sound and graphics
owners. A second initialization/close pair at movie-to-menu entry is wrong.

The keyboard is already initialized by input setup006DB620→IGraph+90(self,0).
`EngineKeyboard` initializes again and intentionally refuses an already-owned
device. Do not construct it unchanged in the new session. Borrow the initialized
device through checked IGraph polling/scan/character methods, or extend the native
poller with explicit borrowed ownership. Use one snapshot/translation state per
device; independently polling twice loses previous/current edges.

## Concrete state contracts

Offsets below describe original evidence, not a required native memory layout.
Both states use status at+04 and a separate pending word at+08, initially2.
Base getter0067EA00 returns+04; +24 setter0067EA10 writes+04; +28 getter0067EA20
and +2C setter0067EA30 access+08. After-update0067EA40 copies pending to status
only if pending!=2, then restores pending2. These small methods are actual
bodies, not assumed default interface behavior.

| Slot | Startup video, final table00815BC8 | Front end, final table00815E30 |
| --- | --- | --- |
| 00 destroy | 006C0DC0→006C0DE0 | 006C1660→006C1680 |
| 04 initialize | 00688040 | 0066AD10 |
| 08 shutdown | 0066ED30 | 0066B140 |
| 0C input | 0066ED50→00688BB0 | 0066B3E0 |
| 10 fixed update | 0067EC60, actual RET4 | 0066BE90→virtual4C, actual RET4 at0067EAA0 |
| 14 update | 0066ED80→00688C10 | 0066BEA0 |
| 18 render | 0066EDA0→00688F50 | 0066DE00 |
| 1C type | 006C0DB0 returns10 | 006C1650 returns2 |
| 20..30 | Shared status/pending methods above | Same |

Video allocation006C0CA0 is0x84/alignment16, retained prior owner+80=null.
Initialize known fields individually: no whole-object zero claim. Helper0066EAE0
sets sequence+7C, creates an owned0x1C descriptor (type5, strings+10/+14 both the
filename, flag18=0), initializes the video, then sets status1 through+24. Its
returned AL is zero; callers ignore it. Do not treat that as open failure.

Input/update wrappers promote base status2 to controller status13. Natural
completion or failed IShow open therefore uses the same controller transition.
Controller owns Logo1→Logo2→Logo3→Intro and calls shutdown before the next movie.
On index4 completion006C0EF0(null), shutdown and delete the old owner, construct
the front end, install it before initialize(nullptr), ignore initializer result.
Do not retain `OpeningPlayback`'s separate sequence-advancement loop under this
controller; factor its ordinary single-movie operations instead.

Native shutdown and native destruction need separate responsibilities. Video
shutdown0066ED30 performs00688460 then deletes/clears descriptor+70. Destructor
006C0DE0 frees subtitle/audio vector storage and string owner, not another full
shutdown. Controller replacement must not close/release twice. The same applies
to menu shutdown0066B140 versus destructor006C1680. Native exception cleanup
must still reclaim resources acquired before initialization throws.

## Frame order and input consumption

Original006DCAE0, with default byte00843470=1 and normal shared gate7:

1. Pump IGraph+14; poll keyboard+9C; query scan+A0 for screenshot handling.
2. Obtain recovered frame-clock delta006DC950 (existing native frame clock).
3. Poll mouse+E8, reverse-poll global joystick list+58(1), then manager
   produce(false)006C7870. Do not poll those devices again inside the producer.
4. I3D driver+0C(self,delta), then controller render006C3190 when gate2 is set.
5. Control queue phase0072C3B0(delta,1), controller fixed update006C3100 when
   gate1 or gate==12 hex; queue phase0072C3B0(delta,0), then controller
   update006C20F0 when gate2 (or its explicit alternate condition).
6. Control maintenance0072CE70(delta), deferred application actions, then
   IGraph+70 present when gate2. Preserve render-before-update/present-after-update.

Screenshot scan query is not an innocuous cached observation. Flags0 keyboard
scan1006F320 scans current/previous without consuming the scan; flags2 uses a
cleared FIFO latch. Reuse the actual setup flags and existing preprocessing
contracts. Both the video input and menu character/scan preprocessing read
IGraph themselves. Avoid consuming scans in an extra host Escape handler.

Controller fixed update subtracts100 only once; suspension still consumes that
tick. Render has no additional suspension gate. Services must reread the current
state after callbacks, as existing `ApplicationController` does. The finalizers
0072A0B0/0072B1C0 are bypassed by their real gate7 predicates, not no-op methods.

Control frame hooks must not become fake services. Existing constructor0072DE00
creates empty queues at1F0/1FC (counts1F8/204=0);0072C3B0 takes real lock430 and
returns if selected count is zero.0072CD50 checks32 records whose constructor
byte08 is zero, so their active-peer branch is absent in cold launch. Maintenance
0072C5D0 advances the real5058 accumulator against5054=50 and checks stream
lengths.0072CA10 initializes its3000ms timer with enabled word0;0073EA90 therefore
returns false until explicitly armed. Gate7 bypasses connection timeout/stats
branches, but does not justify a permanent whole-service no-op. Reconstruct this
cold ownership/maintenance path; guard unsupported populated queues/interfaces
and mode changes until their actual producers/consumers are implemented.

## Menu state: what the preview can and cannot supply

Normal initialize0066AD10(nullptr) sets mode+20=10, creates scene13/camera3,
camera parameters and scene callback0066E0F0, calls derived+40, then resource
setup0066C5B0, acquires shared pointer-input0066A6E0, creates ambient light2 and
click sound4. Existing scene/camera/light/click/factory/stack/pointer/render code
can be reused with its owners moved into the concrete state.

Preserve these reached effects when claiming their corresponding bodies:

- Resource setup saves/restores driver states15/19/11, loads cursor and slot-zero
  confirmation, then derived+38=00627060. That original hook eagerly builds all
  fourteen definitions and eleven augmentation routes. Our live factory supplies
  only Main/Exit. It is a bounded menu implementation, not a complete00627060
  execution. Additional screen builders remain [inventoried](menu-screens.md).
- Derived+40=00626340 creates a real critical section and auxiliary owners;
  0061A760→00618A00 reads mpmaplist.txt then GameData\\mpmaplist.txt fallback.
  This is multiplayer map-data preparation, not evidence of a network connection.
  It is not recovered merely by wrapping Main/Exit. Existing ignored exports
  frontend-content preserve its complete parser for that later owner.
- Normal music: manager+70 current index==-1 calls primary selection+60(0,0);
  current==37 calls queued selection+6C(0,2000,0); otherwise leave it alone.
  Selection004370F0 actually loads SoundCatalog then group12/request0. Reuse it;
  playback producer00433FA0 is not shown as a prerequisite of this music path.
- Update0066BEA0 calls derived+48=006269B0, sound update004368D0, then **calls
  both again** on normal mode10/gate7. Do not collapse these into one tick.
  006269B0 queries006ED960, a cached local date/time formatter, and enqueues
  01000014 for target04300000 only when the cached hour/minute pair changes (comparison006ED940 checks
  tm_hour and tm_min, not the full date).
  It is not a network-error poll. Then drain queued events0066C770, update shared
  pointer input0066B990, current screen, overlays and reverse notifications.
- Render0066DE00 checks derived+50=0067EAB0 (actual false), draws scene, preserves
  the two-pass modal driver-state5 protocol, then cursor0066A870. Existing modal
  composition is confirmed. Menu update is not part of render.

Shutdown0066B140 invokes derived+3C=00626950, clears stack/registered screens,
resource caches0066C6C0, shared input reference0066A780, then camera/scene and
queued messages/slot-zero owner. Scene sound/light references must be released
before driver shutdown. Shared pointer input must survive its screen borrowers.

The preview text loader consumes generated MenuText/labels.bin and Set2 assets
at800x600. New integration must bind captions to the actual loaded text cache or
explicitly retain that bounded rendering policy; do not advertise arbitrary
language/resolution support. Device attachment and local text availability do
not automatically replace the preview's text/resource consumers.

## Real video services required for full playback delivery

Ordinary00688040 initializes both IShow and a separate audio voice:

- ISnd driver+28(self)→1007AB20 creates0x1C object via1007BEE0, final table1009DE70.
  This is a voice factory, not a saved global volume. Object+00 release1007BFB0,
  +04 bind sample1007BFE0(self,sample,flags), +08 close1007C070,
  +0C play1007C0E0, +20 volume1007C9C0. Sample factory remains driver+20→1007AA80.
  Subtitle `S<time>filename` records create sample objects; playback rebinds
  this voice to a sample. Cleanup closes voice then releases it separately.
- IGraph+38 creates IShow; existing `EngineVideo` already forwards its checked
  ordinary methods. Add separate frame-position+38 for subtitle/audio timing;
  manager byte-position+34 is a different query. Failed open still sets status1
  and update sees IShow status-1; preserve progression into the next movie.
- Primary fade target is float bits3C343958 (about0.011), duration2000 for normal
  descriptor flag0; alternate flag1 uses0.05. The actual video volume is
  settings.float08 * float32(0.01) * float32(1.1), in that order. Keep x87/storage
  rounding policy consistent; do not carry the preview's host volume constant.
- Subtitle path006890F0 is executable_dir\\text\\language\\subtitles\\stem.txt,
  stripping the final filename extension.006ED860 is a real CRT existence check;
  only an existing file goes through line loading006E4CF0. Non-credits parser is
  00689970: timed audio samples and, when profile byte72 is set, text records.
  Text intervals use strict start*100 < movie_time < end*100 comparisons.
  Rendering text requires the real text/font owner00688630 and006443C0, not an
  adapter that claims it rendered. Credits parser00689340 is outside normal logos.
- Update00688C10 begins with actual sound-manager update before IShow status and
  subtitle/audio timing. The manager world view comes from00698500's global
  008AE7E4 (mission owner), not automatically the newly allocated menu scene.
  A null view before a mission is the real branch; do not permanently fake one.

Cleanup00688460 clears/presents three times, marks a recognized movie available
through006B1290, restores primary fade or queued selection, closes/releases IShow,
closes/releases voice, destroys subtitle/sample records, releases text/font uses.
Three logos are not in the profile movie list, but Intro is. With a selected
profile, even already-enabled Intro causes006B1290→006B09A0 persistence; its
filename match does not test whether the old flag changed.

Writer006B09A0 uses the actual display-name→physical-directory mapping in wide
mode, opens PlayersProfiles\\<physical>\\profile.sav for write, root1EDC, then:
display name1EDD; difficulty1EFF; four DWORDs74..80 under1F05; four84..90 under
1F06; strings04/08 under1EEA/1EEB; five slot IDs0C/14/1C/24/2C under1EF5; five
slot strings10/18/20/28/30 under1EFA..1EFE; movies under1EF0→1EC8. Strings include
NUL.006A6D80 writes exactly ten flag bytes in reverse order. Open failure returns
false; individual write results are ignored by this outer original writer. Use
the recovered ChunkFile writer and an isolated staged profile, not a write stub
or hardcoded root profile path. Broader cleanup belongs to the third/fourth items.

## Exit and validation before another delivery claim

The current controller source throws if input/update changes status to4, although
it returns false when4 is present at update entry. Existing preview Quit writes
a host `running` Boolean. Replace that bypass with the actual state/continuation
path: original switch case4 calls006C0920,0072E4E0(1), returns false. Full cold
control reset has real graphics/device/queue effects; it is not just gate=7.
Either reconstruct that reached shutdown path or keep the narrower host-close
limitation explicit until the shutdown item; do not advertise original Exit
teardown merely because the outer RAII destructor runs.

Implement in this order, as coherent batches:

1. Restore binding constructor mode2; expose startup phases/borrowed owners and
   single device lifetime. Add concrete controller services, including real
   command-line checks, input views, mouse request and profile-volume delay.
2. Build video and front-end owners with the above slots, distinct shutdown/
   destruction, and move the existing bounded menu session inside that owner.
   Preserve real unsupported branches and explicit source coverage limits.
3. Drive the recovered frame order; support both input views, single polling,
   natural/failed movie completion, replacement and a reviewed exit path. Use
   real sound/catalog/voice/text services for any newly enabled playback effects.
4. Temporary native ASan/UBSan audits:63/63 cold views, startup prepare/reset once,
   render-before-update, transitions after input/update/after-update, failed opens,
   shutdown-before-delete/initialize, fresh-state callbacks, partial construction,
   shared device teardown and mode changes rejected before unsupported calls.
   Compile/link strict i386 adapters; never execute the Windows fixture.
5. User desktop: new executable logs controller phases/types/sequence and the
   exact moment of menu-state installation; four opens, actual menu render/input,
   focus return and close must work in the isolated startup runtime. Only then
   check item2/commit interactive work. Items3–5 remain separately open.

Extend staging with the existing safe four-movie copier. Startup staging already
copies Text/GameData/Tables/profiles and base archives, but not loose GameMovie.
Never let existing immutable movies be silently restored during a missing-movie
test; stage first, then isolate the selected runtime file for the user's test.

## Evidence and reproducibility

[Static map](controller-integration-map.json) checks both original fingerprints,
state tables, small-method bodies, constructor mode, wrapper payloads, engine
voice ABI and direct-call boundaries. No new ledger credit:284 partial, zero
complete, frozen18,783. Native integration and all newly researched bodies remain
unimplemented until their source is actually changed and verified.

Read-only/noanalysis Ghidra exports newly retained under ignored analysis/work:
controller-integration-20261030 (8), controller-services-20261030 (5),
controller-remaining-20261030 (5), controller-persistence-20261030 (4),
controller-data-helpers-20261030 (5). All27 report decompiled=true. Earlier
frontend-defined/frontend-menu/mission-boundary/initialization and startup-movie
exports supply the already-confirmed larger bodies. Saved projects unchanged.
Always check each export, not only headless exit status. Use section-mapped PE
reads; several useful vtable targets are absent from the frozen function census.

Validation:64 anchors,35 client state slots,10 engine slots and65 direct calls
pass both fingerprints. Four altered constructor/call/state/voice maps reject.
The temporary binding/controller audit passes native ASan/UBSan and strict i386
compile/link; Windows fixture never run. Temporary audits/logs removed. Added
Markdown links, whitespace and frozen function accounting pass. No runtime code
changed after the desktop-accepted commit, so no further desktop claim is made.
