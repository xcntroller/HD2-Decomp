# Sound-manager movie coordination

Source-only recovery, 2026-10-30, following [catalog/primary selection](sound-catalog.md).
Previous settings/configuration/catalog work and AGENTS.md refinements were
committed as039e528. This slice does not change the live preview.

Follow-up: [sound runtime](sound-runtime.md) now composes the full update and
registered data owner. It proves zero-fill provenance for the actual static
singleton; the unknown-field discussion below remains specific to an arbitrary
borrowed instance without that provenance.

[sound_movie_channels.h](../recovered/frontend/sound_movie_channels.h) implements
the two-channel movie phase and its selection/queue helpers using the actual
catalog, settings, primary selection and IShow change operations. It borrows
manager state; it does not replace full manager construction or destruction.

| Original | Recovered contract |
| --- | --- |
| 00432400 | Ungated secondary opened query |
| 00436ED0 | Signed secondary fade rate, signed duration clamped to at least1 |
| 00436F10 | Schedule both fades and queue a primary request |
| 00436F60 | Secondary selection and explicit cancellation |
| 00436A42..00436BC5 | Entire movie phase within update004368D0 |

[Evidence map](sound-movie-channels-map.json): both fingerprints,21 instruction
anchors,26 direct calls, two client virtual targets and ten engine slots.
The final manager's +60/+64 targets are004370F0 primary selection and00436E90
primary fade scheduling. Constructor evidence remains in the
[settings map](startup-settings-map.json); its final body leaves secondary
target5C/current60/rate64 and pending6C unspecified. Native state uses unset
optionals and throws if those undefined values are consumed. Selection defines
the pending flag; queuing defines request70/duration74/position78. No default
secondary fade or whole-manager initialization is invented.

## Secondary selection and queue

00436F60 first queries secondary opened, even without the shared gate. An opened
movie blocks requests other than-1; pending6C then blocks every request,
including cancellation. Cancellation queries opened again, stops/closes if
opened, releases through IShow+04 and clears slot68. It does not restore the
primary fade. The slot is reloaded after each callback, as in the original.

Other requests create a movie before validating the index, even when negative,
out of range or gate2 is clear. Selection uses group12 and unsigned CRT random
modulo, without calling the catalog loader. A failed first factory can be retried
by the existing change helper. That helper receives volume0 and loop0. Current60
is reset to0 even on helper false. Helper true schedules secondary to1 and
primary to0.15, both over2000ms; this includes gate-disabled and failed-open paths.

00436ED0 stores `(current-target)/max(signed-duration,1)` as float32, preserving
the sign. This differs from the primary scheduler's absolute rate. Secondary
frame advancement always subtracts `milliseconds*rate`, including a rising fade.

00436F10 halves signed duration with truncation toward zero, schedules secondary
then primary toward float0.01, sets pending and stores request/halved duration/
position. This is ungated and does not require movies. Stored metadata survives
request consumption. No unsigned conversion or whole-duration substitution occurs.

## Per-frame movie phase

The original tests gate2 once before active voices at004368D9. Movie code follows
those voices at00436A42, before application/listener/driver work at00436BC6.
`advance_sound_movie_channels` takes that earlier gate snapshot explicitly;
it is not a callable replacement for the whole manager update. A queued primary
selection still passes the live shared gate to the actual movie change helper.

1. If primary target differs from current, advance in the target direction using
   the absolute rate, clamp, then update the opened movie's scaled volume.
2. **Only a downward primary clamp consumes pending.** Clear pending before
   primary selection, then schedule primary back to1 using stored halved duration.
   Equality and upward arrival leave pending untouched.
3. Advance secondary using its signed rate, clamp according to the original
   target direction, then update its opened movie's scaled volume.
4. Query secondary status. Exact1 or an unopened result triggers another opened
   query, conditional stop/close, release and clear, then schedules primary to1
   over10000ms. Other status words are not treated as completion while opened.
5. Exact primary status1 seeks to0 and plays again, without an opened query.

Comparison uses the extended x87 intermediate after storing current as float32.
NaN enters the unequal path and follows unordered branch behavior; signed zero,
negative elapsed time and float32 rates retain their effects. A nominal100ms
fade from1 to0.01 can miss the clamp at exactly100ms because its stored rate is
rounded; an additional tick consumes the request. No elapsed-time completion
shortcut is added.

[SettingsServices](../recovered/host/settings_services.h) now forwards checked
IShow+04 release ->10070FD0 and+2C status ->10072160. Existing factory/opened/
open/stop/close/volume/seek/play operations are reused. DLL/codec/device behavior
remains delegated, and all referenced owners must outlive these borrowed fields.

## Verification and remaining work

Temporary ASan/UBSan audit passes for2016 independent x87 scheduler results and
4320 frame cases, including NaN/infinity, signed zeros, negative/zero times and
extended comparison versus stored float. Composed cases cover selection blocks,
cancel/release ordering, factory retries/failure, gate clear, failed open, queued
switch/seek/fade, pending equality/upward behavior, exact status handling, primary
restart, slot replacement during callbacks and undefined-field guards.

Real DTA/LS3DF adapters strictly compile/link as i386; no Windows fixture is run.
Existing native checks and all158 Python tests pass. New and adjacent maps
validate; seven altered instruction/call/vtable maps are rejected. Temporary
audits are removed, with no permanent new tests or original game execution.

Three new partial client entries:00432400,00436ED0 and004368D0, the last credited
**only for its movie phase**. Total264 partial (259 client/five engine), zero
complete; denominator18,783. Queue00436F10 and selection00436F60 are absent from
the frozen census and receive no credit. Delegated engine bodies receive none.

The subsequent [runtime recovery](sound-runtime.md) now implements registered
data/resource lifetime and full voice/movie/listener update with this gate order.
Current next work is playback/cache production and restore, shared gate/control
and concrete application/state/protection/cache owners before live launch wiring.
