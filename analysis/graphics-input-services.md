# Concrete graphics input services

[`host/graphics_input_services.h`](../recovered/host/graphics_input_services.h)
connects the source-recovered input manager, installer and producer to the
original LS3DF and existing checked IJoy adapter. It is source-only: no live
host path is changed and neither original DLL's internal input implementation
is reconstructed by this bridge. The [evidence map](graphics-input-services-map.json)
checks both client and engine fingerprints, startup/producer instruction regions,
engine method bytes and 31 relevant vtable slots. IJoy's separate method
fingerprints are in [its evidence](joystick-devices.md).

| Use | Original graphics slot | Engine RVA |
| --- | --- | --- |
| Initialize keyboard, flags 0 at startup | +90 | 6F040 |
| Pump graphics | +14 | 6A470 |
| Poll keyboard | +9C | 6F190 |
| Install keyboard scan map | +B4 | 6F4B0 |
| Read packed 64-bit keyboard snapshot | +B8 | 6F500 |
| Poll mouse | +E8 | 6F8A0 |
| Read mouse X/Y/Z | +EC/+F0/+F4 | 6FA10/6FA20/6FA30 |
| Read mouse button mask | +104 | 6FB30 |

Graphics methods use stdcall with explicit self; +B8 returns EDX:EAX, and the
map installer also takes the scan pointer and count. The original manager uses
its attached graphics pointer, so the service methods use that pointer instead
of looking up the singleton on each call. They forward joystick retain, release,
configuration, polling and both state reads to the supplied real IJoy adapter.
The original producer controls order and ignores method results. Checked method
identity and null/unsupported dependency failures throw in native code; those
are explicit safety differences from original unchecked virtual calls.

`GraphicsInputSetup` resolves checked GetIGraph and calls it for each graphics
query. It supplies keyboard initialization and the borrowed joystick-list view to
`setup_graphics_input`; the latter snapshots the view before its second query,
clears/attaches the manager, then performs language and text loading. The setup
is deliberately separate from list creation: no discovered original caller
justifies adding joystick enumeration to normal empty-list startup. Both DLLs,
the joystick adapter and list must outlive their users; manager owns only its
retained joystick references and does not close graphics devices.

The original normal loop polls mouse and the joystick list outside the producer,
then calls `produce(false)`. Calling `produce(true)` there would poll them twice.
The existing preview's independently owned native keyboard/mouse state cannot be
silently mixed with this adapter's original engine-owned state. Actual lifecycle,
focus handling, profile settings, protection and controller wiring remain open.
No successful no-op backend was introduced.

Validation: a strict i386 fixture compiles and links the actual IJoy adapter,
list, manager, setup, localization platform/text loader, default binding
installation and both producer modes. It is never executed. Separate compiler
output confirms a stdcall uint64 result uses EAX and EDX. The map validates 12
instruction/data anchors and 31 engine slots; three mutated anchors/slots are
rejected. Dependent input-manager, localization and IJoy evidence validators,
existing native suite, 158 Python tests and unchanged host build pass. No
permanent tests added. No new recovered-function credit is claimed for calls
delegated to original libraries.
