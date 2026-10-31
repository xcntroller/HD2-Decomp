# Credits flow — integrated; desktop acceptance pending

2026-10-31. Plan section3 first checkbox / `media.credits`. The complete compiled
Main → Credits → retained Main flow is implemented and native-audited. Its delivery
checkbox remains open until actual desktop confirmation. Keep uncommitted.

## Original contract and resource correction

Original client SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`,
image base00400000. [Fingerprint-checked evidence](credits-flow-map.json).
Read-only Ghidra exports/instructions are ignored under
`analysis/work/credits-{research,scroll-research,layout-research,layout-tail}-20261031`.
No original client/DLL/Windows fixture/Proton was executed.

The earlier family plan's movie resource gate was incorrect. `credits.bik`
(00842EFC) is a selector:00688040 compares its beginning with `credits`
(008415D0), enters the scrolling-text branch and **skips both IShow and voice
creation/open**. No matching loose movie is required.006890F0 loads
`Text/<language>/subtitles/credits.txt` into a separate real localized-text cache.
Credits ignores the ordinary subtitles preference. The installed English file
is26,111 bytes; its actual parser produces1,055 numeric records. No proprietary
script or atlas was added to tracked source.

- 00626B20 maps the original Credits button event0600103A to state12.
- 006C20F0 retains the frontend without shutdown, installs type10/sequence0 and
 starts it. On status13, it restores the retained pointer **before** shutting down
 and deleting the video owner, then starts the retained owner with status1.
 The native controller uses a separate owning pointer instead of the original
 wrapper's+80 field. It supports Main Credits; Cinematics filename selection,
 game/descriptor/outro/restart/exit transitions remain separate.
- 00688040 saves sound request/position (+70/+74), then queues request37 with
 duration2000 and position0 unless37 was already selected. Recovered real sound
 queue00436F10 halves the duration and fades both channels; its actual runtime
 performs the music selection/seek. No successful audio placeholder is supplied.
- 006887C0 creates the .02/.98 screen rectangle, starts offset at
 `-width/1024*(height+100)`, and defaults speed to `width/1024*50*.001f`.
- 00689340 consumes records in signed-ID order. Exact tags TI/NA/SM/EL/ES and
 TICZ/NACZ/SMCZ select styles0..7. Complete C_1/C_2/C_3 marker sets split SM into
 styles11..13 and SMCZ into8..10. Unknown/malformed tags are ignored. Valid SPEED
 numeric prefixes override the speed; the installed script selects70. Raw8-bit
 CE characters are preserved; this is not an ASCII-only renderer.
- Table008412C0 contains fourteen52-byte styles: all white, horizontal center,
 vertical top, wrapping disabled (+2d=0), no extra row spacing. Fonts2/3 are ANSI
 BIG/SMALL and5/6 are CE BIG/SMALL. Heights are .06/.04/.022/.04/.025 for the
 ordinary tags. Column anchors are .25/.5/.75 of the rectangle width; flags18
 start a new row and flags10 append columns. Native owned style data replaces
 original global style handles; it does not implement the full shared style cache.
- 00645270/00645F20 group the nonwrapping records into rows;006469C0 accumulates
 glyph-height baselines.00646A10 centers ordinary lines with signed integer
 remaining-width division;00646DB0 centers columns on their float anchors.
 00646790 only flags overflow; the constructor's inset10 does not shrink this
 Credits alignment interval.00648860 emits actual font-metric glyph triangles.
- 0064B940 stores the scroll word.0064B950 subtracts its signed low16 bits from
 each signed row baseline. Negative bottoms permanently advance the first-visible
 cursor; rows beyond surface height+100 stop drawing. It then draws black edge
 fades, each `trunc(width/1024*100)` pixels tall (78px at800x600).
- 00688C10 updates sound first, advances the float offset, and accumulates a3000ms
 completion dwell only once0064BCC0 reports all rows consumed. **Rendering**
 advances that exhaustion cursor; updating without rendering cannot finish a
 populated script. Native out-of-range conversions throw instead of C++ UB.
- 00688BB0 Escape clears/presents and finishes; Space is ignored for Credits.
 00688F50 clears black and uses a separate text scene under saved/restored graphics
 state0. The outer application loop presents; there is no inner Credits loop.
- 00688460 clears/presents three times, invokes the existing profile movie-recorder
 (Credits is not a recognized unlock filename), restores the saved music request
 and position, or fades to1 over2000ms if either saved word is-1, then releases
 resources. Native exceptions still release all acquired resources; a failed
 initialization does not record an unstarted movie. Closing the window during
 Credits also shuts down the retained menu before clearing controller caches.

## Integration and boundaries

[Portable parser/scroll](../recovered/frontend/credits_text.h),
[actual layout/glyph generation](../recovered/host/credits_geometry.h),
[four-atlas renderer](../recovered/host/credits_text.h),
[controller](../recovered/frontend/application_controller.h),
[state/backend](../recovered/host/application_launch.cpp).

The existing Main factory enables only Credits and Exit for this application.
Original declarations/IDs/messages stay intact. The diagnostic factory's default
availability stays unchanged. Main focus/input/scene/text owners survive Credits;
return does not recreate Main or reset its selected control. Main hover text now
uses the actual focused control, so Credits receives the normal highlight.

Real startup music services save/query/queue/restore the existing sound manager.
The text pass uses the actual engine atlas loader and primitive consumer, plus
its existing verified D3D8 state bridge. All engine bodies remain delegated.
The Credits backend creates no video decoder or voice. Native resource ownership,
style records and strings replace original object/container/allocator/SEH ABI.
The current menu delivery boundary remains English800x600; other locales and
shared wrapping/general text features are not claimed.

[Staging](../tools/stage_credits_resources.py) adds the two CE atlases to isolated
Maps. Startup staging already copies the script and CE metrics in the Text tree;
ANSI resources reuse existing menu staging. CE BIG is the original512x512 atlas
with a legacy extended TGA tail, fingerprint-guarded and copied unchanged;
CE SMALL is256x256. Preflight rejects modified/linked targets and invalid inputs;
atomic no-overwrite publication preserves other files. No movie substitution.

## Verification and desktop gate

- Strict i386 compile/link: application-launch, engine-window, startup-probe.
- Existing native suite and all158 tooling tests pass.
- Temporary native ASan/UBSan audit uses the real script and all four metric
 files:1,144 styled records,1,054 rows,50,434,230 submitted vertices over the
 simulated run, automatic completion at406,624ms using16ms ticks. It checks exact
 tags, raw CE bytes, empty lines, column centering, baseline increments, speed,
 fades, low-word scroll and3000ms dwell. These counts are a native simulation,
 **not a desktop observation**.
- Actual queue routing/controller/state audit verifies status12 entry, ignored
 Space, Escape, automatic completion, repeated entry, same-pointer restoration,
 no menu shutdown on return, retained cleanup on close, and failed acquisition.
 Real two-channel music queue metadata retains the original seek word/duration.
 Engine rendering/audio/OS behavior still requires desktop acceptance.
- Stager audit exercises original assets/identical reuse and rejects modified
 target, symlink, unsupported CE BIG and malformed metrics without target writes.
- Original fingerprint/map, frozen function ledger and whitespace checks pass.
 Four new partial body entries:006887C0/00689340/0064B940/0064BCC0. No new credit
 for absent0064B950, delegated engine calls or generic layout helpers.
- Stage-only launcher completed; built/staged SHA256
 `121ee7ead4ad8fd410b453b59f3f17af636c2a1e58772499a752e357dacc4236`.
 Logs/manifest: `analysis/work/credits-verification-20261031`. Temporary audits
 were removed after recording their results. No staging/index/commit/push action
 in git; the previously staged inventory remains intact.

Run `python3 -B tools/run_application_launch.py` for the desktop check:

1. Preserve opening skips and Main/Exit; activate Credits with pointer and keyboard.
2. Check headings, names/CE glyphs, three columns, black background/edge fades,
   smooth scrolling and Credits music. Space must not exit; Escape must restore
   the same Main selection, mouse/keyboard input and prior menu music.
3. Re-enter and allow natural completion (about6m47s at the original script speed),
   including the final3s dwell; check the retained Main return again.
4. Close the window during Credits; verify clean teardown. Repeat with sound/music
   disabled and ordinary subtitles disabled; Credits text must still appear.

Until that actual test is reported, both the family delivery and plan section3
checkbox stay unchecked. No commit is authorized.
