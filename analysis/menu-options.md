# Options construction and dependencies

This is an evidence inventory, not a complete or runnable options definition.
The client fingerprint is recorded in [the factory map](menu-control-factory-map.json).
The five builders below are called in order by 0061CF60 after the options
navigation group is assembled. Ghidra exports are local research artifacts.

| Builder | Top-level kind-five groups | Selected child dependencies |
| --- | --- | --- |
| 0061DBC0 | `graphics_setup` | Text, kind 8 gamma slider, kind 0x10 geometry selector, seven kind 0x24 controls `GD00`–`GD06`; settings observers and conditional disable/hide. Original frames `nextdetail`, `shadows`, `mirrors`, `morphing`, `dynamic_lightning`, `light_glow`, `use_high_models` are renamed first. |
| 0061E140 | `sound_setup` | Text, sound and music kind 8 sliders, each initialized with range 100 and an observer. |
| 0061E390 | `keysscreen`, `keysscreen01` | Controls/mouse settings and additional selector/key configuration; decompiler argument recovery is incomplete and individual definitions still need instruction-level tracing. |
| 0061F1D0 | `gameoptions` | Text and kind 0x24 subtitles/parental-lock controls, observer 00620440 and conditional disable. |
| 0061F350 | `network_settings` | Text, kind 0x10 connection selector, kind 0x24 voice-chat control, kind 8 volume slider, and observer 00620720. The builder hides the group and also issues additional hide initializers; confirm exact targets before implementing. |

The original creates engine dummy `GROptions` under the options model, then
reparents six navigation frames to it with link mode 1. The corresponding UI
definition has kind 5, with buttons for video, sound, controls, game,
multiplayer and back. Its page activation bindings address distinct top-level
groups; controls has an additional key-configuration group. The builder also
adds four initial text controls and a version control. A screen show observer
006204D0 queues a conditional caption update to the third top-level child.
Its data source and complete ownership contract remain unresolved.

Entry handler 0061BEA0 handles 02000071 by reading graphics and settings,
sending multiple targeted initializers, then queuing the screen transition. Its
other branches also interact with settings and page teardown. It cannot be
replaced faithfully with a simple screen push. Original widget constructors,
input/focus for nested children, observers, settings synchronization and apply/
back behavior all need further recovery before options can be enabled.

## Native widget and observer reconstruction

The [widget evidence map](menu-options-widget-map.json) records source-only
state/message/input implementations for kinds 8, 0x10 and 0x24, and the four
original settings observers. Engine visual/internal-part constructors remain absent. The
[canonical profile settings backend](menu-caches.md) now supplies real preference
storage and individual application helpers; unused widget routes remain source-only. The live factory continues to reject these
widget kinds; adding names to the enum does not enable their screen controls.

- Sliders preserve the range/position asymmetry: programmatic position accepts
  the range itself, while ordinary steps compare against range minus one. Wheel
  magnitude rounds up in units of 120, and repeat timing uses a strict negative
  test with initial/repeat delays 300/50. Thumb changes queue linked updates;
  arrow/track steps use immediate copied linked updates. Value-change tag carries
  zero; observers query the thumb's floating-point percentage separately.
- Selectors own shared item strings, preserve query payload ownership and signed
  selection rules, and wrap previous/next only while enabled. Removing an item
  can leave the caption unchanged and disables when the selected index is at most
  zero, even if several items remain. Programmatic selection still notifies while
  disabled. Invalid queries retain the old payload and report index minus one.
- Toggles distinguish notifying 0100006D from silent 01000076. The notifying path
  falls through after observers, rereading state before a possible silent change.
  Direct activation/click toggles without an internal disabled gate; the parent
  owns that input gate. Yes/no focus follows target low nibbles one/two.

Graphics observer 0061FCA0 queues seven silent toggle values followed by seven
availability events, then applies graphics. Slider observer 006202A0 queries
before identifying gamma, sound, music, mouse sensitivity or voice-volume
targets. Sound volume additionally plays the click preview; music below one is
zeroed. Toggle observer 00620440 routes invert mouse, subtitles and parental
lock; only invert mouse calls an immediate apply helper. Voice observer 00620720
stores the low byte but enables/disables its volume slider using the full
argument's zero test. Typed native services stand in for preference storage,
feature computation and engine application; those service bodies are not rebuilt.

A temporary ASan/UBSan audit instantiates the native runtimes with supplied
visual/input backends and actual control/text/observer helpers. It checks value
boundaries, shared-string lifetime, focus and disable forwarding, wheel/repeat,
direct/queued synchronization and the connected preset/settings graph without
notification feedback. The audit also compiles for i386. This is not live engine
widget validation; no additional desktop test is requested for this source slice.
