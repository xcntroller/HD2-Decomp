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
