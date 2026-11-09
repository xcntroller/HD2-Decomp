# Local spawn dispatch and first Human camera — 2026-11-09

Source subsystem within the FULL P2.2 loading checkbox. GameApplicationState
still declines loading; no world frame or desktop verification is claimed.
[Selected original bodies and verification receipt](checkpoints/p2.2-camera-local-dispatch-20261109.json).
This follows [activation and prepared spawning](session-activation-loading.md).

## Pending local packets

SessionLocalPackets borrows the SAME ControlStartupState, cold-frame timer state,
process HumanInputShared and three broadcast streams.0072C170/0072C230 allocate,
sample actual CRT clock007E814F, copy, then append under lock430. FIFO payload
ownership survives decoder failures; dequeue/unlock precedes callbacks.0072C3B0/
0072C0B0 read five-bit module IDs, stop at terminator19, and use actual authority/
presentation receivers. Missing/unloaded world or a missing module is genuine
false; app-control20/input-history flag20 require their actual receivers.

0072CE70/0072CD50 ordinary offline7 retain record08/6C gates under lock418;
connected peers reject until their real receiver exists.0072C5D0 keeps signed,
wrapping period/catch-up behavior, the genuine initially disabled0073EA70 timer
and process0097C90C flag. Streams flush in ascending order: append terminator19,
enqueue presentation, THEN reset cursor.0072BF90 target0 alone never enqueues.
Authority request stream2BC/enabled timer producers remain excluded. This source
pump is not yet connected to the host's cold control/world execution path.

00475630 actual module13 virtual24 reads Scene2 identity10 bits, routing reserved
0..31 through required005A5FB0 and ordinary identities through the actual actor
virtual1C. This body is absent from the frozen inventory and receives no credit.
006125A0 SpawnPoint subtype0 decodes player/Human identity, position/direction,
signed generation byte, name/value and full soldier payload BEFORE publishing
0072E940 and006123A0. Offline7 skips the native remote-generation/stat path.
006123A0 uses the SAME already-created A8 Human. For local player and spawnA0=0,
camera commands0/2 precede actual Players roster append, including the first
Human. No second Human or decoded-pose application is introduced.

00727420/00727660 retain WORD equipment identity/position, unsigned byte count,
append semantics for carried records and raw eight-word bookkeeping.0073D0A0
consumes the native bounded string limit and forces the last consumed character
to NUL if necessary, without draining the rest.0073D530/0073D710 preserve actual
process quantizers, signed DWORD division, compressed Y flag, strict near-zero
step comparisons and extended trig. Queued byte payloads use bounded owned
DWORD copies rather than the original temporary borrowed-buffer ABI.

## Camera first update

00464DD0 constructs the CameraModule at static BSS0089AEA0. Original unwritten
50/58/27C initially zero; they are not guessed heap defaults. Native24C sector,
25C hit and288 look target are typed borrowers. Scalar2A4/2A8 are FLOAT amounts.
0046CB20 offline command0 distinguishes parameter presence from parameter value,
reads actual active camera pose when enabling26D, and otherwise calls actual
0046C370. Command2 is a genuine offline no-op. No-current-player0046C370 retains
the old target/configuration and still processes the signed280 sentinel.

00469300 ordinary unattached Human update reads actual graphics EC/F0 and the
CURRENT settings39 inversion. Current Players2C differs from selected28: first
roster addition can leave selected NULL. Gates26F/game6E, configuration2 B0
virtual4C direction/height check, yaw/pitch limits, and flag1/8 target-marker
orientation preserve that distinction.00468130 then00466970 precede the final
camera update, actual active-camera scene-sector query every500ms, recoil timers
29C/2A0 and capped zoom timer294. Debug0046BEB0, detached0046B550, attached
0046ADA0, target configuration8 and look-target producers explicitly reject.

00468130 keeps native quaternion direction interpolation: near-forward planar
angle uses the DOUBLE0080FF20 (~.08), temporary +.5/- .5 rotations, first-snap
configuration smoothing1000, native angular speed16 then6, and actual engine
SetDir/Slerp/GetDir calls. Posture/height offsets and simple noncollision rows
remain distinct from flag80 collision rows. Actual sweep00718530 precedes the
separate006F7C90 ray. Current Human DD0 descriptors are excluded around BOTH
queries and restored before subsequent first-person/position work. The ray hit
is pushed along normal*(radius+.05); dynamic-frame flag40/name-prefix matches
are ignored. Distance254/hit25C history, no-hit return, close-hit and temporary
280 behavior use actual identities. Auto first-person requires real FPV enable.
Exception scopes restore query exclusions and temporary settings; they do not
claim original SEH/unsafe malformed-data behavior.

00466970 uses actual main-camera point transforms minus world translation for
90 positional and80 directional offsets; shake60/70 and previous30/40 remain
separate. Zoom breathing observes actual B0 action52C/focus4D4, signed stat4
conversion005A6530 and process FLOAT00839268 (initial .005). Native nested trig,
FLOAT stores and three components are retained. Actual target model100/kind9/
EC Scene2 binding resolves the Human for head/body distance fade, independent
of current Players Human;2A4/2A8 step by .1 per update before005A9760. FPV58/59
skip that fade. A target B0 direction is virtual4C: Player00811848→00584B50,
AI tables→0056FC00; virtual48/current direction is a different getter.

005AC1C0/005AC1B0 use the SAME inline98/DD0.0075C960/0075CAC0 traverse actual
primary, shot, secondary descriptors for shared exclusions or bucket remove/
insert.007860B0 reads movementBD4; sentinel00816C6C (-10000.099609375) leaves
height output untouched. Actual DD0 f1 bone840 worldY+.2 compares extended
against height; valid means threshold<=height. Its separate AL result reads
LookB8==5. No DWORD stores a host pointer.

## Ray collision

CollisionRay reconstructs006F7C90's dominant-axis traversal and006F5890's
near-vertical branch over the SAME CollisionTree and dynamic buckets. Original
full-segment Y cell limits survive dynamic-hit shortening; the vertical branch
also retains its original static bounds. Z/X dynamic traversal006F2C50 observes
both descriptor flags and layer masks, including own-Human exclusion bit8.
Actual triangle receiver006F53F0 preserves sorted-X early exit, Z bounds,
positive-dot backfaces/bit40000 two-sided behavior, strict best parameter and
classification1..7 barycentric tests.006F4310 preserves slab entry ties and
inside-start Y behavior. Required primitive40/41/42/44 and80/81/82/83 receivers
are reconstructed; unsupported families throw. CameraCollisionQueries returns
borrowed real tree-record/descriptor identity plus flags/frame, not synthetic hits.

Focused native ASan/UBSan/LSan checks cover actual source packet/Players/Human
owners, captured base_gamecam.set and real source tree/descriptors/sweep/ray,
player-state getter/breathing gate, first-person selection and injected query
failure cleanup. Engine transforms, inline presentation and containing world
lookups are doubles. Strict actual-host i386 template compilation and app link
pass. Temporary fixtures are hashed and removed at this checkpoint. No engine
code or Windows fixture was executed; actual world/VM/presentation/other actor
physics composition, genuine first frame and user failure/repeat acceptance
remain required. Continue the full checkbox after this checkpoint.
