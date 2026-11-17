# H8 Human command and Active firing callbacks — 2026-11-17

The actual offline Human path now binds0045BD10's remaining vehicle-attack
candidate policy and Active008109F8+114=0052F1F0 line-of-fire callback. The
artificial task whitelist before00528950's common path is removed. H8.2/H8.4/
H8.5/H8.6 and the HUM matrix remain open; this is source-only, not a running game.

## Original boundary

Static client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research-index lookup preceded the one new read-only/no-analysis0052F1F0 export.
Existing command-selection/producer, occupied and inventory exports were reused.
WORK/human-h8-selector22-full.asm starts at the actual function boundary and
resolves calls45C19F/45C1B4/45C1D8. The earlier initial-window asm starts inside an
instruction and is REJECTED as behavioral evidence. Full Active raw instructions
are authoritative over the C export's inferred floating expressions.

## Vehicle-attack selector and common tasks

0045BD10 sorts with the already verified pointer-only ordering before action22.
For each candidate it rereads target armor: Car4 field5E0, or Animation25 D8==2.
A nonarmored target accepts immediately, with no B0/control/inventory query.
Otherwise005328A0 checks current attachment and actual virtual98; only true
permits005328D0's current B4 payload48 kind2 check. If that pair does not accept,
005304B0(kind6,false,false,&scratch) searches that SAME Human. First accepted
candidate wins; no eligible candidate returns NULL. Actual control and payload
receivers remain distinct; no claim/occupancy substitute is introduced. Native
Human B0 NULL dereferences are guarded at the corresponding armoured queries,
not before the native nonarmoured return. Existing kind6 inventory body is reused
and receives no duplicate credit. Type20/34 attachment requirements stay with
their actual H8.4 owners.

00528950 has no admission whitelist. Its22/25/26/32 target loops coexist with a
common path for other task words; the reconstructed pre-switch rejection had no
original counterpart. Removing it retains the actual attachment, movement,
turn/start, common-command effect, target-unavailable and completion receivers.
This does not make unknown downstream commands or absent actors successful.
In particular the existing completion body already has actual8..13/30 cases;
these may now reach it rather than an invented earlier failure.00534BF0's
Human3 handoff rejection and nonHuman capability-layout requirement are retained.

## Active line-of-fire0052F1F0

The final Active table's114 pointer is0052F1F0. World previously handled Guard4,
Script5/Remote6/Area8, then threw for Active3, leaving a mandatory ordinary squad
firing callback unavailable. Active now uses the actual shared Players roster,
not Guard's friendly-perception list or a copied actor vector. It visits FORWARD,
rereading count after each iteration, skips signed-health<=0 and the current
state's Human, and uses fresh model matrices and the actual DD0 primary spheres.
The head260 matrix is read even with an empty roster. Missing required head,
roster Human or physical sphere storage fails explicitly.

Input*FLOAT200 spills XYZ; input normalization retains extended(Y²+Z²)+X² before
the unit comparison and uses the native FLOAT square for sqrt/tiny-axis policy.
Other-Human horizontal differences spill before normalization; X/Z products
remain extended through the positive-forward dot. Sphere traversal is reverse.
The native distance is to an INFINITE line;200 is not a segment/range clamp.
Projection keeps dx/dy/dz extended, saves dz FLOAT, spills projectedY/Z and
residualX, then spills residual(Z²+Y²)+X². Signed-bit tiny/unit/sqrt behavior is
preserved. Tiny line length uses native negative FLOAT1e16 sentinel. The threshold
is radius*DOUBLE1.15 from00810BA0 with no FLOAT margin spill; equality/unordered
does not block. The C export's cast/term order would lose these details.
Guard00558E50 is unchanged and its existing verification is reused.

## Verification and limits

833 strict native ASan/UBSan/leak selector/common-task checks pass. They cover all
control/payload/launcher combinations, short-circuit ordering, genuine no-result,
empty/invalid candidates, common completion task words0..255, omitted initial
motion cases and unchanged special target loops. Actual policy/scalar words are
used; inventory/control/query/terminal completion are explicit endpoints.

10023 strict native ASan/UBSan/leak Active-line/actual HumanRoster checks pass.
The independent straight-ray analytic oracle excludes ambiguous near-boundary
rounding; explicit checks cover the DOUBLE margin, dead/self/behind, beyond200,
tiny/zero/NaN, physical-count failure and roster append/clear during model
callbacks. Frame math and primary spheres are explicit endpoints, not loaded
engine/World execution. Strict actual full module+UI+explicit selector22+Active
callback i386 link and forced O2 launcher/probe rebuild pass, never executed.
Two frozen bodies receive partial credit only; existing selector/task/inventory
bodies gain composition evidence without duplicate counts. No original/DLL/
Proton or Windows fixture was run; no application or desktop acceptance claimed.
