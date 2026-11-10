# Authority, presentation and game modes

The four domains below are independent. Evidence:006C20F0,006989B0,
0069AF10/0069B230/0069B870,00722790,00723D20, and the
[fingerprinted manifest](evidence/manifest.json). No server EXE was analyzed or run.

| Domain | Values actually reached | Meaning and limits |
|---|---|---|
|00982100 low bits|7 offline;3 listen authority;1 dedicated authority;2 client|bit1 authority pass;bit2 presentation pass; `(flags&7)==7` specifically selects offline branches. Authority alone is not offline. Transitional flags from connection/discovery are not playable-world modes|
|Configuration008AE920+04|0/1/2|No submitted MP request/client/host. Neither the process bitmask nor a proof of connection readiness|
|Configuration170/174|dedicated / domain|170 controls absence of local presentation;174==1 enables the reached auxiliary service pump. Separate from rules|
|Configuration224|0/1/2/3|Deathmatch/occupation/objectives/conditional cooperative-style rule arms. A style3 branch does not prove support for campaign co-op|
|Game04,88,1C|controller status / match phase / session discriminator|Do not merge them into one game mode. Match88 is used by spawn/outcome gates; game1C can choose direct spawn presentation|

00722790 host sets `(dedicated==0)*2+1`, local slot00982104=0 for listen host or
FFFFFFFF for dedicated, and resets flags7 on reached failure.00723D20 client sets
flags2 before Connect; reached failure restores7. Connect completion supplies a
local peer slot from the low7 bits of its reply byte. This is not an actor ID.
DirectPlay host/connect internals stay in the paused networking workstream.

## Common owners, conditional execution

| Service | Offline7 | Listen3 | Client2 | Dedicated1 |
|---|---|---|---|---|
|00698510 game +006CA780 scene load|same|same|same|same|
|19-slot registry, Scene2 IDs, Items/frames|same owners|same owners|same owners|same owners|
|Authority0048DB00(true)|yes|yes|no|yes|
|Script004270D0 authoritative contexts|yes, ordinary package|yes, nonoffline package|no independent authority VM pass|yes, nonoffline package|
|HUD, presentation modules, FPV, camera|yes|yes|yes|no local player presentation|
|Application prepared squad automatic spawn|yes|no|no|no|
|Network spawn/peer state|offline loopback subset|authority + own presentation|received presentation|authority only|
|Scene render0069B870|yes|yes unless dedicated flag|yes unless dedicated flag|suppressed by dedicated flag|
|00698F80 application live squad|snapshot|retire|retire|retire|

Registry slots are fixed0..18, not a per-mode replacement registry. Fresh traversals
observe actual registration: ascending initialization/load/activation, reverse
finish20, separate file-unload18. Module modes must be applied to each concrete
receiver. An initialized cold empty container is not a complete positive path.

## Simulation and events

0069AF10 is the shared fixed coordinator. Players processing precedes its pause
gate. Authority advances game28 and invokes authority module processing; dedicated
also updates the world scene without a local presentation pass.0069B230 is the
shared variable coordinator: client-only (bit1 clear) advances game28 there, while
authority advances it in fixed. Do not advance both clocks on a listen host.
Nonoffline paths also use peer/world-health checks, match phase timers and input
publication. Bit2-clear behavior includes Human virtual14 iteration; it is not a
headless camera renderer. P2.5 pause64 gates are still distinct; opening a client
menu is no evidence that a remote authority pauses.

00699320 accumulates process008AE5D8; strictly `>4999` enqueues application event8.
0069EED0 uses a500ms accumulator and excludes match88 phases1/2/3: authority applies
actual Human hazard2000 effects, presentation handles local4000 notification.
00699B50 uses a1000ms gate and branches on rules224. Style3 consumes270/27C and
objective/death/reinforcement conditions. These are actual multiplayer rule
consumers, not offline campaign progression or a reconstructed cooperative mode.

00699290 emits world-state synchronization as application command0x14, which calls
module12's004782C0 writer. Presentation command0x14 calls00478520. Command0x0E is
different: peer side-change notification; never confuse decimal14 with hex14.
Script presentation module4 consumes8-bit opcodes; current source has only the
actual5F visibility receiver and rejects nonoffline publication. Authoritative
script variable/event/timer state is not implied to execute independently on each
client. Co-op would need an explicit mission-script authority policy.

## Required interfaces and unresolved cases

Original mode reads are process-lifetime values, not a permanently captured
constructor argument. Use the current shared mode/input owner. Reuse process
streams and real module registry; preserve fixed/variable phase ordering and
application command drain rather than synchronous world callbacks.

The bounded P2.8 entry can be source-ready without implementing every multiplayer
rule or packet leaf. Complete nonoffline simulation, input-history timelines,
populated snapshot004782C0/00478520 and the exact match progression receivers are
conditional runtime dependencies. Their original leaf bodies must be implemented
before admitting those paths; they must not be represented by successful stubs.
Live host/client verification remains Phase3. Current c09c22c explicitly rejects
these nonoffline update/activation/finish branches; the matrix describes original
behavior, not current delivery.
