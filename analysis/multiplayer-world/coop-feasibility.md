# Campaign co-op feasibility — inference, not an original mode

The shared world makes reuse plausible. It does **not** establish that original
multiplayer runs campaign missions cooperatively. Rules224==3, strings mentioning
cooperative play and multiple Humans are insufficient proof. This assessment uses
the verified [entry](shared-world.md), [mode](authority-modes.md),
[identity](player-control.md), [packet](packet-synchronization.md) contracts and the
preserved [preparation](../future-dependencies/p2.4-game-handoff.md),
[persistence](../future-dependencies/p2.6-save-load.md) and
[results](../future-dependencies/p2.7-results.md) research.

| Area | Verified original offline | Verified original multiplayer | Inference / proposed extension |
|---|---|---|---|
|Mission selection|GameData tree, campaign progress and descriptor traversal|Selected map imported to inline mission; objectives kind12 and rules metadata|Reuse actual loader, but design explicit campaign-descriptor distribution and resource agreement. A campaign resource string alone cannot replace its descriptor/progress|
|Squad/equipment|Application live squad, HumanStore identities, two stock owners; prepared spawn|Peer records/full prepared soldier and separate side/spawn rules|Assign peer→prepared soldier without replacing squad/HumanStore IDs. Decide which authority owns stock mutation, selection locks and equipment refunds; no original co-op consensus protocol found|
|Controlled Humans|One local selected/current Human among squad roster|One local peer/camera per process; remote actors in same registry|A peer can plausibly control an assigned Human. Existing offline roster switching must not let two peers claim one Human; design authority-approved possession transfer|
|AI squad|Actual Human/B0 AI and script owners|Authority module pass and nonoffline script package; clients receive presentation|Likely host-authoritative squad AI. Prove campaign scripts do not require offline7 branches/local singleton selection; do not run independent campaign VMs on every client|
|Mission logic/objectives|MissionA8, script contexts, variables/timers and result producer|Authority objectives/rules with presentation packets|Replicate presentation/objective transitions via established module interfaces; campaign-specific opcodes/closures and resource refs need a reachability study, not a guessed world event bus|
|Deaths/reinforcements|Injury/death/profile/result effects and squad selection|Peer spawn/respawn, side/match phase and conditional rule3 reinforcement gates|Define disconnect/death ownership and campaign failure policy. Original multiplayer score/respawn must not silently replace campaign permanent consequences|
|Camera/HUD|One Players/Camera/HUD/FPV using selected Human|One local presentation set per client; dedicated lacks it|Strong reuse for separate-process clients. Split-screen is a different architecture and not supported by this evidence|
|Save/restart|Application preparation then existing world/module/script/actor replay; nontransactional failure|App7 shares snapshot receiver; dynamic snapshot0x14 and roster0x15 are distinct|Do not assume saved snapshots include live peer connections, sequence queues or input histories. Proposed host-coordinated quiescence/rebind/restart needs a new protocol and fully resolved P2.6 leaves|
|Results/progression|HumanStore/profile, medals/stats, campaign continuation, real save outcomes|Network finish retires application squad; match rules/peer stats|Major incompatibility: preserve canonical campaign soldier data through multiplayer cleanup, select authority for profile mutations, avoid double awards/repeated debrief writes. This would be an explicit extension|
|Joining mid-mission|Not an offline feature|Scene2 count comparison plus populated roster/dynamic snapshots|Count equality is insufficient for co-op resource/script agreement. Proposed stable campaign snapshot handshake/checkpoint policy must preserve original actor domains and validate references|

The most promising architecture is one authority running the existing campaign
world/AI/script services, with multiple clients presenting assigned Humans through
the existing process packet and module interfaces. This is an architectural
inference. It requires deliberate decisions around every `(flags&7)==7` branch;
simply setting flags3 would select different spawn, script package, match and
cleanup behavior. Calling a network authority “offline with more cameras” loses
these real differences.

No co-op implementation should start by inventing a unified player/actor/profile
identifier or a second inventory/scene. Preserve each original domain and define
explicit checked mappings at assignment, possession, disconnect and restore.
Any new stable peer-to-campaign identity or protocol version is a **proposed future
extension**, not recovered original state.

Research gates before a co-op design can be called implementation-ready:

- Complete a real P2.3 world/control lifecycle and P2.8 bounded source handoff.
- Validate real authority/client packet histories, populated0x15/0x14 and actual
  Human action paths against reconstructed controlled Phase3 server/transport.
- Audit the selected campaign's reached script opcodes and offline/mode predicates;
  identify the single authority and each actual presentation consumer.
- Resolve P2.6's remaining actor/script async state and repeated-load cleanup
  before claiming shared checkpoint/load compatibility.
- Specify stock/profile/result ownership and failure/reconnect policies as an
  explicit extension; preserve single-player acceptance as a separate behavior.

These gates are not new universal P2.8 Phase2 requirements. Co-op proposals remain
nonblocking, inferred/deferred graph relationships. No original co-op support,
network success, save compatibility or desktop result is claimed.
