# H6 shell, capsule and grenade production — 2026-11-12

Source-only producer/lifetime checkpoint after0398a50. H6 remains open for
immediate melee, Player medical policy, completed-H4 integration and final item
composition. Full asynchronous projectile/collision/combat frames remain H8.
No original/Windows/Proton/application or desktop execution.

Shell0044E5D0 gates actual C8/marker/CC and current Player BEFORE three LS3DF
velocity draws and same64-pool acquisition. It publishes actual134 shell owners;
00454190 retains its first cloned visual across pool reuse, including a later
view change, then writes pose/placement/visibility/update and command.004546C0
hides/updates that visual without releasing it until pool destruction. Native
shell return3 is consumed as completed2; selected00454360 still fails under H8.
The historical cold `decals` label now holds actual shell records, not a second
pool. ShotMgr presentation/finish/failed cleanup drain them while dependencies live.

Capsule00451670/00448CE0 uses actual weapon payload/model, retained source and
module17 particle91. Velocity scales XYZ by payload native38, preserves W, sets
native damage10 and last position30. Selected weapon caller supplies incoming
sound=false/special90=false. Retirement00448FC0 stops particle while the model
is alive, releases the model, then handles existing camera9 policy. Actual
00449290/00449030 async frame bodies remain required H8 guards.

Grenade00450400/0044CFF0 allocates the actual World RigidBody type in the SAME
registry/buckets. It scales direction by item98×payload38 with FLOAT store and
minimum5, uses payload4C fuse, cloned model and original parameter/link/pose calls.
Body bind/configure/reset receives motion, mass2, coefficient0.5, flag40 and the
actual record callback context. It never seeds an invented native5F9 value.
0044D210 closes body collision before model release;004505B0 then destroys and
unregisters the body. ShotMgr's actual114 stable erase retires completed kind3/4
records without recycling them into the bullet pool.

Grenade publication precedes offline0045EAB0 notification and005A7B70 inventory
removal. The current H6 guard at Active AI00530FF0 stays honest at this checkpoint;
completed H4 now supplies that body on merged HUMb9d67aa and is the next import.
No unfinished H4 branch or worktree is required. Capsule/grenade constructors
retract partial owned resources through RAII; original object/allocator/SEH ABI
and undefined inputs are not claimed equivalent.

Verification: prior native shell PASS100 plus new ASan/UBSan/LSan PASS2505 across
100 capsule/grenade lifetimes and actual ShotMgr stable retirement. Checks cover
payload scaling/fuse, missing model, particle/clone/configuration failures and
body-close→model-release→unregister order. Engine/particle/Body/selection/motion/
async callback endpoints are explicit doubles. Strict nested actual World/
RuntimeItem/Human/ShotMgr/RigidBody i386 compile and latest launcher/probe builds
PASS. This is compile-only integration, not full application execution. Temporary
new sources/native binaries/object are fingerprinted/removed in the
[receipt](checkpoints/p2.2-human-projectile-production-20261112.json).
