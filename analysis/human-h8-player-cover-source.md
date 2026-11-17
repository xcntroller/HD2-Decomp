# H8 Player cover commands — 2026-11-17

00459740 is now bound from the actual inline Player dispatcher (commands8/9)
through the SAME World, first navigation table, Scene sectors, collision ray and
current Human B0.00537A40 produces the actual task41. This is source-only;
H8/P22-HUM and application/desktop acceptance remain open.

Original static image SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Existing research-index lookup and unknown-ABI/temporary typed-ABI exports are
retained under `analysis/work/mission-world-20261103`. The typed export clarifies
purges but still aliases stack locals; `human-h8-player-cover-move.asm` is the
authority for offsets, x87 comparisons, spills and traversal order. No original
program or Windows output was executed. No new Ghidra export was needed here.

004595F0 initializes the five ideals at89ABD0:
(-10,0,15),(10,0,15),(4,0,3),(-10,0,-6),(10,0,-6).89AB80 is a distinct adjacent
table and is not used. Identity/actual engine SetDir precede translation by the
selected MODEL position plus normalized horizontal command direction times2.
The ideals transform as points, in reverse count order. More than five recipients
reaches native invalid scratch storage and is explicitly rejected. Empty
recipients return before selected/model dereference.

Actual model position is queried again for Scene sector equality (including
NULL==NULL). Nearest-volume(center,10,-1) supplies the height reference. Actual
spatial-cover(center,20) appends ascending; candidates are examined in reverse.
Dark means both light values exist and shadowed<0.2. Other candidates require
standing cover in the command octant, the same sector, and an actual ray hit
with normalY<=FLOAT0.6 (unordered admitted). Ray origin adds FLOAT0.3 to Y;
travel is direction*(point radius+FLOAT0.7), preserving native component spills.
The normal is the fourth output, not a distance/fraction. Candidate radius is
byte13*0.0625. Adjusted height permits inclusive[-2,2], including native unordered
comparison policy. Missing reference fails only when the height dereference is
actually reached.

Nearest-ideal metrics keep XYZ differences extended and compare strictly to a
FLOAT best initialized10000, spilling only winners. Slots displace worse points
into an append-order extras vector; ties retain the old point. Chosen positions
use radius-product FLOAT spills. Chosen count<=recipientCount*0.5 clears ALL
slots and skips extras/ideal fallback. Otherwise reverse chosen/extras removes
points with strict squared distance<4 using stable erase. Reverse empty slots
select nearest surviving extras; remaining slots query nearest-volume at their
ideal and require sector equality. This fallback does not reapply admission.
Native invalid nearest indices/pointers fail explicitly rather than inventing a
successful assignment.

Slots dispatch forward; nearest remaining Human is selected in reverse using a
fresh actual MODEL position. Recipient removal is stable and follows its task
callback. Low-cover byte18 rotates the octant left/right according to the
FLOAT lateral dot product until uncovered or back to the original bit.
005245C0 returns the first low set bit's original8AAC00 vector; zero uses row0.
CRT005244C0 proves all eight XYZ values; diagonals are unnormalized until the
caller. Posture uses fresh lighting and standing cover in the COMMAND direction.
Remaining Humans run in reverse, consume CRT rand (distinct from engine random)
and pass their OWN model position to00537A40, retaining its ordinary near refusal.
Their look normalization retains native extended X subtraction and product.

00537A40 reads the Human model again, compares extended(Z²+X²)+Y² to DOUBLE0.04,
and emits category68/69 with priority0/delay300/flag0 on ordered-near refusal.
Otherwise190 clears BEFORE actual task initialization41/radius0.5; then1C0,
204/209/188/160/218 publish, mode8/9, empty-order notification, and actual group
posture policy in that order. Current B0/lifetime/actor loans validate across
callbacks; no private roster/navigation/engine owner is introduced.

Verification:1364 qualified native ASan/UBSan/leak checks pass, exercising actual
navigation, assignment and producer source. Engine transforms/sector/ray and
previously verified notification/posture tail are explicit endpoints. Cases cover
all octant masks, empty/NULL selection, all five slots, exact-half rejection,
normalY unordered, light/cover/ray/sector rejection, extras separation and
reassignment/nearest fallback, low-cover rotation, missing reference and callback
retirement. Strict actual full module+UI producer i386 link and forced
launcher/probe rebuild pass; no Windows execution. Native undefined W lanes are
retained as typed scratch zero and receive no original-value claim. RAII scratch
allocation does not establish native allocator/SEH/object ABI equality. Three
frozen client bodies receive partial credit; absent CRT initializers and engine
calls receive none. See the checkpoint receipt for fingerprints and logs.
