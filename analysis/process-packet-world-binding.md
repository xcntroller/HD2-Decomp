# Process packet ownership and actual camera binding — 2026-11-10

Source-only continuation of the FULL P2.2 checkbox. Application loading still
DECLINES; no first frame, engine execution or desktop acceptance is claimed.
[Receipt](checkpoints/p2.2-process-packet-world-binding-20261110.json).

## One control/process lifetime

Native0072DE00 owns the three broadcast streams208/244/280 and both packet queues
across game/world retirement. Source StartupSession now owns SAME shared
HumanInputShared, SessionBroadcastPackets and SessionLocalPackets. World borrows
the process streams for prepared spawn and Script; no World destructor discards
queued bytes. SessionPacketWorldLink binds the complete World only after all
module owners exist, then unbinds before their destruction. Missing/unloaded
World is genuinelyfalse at0072C3B0's existing gate; queue payloads remain owned
by the process and retire when drained/reset. Foreign unbind cannot detach the
actual receiver, and duplicate bind rejects. There is no successful receiver
when a loaded World lacks a required packet implementation.

ColdControlFrame now reads the ACTUAL broadcast cursors. The old four-array
words/bits shadow is removed: only the unrecovered2BC request stream's separate
scalar words/bits remain. SessionLocalPackets keeps its real FIFO payloads and
mirrors queue counts into SAME ColdControlFrameState; those counts are not a
second queue. Startup uses this actual pump when World is bound or bytes remain.
Otherwise it retains the previously accepted empty cold control path. The
application does not construct this source World yet, so this is source-only
wiring, not a newly enabled interactive mission/control path.

0072E4E0 reset and final0072E460 can flush the actual packet owner without a
preliminary cold reset. Required1000ms flush runs ONCE between graphics/firewall
restoration and field/record/queue clearing. Final lock deletion and idempotence
are retained. Unknown live transport, request buffers or enabled timer still
reject before mutation; populated source broadcasts/queues are admitted only to
the actual flush/clear receivers. No second reset flush or per-world clock/cursor.

## Module13 and camera receivers

0048DC50/70 now dispatch using the fresh original startup registration, returning
false for absent/out-of-range source slots. Presentation module13 uses the actual
receive_scene2_presentation_packet→WorldActors→SpawnDecoder. Unsupported positive
module/authority receivers explicitly require their real implementation. Source
00475630 has already established absent frozen membership and gains no credit.

Initial spawn identity>=32 decodes the real pending broadcast, then006125A0 and
006123A0 use SAME SpawnPoint.A8 Human; actual record/Players/FPV/HUD/camera owners
remain shared. Reserved identities0..31 still REQUIRE005A5FB0/005418E0: original
005A5FB0 invokes the discard grammar for null/missing B0/notkind1-or7, and actual
Human virtual1C otherwise. Null is not permission to skip payload. Read-only
exports preserve those gates and full original24-command discard dispatch;
neither source receiver is silently supplied or newly credited here.

GameCameraLoading now binds existing00469300 simulation to SAME actual Players,
current/selected Human, inline98 owner, FPV, frame registry, scene tree/dynamic
buckets/collision resources and current Startup graphics input/profile inversion.
It rereads session6E input gate, session64 pose suppression and signed local ID.
Queries are temporary scratch over real shared collision owners; no copied
geometry or synthetic hit. Simulation rejects before loaded14. Breathing reads/
mutates the persistent actual process FLOAT00839268, initial bytes0A D7 A3 3B
(.005), retained across worlds. This binds an existing verified body; it does not
supply the surrounding0069B230 update loop or engine render callbacks.

## Focused checks and remaining scope

Native ASan/UBSan/leak fixture passes actual prepared-spawn serialization→pending
process stream→flush/FIFO→Scene2 decoder→SAME Human/roster/camera callback order,
repeat/duplicate roster, queue survival across unbind/unloaded World, decoder
failure retiring only current bytes/unlocking, actual cursor gates, single-flush
reset/final locks and100 empty cold lifetimes. Engine/actor/camera leaves are
explicit doubles; no scene transform/render result is inferred. Strict actual
complete i386 loading/activation/World/module/packet/camera/finish-binding graph
passes, application links and27 tracker tests pass. Previously verified camera
math/unchanged subsystems were not re-audited. Fixtures/binaries hashed/removed;
logs/research retained under analysis/work/mission-world-20261103.

Two additional frozen partials0048DC50/70; zero complete/desktop promotions.
Source-only callers still require the actual008AED48 shared transient finish
owner, meaningful integrity observations, reached required positive receivers,
original game update/render coordinator/callback services and user Python desktop
failure/repeat acceptance. Both P2.2 checkboxes remain open. Continue the FULL
checkbox after this checkpoint.
