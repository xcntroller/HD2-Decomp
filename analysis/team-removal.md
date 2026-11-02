# Team removal and widescreen follow-up — 2026-11-02

The user reports rapid clicks after squad removal restoring “Remove” with no
selection, failed drag removal, and missing first-picker selection after removal.
Their screenshot shows vertically displaced text/artwork in widescreen Spanish
Main. They authorize a small widescreen fix. They confirm localization works
perfectly and Japanese localization was unfinished, so its missing main file is
expected. This accepts localization as reported, without inferring individual
desktop runs of all seven languages. The four corrections below are now
[desktop-accepted](menu-fidelity.md#desktop-acceptance--2026-11-02).

## Selection and removal

Original `006948D0` rejects selected image index `-1` before its caption/outline
path. The native callback lacked that guard. Original membership helper
`006953B0` compares all four slot IDs, including empty sentinels; asking about
`-1` therefore finds an empty slot. That helper is unchanged. The adapter now
requires a real human before producing a membership-based Remove caption.

`image_human()` rejects empty portrait data, squad data that no longer matches
the live slot, and roster callbacks for an already assigned human. Selection,
double-click and drag-begin share this validation. This covers another callback
before queued portrait refresh and later clicks on an emptied slot. The picker
selection and outline survive. The production `0066B990` dispatcher sends
double-click before click when both flags are set; the audit exercises that
ordering explicitly.

Button and double-click toggles share one completion path. Successful removal
selects the first non-squad candidate through the existing `00694AE0(0)`-based
helper and sends list visibility/focus request `020000AA,0`, making that picker
entry visible even after scrolling. Drag removal uses the same helper. Failed
or locked transfers do not move selection.

Original `00694C40` can retain a neighboring squad member when one remains;
its empty-squad branch requests roster selection. First-picker selection after
**every** removal is the user's explicit requested policy here, rather than a
claim about every original removal branch.

Original `00693C10` compares normalized X with `0080F518 = .5`. Drag-end still
compared pixels with400 after the rest of the application became dimension-aware.
At1600-pixel width, the correct divider is800. It now uses `menu_width() * .5`.
Squad removal includes the divider; candidate assignment includes it from the
opposite direction. Slot swaps require the squad half and retain the actual
`reorder()` result. Ghost/source lifetime, locks, incremental membership and
immediate post-drain slot refresh remain in place.

## Bounded widescreen correction

This needs one additional checked camera call. Constructor `10009840` initializes
aspect `+138` from graphics width/height. Matrix builder `10009B30` computes
orthographic Y scale as aspect × extent. Menu text/picking uses the authored
1 by .75 plane, centered at(.5,-.375), with extent2. At16:9, the unadjusted camera
projects vertical edges beyond the window while text uses the full-height
4:3-coordinate mapping, explaining the screenshot's increasing vertical drift.

The configured application calls camera virtual `+8C → 1000A010` with aspect4/3
before the existing projection/extent calls. Those subsequent calls rebuild the
matrix; the aspect setter alone skips rebuilding an already orthographic camera.
Artwork, text and picking consequently share one plane. This stretches original
menu artwork to the selected window, without adding gameplay widescreen/FOV work.
The diagnostic preview keeps its original setup.

[Static evidence map](team-removal-map.json) checks ten client/engine anchors,
three engine slots and three calls against exact fingerprints, and links cached
original team exports. No original executable/DLL, Windows fixture or Proton
was executed by the agent.

## Verification and staged build

All three strict i386 targets compile/link; existing native regressions,158 Python
tests, static maps and frozen ledger checks pass. Credit remains **564 partial
(559 client/five engine), zero complete /18,783**.

Temporary ASan/UBSan extracts the actual production refresh, selection, transfer,
drag methods and camera setup calls. It uses production squad state and pointer
dispatch, with native queue/frame/device fixtures. It passes60 removals covering
every position in squads of1–4 and12000 rapid empty-slot click/double-click bursts.
Assertions cover stale same-frame callbacks, first selection, Add caption,
outline, scroll request, updated slots and picker membership. It passes18
remove/cancel midpoint cases across640×480,800×600,1024×768,1280×720,1280×1024
and1920×1080, plus assignment, swaps and locked rejection. Actual camera setup
calls aspect → projection → extent; native matrix calculations match text
rectangle centers and inverse picking at54 points across those six modes.
These checks establish source behavior, not actual D3D rasterization.

Logs/provenance: `analysis/work/team-removal-20261102`. Temporary audit generator,
source/binary and disassembly scratch file are removed after verification. No
permanent tests, new function credit, commit or push.

Staged executable SHA256:
`bb8cbe802887e64d25f7ea8f4c700f47faaa822730c1c0214e26239331340071`.
Stage-only preparation used1920×1080/Spanish and preserved all seven current
profile/configuration files by hash. A pre-stage snapshot already had the same
new hash; it is a staging snapshot, not user acceptance evidence.

User recheck: rapid squad removal, dragging a member into the picker, first
picker selection after removal, and widescreen artwork/text/picking alignment.
The user subsequently confirmed “All fixed and works as expected. Record as
complete, commit and continue.” This accepts all four corrections against the
staged hash above and closes the parent presentation checkbox. The earlier
pending-review statements describe intermediate delivery, not the current
acceptance state. No additional case-level execution is inferred; source-only
loadouts and game/server exclusions remain unchanged. The user authorizes the
commit and continuation, without a push.
