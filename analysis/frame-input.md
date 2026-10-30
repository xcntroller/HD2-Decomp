# Original frame timing and keyboard production

Checkpoint `8eef91d` contains desktop-confirmed opening playback and the audited
shared application controller. This subsequent slice replaces host pacing with
the recovered original clock. **The user desktop-confirms pacing**, with four
movie opens and clean teardown after 899 frames. The subsequent keyboard-device
integration is also user desktop-confirmed, including focus loss/return.

## Frame clock

Client `006DC950` reads the performance counter in microseconds, subtracts the
previous low 32-bit value with unsigned wrapping, then divides by 1000. It
checks maximum first, minimum second. When below minimum it schedules a
multimedia timer with flags 0x20 (event pulse), waits up to 100 ms, ignores both
API results and returns the requested minimum. All branches reread the counter
for the next baseline. The original caller supplies minimum 10 and maximum 100.

[`frontend/frame_clock.h`](../recovered/frontend/frame_clock.h) implements those
rules; [`host/frame_clock.h`](../recovered/host/frame_clock.h) supplies the actual
Win32 performance counter and event/timer calls. The event is manual reset,
initially nonsignaled, matching `006DCAE0`. Failed event/counter setup throws
instead of continuing with invalid original state. Counter multiplication keeps
the original low 64 product bits, then the low 32 quotient bits.

Both movie and menu loops now use this clock. The separate Sleep(1)/Sleep(16)
pacing and mouse-local performance counter have been removed. The same interval
goes into mouse/menu input. Other original main-loop operations and the full
controller remain separate dependencies. No fixed simulation rate is claimed.

## Keyboard source

[`input/keyboard.h`](../recovered/input/keyboard.h) recovers three LS3DF methods:

| Entry | Behavior |
| --- | --- |
| 1006F190 | Snapshot previous keys, clear/read current keys, reacquire/pump/retry and original debug toggles |
| 1006F320 | Message-mode consumption or scanned new-key/repeat selection and character translation |
| 1006F470 | Return and clear stored character DWORD |

Missing device returns 80000009 before modifying either snapshot. Read failure
reacquires; input-lost 8007001E loops through the graphics pump before another
acquire. A second failed read clears current keys and returns success. Debug
toggles only run after the first read succeeds; their chord is left Shift,
left Alt and left Control, with F12/F11/F10 edge checks. Retry success skips them.

New keys require current high bit and an entirely zero previous byte. Six
modifier scans are excluded. Translation receives modifier mask 0xD and stores
the low result byte. Message mode consumes/reset pending scan and character.
Repeat checks occur inside the scan loop, so a due repeat may beat a new higher
scan. Release clears the repeat index and returns zero immediately. Repeat
thresholds are strict signed elapsed >350 ms, then >34 ms, using wrapping DWORD
subtraction. Repeats leave the translated character untouched; its getter clears it.

These are native state/device/service reconstructions, not original globals or
class ABI. Invalid native repeat indices throw. All three indirect engine entries
are absent from the frozen inventory, so they earn no census entries.

## Keyboard services and device integration

The follow-up recovers modifier queries `1006F550/570/590`, combined flags
`1006F5B0` and translation `1006F660`. Shift contributes aggregate 1 and
left/right 20/40; Control contributes 4 and 80/100; Alt contributes 8 and
200/400 (hex). Both sides can contribute. The immutable character table has
63 scan/normal/shifted records plus a zero-scan sentinel. Translation truncates
scan to a byte, takes the first match and uses only modifier bit zero to choose
shifted. Caps Lock, Control and Alt do not alter the character lookup.

[`host/engine_keyboard.h`](../recovered/host/engine_keyboard.h) supplies the
concrete keyboard owner. Original LS3DF IGraph +90 creates/configures the
DirectInput device; +94/+98 query initialized/close. Existing ownership is
rejected; failed initialization closes partial resources. Reconstructed source
handles polling/reacquisition, key selection, modifiers and character lookup.
The bridge copies the immutable table from the fingerprinted loaded DLL.

The `--use-engine-mouse` path now also enables this keyboard owner for movies
and menus. Host-selected flags zero mean foreground nonexclusive/no repeat.
Already-held keys are snapshotted at startup. Enter/keypad Enter/Space/Escape
use device snapshots; duplicate Win32 activation is bypassed. Movie Escape
uses the same owner. Other paths retain Win32 transport, including the F9
diagnostic. Native debug toggles do not update original engine rendering flags.
Keyboard creation, format/cooperative setup and final release remain delegated
engine methods. Client input-manager production and preprocessing remain open.
No additional census credit is claimed for these delegated methods; recovered
modifier/translation functions are absent from the frozen inventory.

## Validation and next work

Temporary sanitizer and Windows i386 audits cover clock wrapping, exact bounds,
maximum-first inverted bounds, failed waits, fresh baselines, missing/failed
keyboard reads, acquisition pumping, debug-toggle gates, modifier exclusions,
message-mode consumption, translation truncation, strict repeat thresholds,
repeat priority, release and character clearing. Strict host build, existing
native checks and 158 tooling checks pass. The [byte map](frame-input-map.json)
validates both fingerprints, 29 anchors including the character table, 52
graphics slots and the timer call. The follow-up temporary sanitizer/i386
audit checks all 64 modifier combinations and 131,072 scan/modifier combinations,
duplicate/sentinel lookup, poll-to-menu Enter/keypad Enter/Space/Escape routing,
hold suppression, character lookup and acquisition/read failures.
One new partial client entry: total 115 partial (111 client/four engine), zero
complete; frozen denominator remains 18,783.

Desktop command:

```sh
python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --opening-movies --frames 0
```

Check Escape skips one movie, menu Enter/Space/Escape and Exit/No/Yes. Holding a
key should not activate successive screens. Check focus loss/return and window
closing. Keypad Enter is source-audited; the user's TKL cannot hardware-test it.
The user confirms this keyboard integration, including focus loss/return.
This does not establish the original application's full focus-loss policy.
No agent-side Proton run. Read-only exports live in ignored
`analysis/work/engine-keyboard-loop`, `engine-keyboard-services` and
`startup-input-prerequisites`.

Follow-up: [client keyboard preprocessing and text FIFO](keyboard-preprocessing.md)
are source-recovered and audited. Concrete input-manager production and character
services are still needed before integration.
`006C7870` is broader binding/axis/joystick processing, not a simple scan queue;
its raw decompilation needs assembly review. Startup `006B65D0` also requires
game-data definitions (`006BB540`) and HumanStore loading (`006A18E0`), followed
by profile setup in `006B6620`. None is replaced by a successful no-op.
