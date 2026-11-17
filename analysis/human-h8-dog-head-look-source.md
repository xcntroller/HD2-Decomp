# H8.4 Dog head-look command/timer source — 2026-11-17

Complete005D8070 request and005D82B0 decision/timer source is now on the SAME
Dog actor's4E8..52C owner. This is independent of the Body action/posture/stance
owner and must not be merged into it. Live activation/retirement and containing
Dog frames remain OPEN; no automatic frame dispatch or successful activation is
added by this component. It is a required dependency of005D5620 and005D4DB0.

## Original behavior and ownership

Research index consulted005D8070/005D82B0 before bounded raw reads. Existing C
under WORK/h8-4-dog-boundary-originals and original raw/constants retained in
WORK/human-h8-dog-head-look use client
SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.

005D8070 masks command bits0/4 and returns immediately for SAME mode without
reading/copying target, parameter or consuming RNG. Changed mode is published
before all four target words. Commands2/6 enable look, reset504/505, set wait=-1,
then consume signed CRT%30 for native FLOAT .01*remainder+(.6 or1) speed;6
publishes timer524 before RNG. Command4 sets504 and uses EXISTING52C, ignoring
its argument. Command8 consumes signed CRT%1500 first, publishes52C and timer524,
then uses subtype0/1/2/3+ branches. Duration uses UNSIGNED CRT division over
wrapped52C*2000 and adds wrapped52C*1000. Division-zero remains a required fault
AFTER the RNG call/prefix. Incidental integer returns are preserved in source;
World ignores them as the original containing callers do.

005D82B0 retains inactive/mode/subtype early returns BEFORE publishing4F0.
Signed wait comparison, wrapped timer subtraction, strict speed>.6 halving,
changed-target504, completed505 and rearmed delays follow the native labels.
The random target update reads actual model direction, normalizes in Y+Z+X
order, consumes two signed CRT draws, applies original quaternion Y-axis rotation,
spills scaled XYZ, then rereads model position. It publishes510 and only on the
native common tail copies it to4F0. A terminal .75 engine-unit draw produces750
through original __ftol(domain-checked source adaptation), distinct from CRT RNG.
No source processing loop, copied process RNG or synthetic target is introduced.

Actor owns this single typed state. World request/update methods bind actual
process CRT/engine random streams, Scene frames and original rotation endpoint.
Cold actor construction does not establish a live head-look producer: explicit
source provenance rejects calls until005D4DB0's initialization is supplied.
The initialization method implements its known4E8/4E9/504/505/508/50C/510XYZ/
520/524/528/52C stores; it does not invent4F0. Undefined fourth scratch lanes use
typed padding and receive no native-provenance/equivalence claim. Later explicit
requests preserve their actual provided fourth word. No additional compatibility
policy beyond the two previously authorized fixes is inferred.

## Verification and continuing boundary

Native132 ASan/UBSan/leak assertions cover every2/6/8/fast mode and0/1/2/3
subtype, repeated same-mode no-copy/no-RNG, real completion/countdown chain,
random target and common-tail publication, inactive no-tail, existing parameter
in command4, signed negative CRT draw, throwing RNG prefix and wrapped divisor
fault AFTER draw. Model/math/random endpoints are explicit doubles. Native state,
request/timer code and ownership are actual source; no live engine is run.

Strict complete actual World i386 request/update instantiation compiles/links
against SAME Dog/process/frame/math owners; initialization is explicitly invoked
by the compile-only fixture. This does not prove live005D4DB0 integration.
Launcher/probe, ledger and removed audit fingerprints are in the
[receipt](checkpoints/p22-h8-dog-head-look-20261117.json). Two frozen partials only;
no full H8.4/complete/engine/desktop/readiness claim. Continue the full H8.4 goal.
