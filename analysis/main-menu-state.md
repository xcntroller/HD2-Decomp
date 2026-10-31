# Main live labels and eligibility

2026-10-31: integrated, native-audited and **user desktop-accepted**.
Previous profile milestone accepted/committed as`977641f`, application SHA256
`5e6882429e45bd8805e167ee72a8f243feb7d6ca6c8bbe576b028123cac5acf7`.
The current changes implement the next section3 checkbox. Gameplay-save loading,
unsupported Main destinations and complete traversal retain their later gates.

## Original contract

Client SHA256`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`;
engine SHA256`12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Original bodies are read-only evidence; the agent executed no original client,
DLL or Proton. Existing screen definitions come from`00623DD0`.

`00627520`, Main's shown callback, reads the selected profile and refreshes the
following controls. The reconstructed helper is
[main_menu_state.h](../recovered/frontend/main_menu_state.h); ApplicationMenu calls
it on Main activation and profile change. Date/time keeps the existing real local
clock update. No filesystem date is substituted for a stored save caption.

| Control | Empty profile | Active profile |
| --- | --- | --- |
| `04500000` current profile | Style12 label2020, NO CURRENT PROFILE | Actual selected name, style12 |
| `04700000` last save | Style12 label2021, NO SAVED POSITION | Three style7 stored fields, or label2021 when malformed/absent |
| `04800000` Resume | Disabled | Original state enabled only when caption parses; receiving gameplay owner remains unavailable |
| `04900000` profile action | Style13 label2117, CREATE PROFILE | Style13 label2106, CHANGE PROFILE |
| `04A00000` through `04F00000` | Disabled | Original state enabled; actual destination availability still applies |
| `05000000` Exit | Remains available | Remains available |
| `05100000` version | Localized style40 version formatter | Same |

Thus only Exit and the profile action accept input without a selected profile.
Returning from profile creation/deletion refreshes these states. Real original
eligibility and delivered receiving-owner availability remain separate: this
checkbox does not enable a dead Single Player/network/Resume destination.

## Save-caption dependency resolved

Read-only export`analysis/work/main-labels-20261031/006b3ac0.c` shows that
`006B3AC0` reads profile+08, already represented by`ProfilePayload::strings[2]`.
The existing actual profile reader consumes chunk`1EEB` (or legacy`LASTSAVETEXT`)
and retains it there. The helper requires a nonempty caption and two`$` delimiters;
it returns the first field, second field and remaining suffix. Empty individual
fields and additional delimiters in the third field remain valid. Missing either
delimiter returns false. The original uses a1024-byte local buffer; the native
implementation guards overflow rather than reproducing undefined writes.

`00627520` clears the previous last-save text, appends the three fields with
style7, or shows label2021. Each style7 record starts a row: font2/BIG, height.03,
left/vertically centered. At800×600 this is three18-pixel rows centered together.
This is actual profile metadata, not an inference of save-file existence,
compatibility or loadability. Broader save metadata/preview entry points and
real Resume/controller handoff remain independent family gates.

## Version and title placement

`00644200` special-cases style40: localized label2006 is
`Hidden and Dangerous 2. ver. %d.%02d.%d`. Original initialized data at`00844F88`
and`00844F8C` contains1 and12. The third value`009820F4` is zero-initialized BSS;
read-only reference inspection finds only its formatter read. The resulting
English version is`1.12.0`. The native formatter accepts the evidenced integer
formats and escaped percent, with bounded argument handling. It is now supplied
through the existing TextObject version callback instead of deferring the label.
Style40 uses SMALL font, height.02, left/bottom alignment. The diagnostic resource
slice now stages label2006 too; the application continues using its real localized
text cache. Existing tooling tests use the declared label count rather than13.

The title difference is not a resolution guess. `00641E30` style10 has leading
space.01 at style+18. `00645F20` scales and rounds that by render height into row+18;
`00646940` excludes the first row's leading space from aligned block height;
`006469C0` nevertheless adds it when placing the baseline. For a single bottom-
aligned title this lowers the baseline by6 pixels at600px height. The bounded
menu renderer now applies that offset for style10. Primitive glyph geometry and
ordinary button/list alignment remain unchanged. This restores the shared title
style, including Main, rather than an arbitrary model/frame translation.

Evidence: existing style/font/alignment exports plus
`analysis/work/main-labels-verification-20261031/{style-spacing,block-height}.asm`
and`version-globals.log`. No new credit is claimed for borrowed engine methods or
for the general text layout bodies underlying this small formula.

## Verification and acceptance boundary

Temporary ASan/UBSan audit composes the actual Main refresh helper with real
TextControlRuntime/ControlStateRuntime and TextObject storage. It covers repeated
active/empty profile changes; valid/empty/malformed save captions; extra/empty
fields; replacement without old rows; original label IDs and disabled controls;
version/format errors; bottom-aligned version; title baseline+6; three-row spacing.
Logs: `analysis/work/main-labels-verification-20261031/{audit-build,audit}.log`.
The audit's visual operations are native shims, not desktop/engine execution.
Temporary source/binary removed after use.

Strict i386 application-launch, engine-window and startup-probe compile/link;
existing native suite and158 tooling tests pass. Staging is stage-only and
preserves isolated profile/config changes. Handoff records final staged hash.

Pending user desktop confirmation: title nearer the line, visible version,
unchanged date/time, correct active/empty profile and missing-save labels;
no-profile Exit/CREATE PROFILE only; restore available actions after creation;
return after selection/deleting the last profile. Displayed populated save captions
should reflect actual copied profile data. Resume remains a later receiver gate.
Main plan/family checkbox stays unchecked; no interactive commit before acceptance.

## Desktop acceptance

User2026-10-31: “Looks good. Record, commit, continue.” Accepted application
SHA256 `d5656467e031f56e7fe77c32dfb280f12580206fd111f2881cc8a4570dc9d953`. Acceptance covers the Main label/
eligibility update described here, including title/version and no-profile state.
No separate execution of every proposed case is inferred. Historical pending
statements above describe the intermediate build and are superseded here.
Real Resume/gameplay and broader save metadata/preview gates remain open.
