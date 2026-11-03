# Phase2 receiver boundary and first owner source

Later update: [constructor/backend/controller integration](game-owner-integration.md)
supersedes the missing implementation limits below; this remains the initial checkpoint.

2026-11-03. **P2.0 complete as source-only boundary research; P2.1 started,
not integrated or complete.** Phase1 closure/source checkpoint committed as
`4e530f7`. This batch continues into implementation rather than ending at an
address census. [Evidence map](phase2-boundary-map.json),
[source/remaining-work index](game-owner-start.json).

## First mission and resource selection

The existing native GameData/route readers were exercised against the actual
22,161-byte `others.DTA` member `GAMEDATA\\Gamedata00.gdt`, SHA256
`f92ab7e8796148af7854438ffca4df34b55cca8e83336878d7f569cdcd3319b1`.
Root0/mode1 ordinary traversal reaches kind6/subtype2 team selection, then kind7;
`route.mission()` returns the real kind3 **Tutorial** descriptor. The game marker
is not the mission argument. No campaign-progress override is needed.

The base `Missions.dta` contains Tutorial Scene files, scene2, tree.klz, actors,
items, tracks, sounds, effects, scripts and other mission resources. Its scene2
is3,684,095 bytes, SHA256
`89d9e37a57ad956804fc45e4f9790078a8d3999b14d41a0fca6ba94977ed4773`:
one root4C53,1,890 named object records and975 model-resource fields. This is
structural dependency evidence, **not** an implemented parser/world. Existing
ALPS1 evidence remains a second regression sample.

Production must consume the actual selected descriptor/resource using the
accepted rw_data registry, not force Tutorial or copy the base sample over an
override. Flags0/default searches loose files first, then registered language/
fallback, Patch and base archives. Installed LangEnglish/Patch are ISD1, outside
the existing static ISD0 extractor; this research does not assert they contain no
overrides. The original runtime resolver remains authoritative. Later desktop
acceptance uses an isolated fresh-profile copy, normal Campaign/team entry,
actual selected world, Main return and repeat entry.

## Receiver, ownership and failure contract

Client fingerprint remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`;
engine `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.

- `006C0EF0` kind7 releases the previous controller state, validates the mission
  guard, allocates0xEC aligned16, invokes `00698510`, installs the state, obtains
  `006B9190`'s actual mission, then calls game virtual+04. Status9 `006C20F0`
  additionally needs00730A40/0072E890/00731610 preprocessing; live networking is
  still Phase3. No additional lobby is inferred.
- Constructor `00698510` installs008159AC; status+04=0, pending+08=2. It constructs
  HUD0049A420 (+58,0x6B900), overlay00490740 (+5C,0x4C370), mission text (+60,0x28),
  scene (+18,driver+54 kind13), then two-set owner00523AB0 (+54,0x78), and clears
  the latter through00523E10. It publishes008AE7E4 **after** scene/table setup,
  then allocates an empty COW string+AC and zeros two64-DWORD global tables.
- The constructor creates no separate camera. A real loaded camera/sector belongs
  to the loader/activation work; the menu's orthographic camera is unsuitable.
- Scene virtual+74 is engine10046820: stdcall(scene,callback,flags), flags6;
  null callback clears both stored callback/flags. Viewport+148 is10048440,
  stdcall(scene,left,top,right,bottom),20-byte cleanup; compare original scene
  +244/+248/+24C/+250 with0,0,physical width,height before setting.
- `006C9260` is **stdcall(stage,word,word), RET12**, although only stage is used.
  Status14 suppresses the entire callback. Stage1 optionally invokes HUD prepass,
  then control+A4 and optional debug. Stage2 chooses HUD or physical viewport,
  then mission text, network overlay, conditional game fade, control+A0 and
  effects. A six-DWORD graphics viewport copy preserves near/far. Its callees
  are mandatory services, not successful default renderers.
- Deleting destructor00698810 wraps00698830 and aligned-free. The latter clears
  transient queues004ECDE0/004ECCD0; destroys HUD0049A750, overlays004908B0 and
  mission text0048EF40/0069F130; unloads0069A3D0, releases scene00699270, clears
  the published pointer, destroys tables00523B20, then remaining vectors/strings.
  Publication remains until scene release. Live shutdown00698F80 is a distinct
  operation with actor/preparation, sound/control/network consequences; an empty
  destructor is not its replacement.

`006989B0` activation is thiscall(owner,mission), one-success. It saves graphics
flags and clears bit20, selects the actual resource (or existing config branch),
and invokes thiscall006CA780(owner,resource,mission). The loader returns
**zero-success**; only then does activation retain the mission, initialize live
systems/actors and reach status5. Both ordinary load failure and success restore
graphics flags. Already-status5 returns0. No successful activation is implemented.

All six protected import continuations reached directly by activation/loader have
been resolved against the original return-address/IAT/name table:

| Return address | Real API | Caller |
| --- | --- | --- |
| 00698AAE | GetTickCount | activation timer |
| 00698C09 | IsWindowVisible | activation window query |
| 00698C5D | GetCapture | activation capture query |
| 006CA913 | GetParent | loader window query |
| 006CAAC0 | IsWindowVisible | loader window query |
| 006CAB29 | IsIconic | loader window query |

These are underlying Win32 operations, not protection-success checks. Earlier
untyped decompilation artifacts must not be treated as function signatures or
substituted with GetFocus/IsWindow. Map anchors preserve each actual table entry.

## Ordered loading work retained for P2.2

`006CA780` resets+66/+68/+D8 and acquires006CB280/006CB0E0 scoped owners;
006D3F20 establishes loading006C9640, unloads any prior mission and retains the
resource/scene. `00783350` is the one-time animation gate, **not protection**:
it sets0098A4A4 before calling00783BC0, which opens `Tables\\anim.sav` and runs
its table readers/fixups. Its unusual failure/retry latch must not become a
synthetic successful animation service.

Next006D4180 opens `missions\\%s\\Scene` through scene+F4(flags7, callback006D4C70),
then scene+F0, Scene2 stage006CBF30/root006CC760/object006CD940, window query,
006D45C0/006D4880/006D4280, progress(4,.3) and006D4B50. Nonzero Scene2 status
fails only after those ordered stages. Success loads `missions\\%s\\tree.klz`
through006EE570(path,1), progresses.8, publishes scene0089A8F4, initializes
effects00445D90, progresses1 and sets loaded+14, followed by006D4B40/006C95B0
and the real window queries. Scoped locals unwind on both results. Additional
activation work after loader success remains separate from merely drawing a scene.
The original006D4B50 entry is a single RET; no invented loading operation belongs
there. Full callees, resource/object consumers, camera and actor activation remain
P2.2/P2.3 implementation, not new matching/audit prerequisites.

## Implemented source and exact limits

- `recovered/game/session_owner.h`: owned preparation plus deep mission descriptor
  copy; required subowner factories; ordered scene/table construction/publication;
  repeat-entry guard, allocated empty string and unloaded cleanup. Native fault
  cleanup retracts publication even when allocation fails after publication.
  Factories are required interfaces with **no default implementations**. Concrete
  HUD/text/table backend and actual controller installation are still missing.
- `recovered/game/overlay_owner.h`: native500×13 rows, each with four owned frame
  references and four shared strings; empty constructor state; verified reverse
  record/row cleanup and forward frame cleanup. Internal padding/original ABI and
  later overlay population/rendering are not claimed.
- `recovered/game/mission_tables.h`: two dataset constructors, full ordered clear
  of present arrays/counts, preserved vector capacity, first→second clear and
  second→first destructor. Cookie/0x24-element destruct/free are explicit required
  backend calls; no guessed parser or fake populated data is installed.
- `recovered/game/scene_callback.h`: all callback dispatcher branches and original
  call ordering, with mandatory actual services. HUD/control/debug/network/fade/
  effects callee bodies remain unimplemented here.
- `recovered/host/game_scene.h`: actual checked engine scene factory, callback
  installation, viewport and release. C++ exceptions are latched at the DLL callback
  boundary and must be rethrown by its caller; they cannot unwind through LS3DF.
  This bridge was compiled/linked, **not executed or desktop-verified**.

These are native source reconstructions with explicit remaining ABI/consumer
work. Twelve frozen entries gain partial source evidence; **no new complete
function or engine/CRT/delegated-callee credit**. Inline004907E0/004908A0 and
record-destructor00524610 lack standalone frozen membership and gain no credit.
The postponed530-entry review queue is unchanged; these twelve newly implemented
partials are tracked by this delivery batch instead.

## Verification and next work

One consolidated sweep passed: existing native regressions,158 Python tests,
application-launch/engine-window/startup-probe builds, strict i386 scene fixture
compile/link and callback RET12 inspection. Temporary native ASan/UBSan audit:
actual GameData/normal Tutorial traversal, deep request copy,100 owner lifetimes,
five service-stage failures, null allocation result, seven allocation-failure
prefixes including after publication, duplicate-owner rejection, reverse overlay
release, populated table clear/free/capacity contracts and1,280 callback cases.
Fixture HUD/text/scene endpoints validate orchestration only; they are not runtime
implementations. No original executable/DLL/Proton/Windows fixture was run. No
interactive changes were made, no desktop acceptance is claimed, and no package
was produced. Temporary fixtures/binaries were removed; ignored logs, hashes and
assembly are under `analysis/work/phase2-owner-20261103`.

Next: finish actual HUD0049A420/0049A750 and mission-text constructors/cleanup,
share the accepted real font/resource services, implement concrete dataset frees
and callback services, then install the owner at kind7 with an honest incomplete
loader result and Main return. Keep the P2.1 boxes open through integration and
user confirmation. Do not add a fake renderer/loader to make the constructor pass.
