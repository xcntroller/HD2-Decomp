# H8.2 on-foot avoidance and containing navigation phase

Continues70cb01d. Actual World→factory→SAME Active now proceeds from the
verified00526550 prefix through on-foot0052E310/0052DC10 avoidance, item refresh,
existing direction-flag reconciliation, actual0057A750 query, native result
handling and0052D460 gait. The returned prefix follow flag reaches gait even
whenfalse. Remaining aim/movement/decisions/output still require their bodies;
H8.2/H8/application/desktop remain OPEN. Occupied continuation remains with the
other agent's H8.4; this checkpoint does not implement its policies.

## Selected-player spacing0052E310

174 bit20 returns BEFORE resource access; active query returns before Players.
Actual selected-player lookup runs twice: first existence gate, then fresh
selected identity after own Human attachment gate. Missing second identity is
an explicit native-invalid-state requirement. Any own attachment and selected
Car4 attachment genuinely return, without supplying an occupied policy.

Cached Human/selected positions compare extended (Z²+Y²)+X² against strict
posture1=2.25,2=1,other=.36. Equality and unordered return. Actual selected1B0
velocity uses extended (Y²+Z²)+X²; exactzero falls back to actual cached direction,
while unordered does not. Native FLOAT normalization and W from the selected
velocity/direction precede0052DC10 with blend1/check-angle0.

## Avoidance vector and publication0052DC10

Actual query active gate precedes Human access. Away vector is horizontal,
retaining X/Z extended through first normalization while Y spills. Input
direction flattens Y and normalizes into FLOAT. Optional angle compares the
native extended dot against FLOAT .85; equality/unordered continue.

The two cross products preserve original register/spill differences: first
cross X/Y FLOAT, Z extended (plus stored shadow); second cross X extended,
Y/Z FLOAT, with its Z*stored-Z term in the extended length. Blend adds FLOAT
.15 times direction before final normalization. Length checks stay extended;
sqrt reloads the native FLOAT length. Tiny-vector fallbacks and negativezero
lanes follow native comparisons. No finite filter or generic all-FLOAT vector
replacement is inserted.

Fresh cached Human position supplies XYZ; proposed W initially comes from
input direction. Only after arithmetic do own40000 and actual E0 floor gate
navigation. Actual GAME+54 navigation clamp0052D720 runs, then actual query
request publishes, then BYTE18C=1 BEFORE formation leader/owner queries.
Actual member position resets pointFFFF/changed88, then membership and existing
route update run on SAME formation. No copied formation or navigation queue.
Missing slots retain a required failure after query/18C publication; native
nonmembership performs no slot or route update. Callback owner retirement is
checked before touching stale Human/B0/formation borrowers.

## Navigation clamp0052D720 and route length005230D0

Uses actual global GAME+54 navigation (the World first navigation table), not
B0A0 or the second path dataset. Proposed point lookup ALWAYS runs, including
disabledfalse; missing point or disabled returns before original-point query.
Original position is copied before mutations, preserving aliasing and W.

Different points search actual proposed→original route with mask0. Success and
route length<=4 leave proposal untouched, including Y/W; exactly4 is accepted.
Failed/long route clamps about the ORIGINAL record's centre/radius using the
proposal-minus-original horizontal direction. X subtraction stays extended,
length uses its FLOAT copy and FLOAT Z. Normalized X stays extended through
radius multiply, then FLOAT offset precedes centre addition; Z radius product
stays extended through centre addition. Original W copies to the result and
record Y overwrites the intermediate vertical coordinate.

Same-point path flattens proposal Y, uses FLOAT X/Z subtraction, extended
length/radius comparison, and only clips outside the radius. Normalized X is
extended through radius multiply; all radius-scaled offsets spill before centre
addition. Final Y is record Y. Native same-point clipped temporary W has NO
writer: the typed adaptation retains proposal W and claims no native W value.
Existing navigation finite/layout guards remain requirements; they are not
replaced with a successful missing route.

005230D0 returns0 for fewer than two nodes, invalid node or absent edge. Actual
link payload is scanned FORWARD, first duplicate wins (005247A0), accumulating
uint32 weights with wrap. The x87 return is unsigned sum/256 without a FLOAT
spill. Missing positive-count payload retains its explicit ownership failure.
The same navigation owner/search/records are used; no extra link-table owner.

## Containing query-result order

After avoidance, refresh precedes actual flag134 reconciliation and query.
Result4 preserves native202/206→200, idle-follow1A4 clear and18C clear. Result-6
queues actual rejected-task speech before gait. Results-5/-4 on an unfinished,
not-arrived task request saved1E0 when targetNULL; a present target instead gets
its actual lazy model update with NO fabricated target-position request.
Retired target policy remains required. Result-10 with active task25/26 requires
actual00532190 specialized replanning after earlier query publication. Other
results genuinely fall through. Gait runs after those effects with native local
follow flag. World passes only unattached Humans through this phase; Runtime
requires remaining aim/movement/decision/output, or H8.4 occupied continuation.

## Evidence, verification and remaining work

Original SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index preceded bounded read-only/noanalysis exports. Reused00526550
containing C and0052E310 from `human-h8-authority-policy`,0052DC10 from
`human-h8-authority-movement`,0052D720 from `human-h8-avoidance-query`.
005230D0 was newly exported under `human-h8-avoidance-route-length`. Raw
avoidance/player/query/route-length/link-index ASM resolves misleading C types,
x87 return, scalar spills, discarded coordinates and W provenance. No project
mutation or original game/Windows fixture execution; H8.4 branches untouched.

Focused ASan/UBSan/leak PASS9201 assertions across100 lifecycle matrices plus
wrapped long-route check. Actual MissionNavigation/records/links/search, AI
resources/query, formation slots/route update and avoidance/clamp/result kernels
run. Human/frame/World/selected-player/E0 endpoints and reused query/gait/speech
receivers in the containing-phase fixture are explicit doubles. Covers route4
vs nextweight, first duplicate, uint32 wrap, missing/invalid links, first lookup
even disabled, second-query short circuit, same/different-volume Y/W and radius,
zero/parallel/cross/blend math, strict posture spacing/NaN, two selected queries,
attachment returns, query/floor/40000 gates, owner/clamp failures, slot publication
before failure, query results/speech/model failures and followingtrue/false.
Strict i386 instantiates complete actual World authority through navigation and
required Runtime remainder, plus direct avoidance APIs. Final launcher/probe
compile/link only. Tracker/ledger and exact removed temporary audit fingerprints
are in [receipt](checkpoints/p2.2-human-h8-avoidance-20261116.json).

Four new partial bodies0052E310/0052DC10/0052D720/005230D0 and extended00526550.
No complete promotion, duplicate005247A0/engine/navigation/formation/gait credit,
or claimed H8.4/application/desktop completion. Next: containing aim policy,
0052B810 movement/target family,005276E0 cover/event decisions, remaining specialized
producer/effects/group completion and offline receivers; then H8.3.
