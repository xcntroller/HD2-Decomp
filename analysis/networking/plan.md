# Networking workstream

Branch `feature/networking`, isolated from the main P2.2 and tooling worktrees.
Base: local `master` d8b64f8 (2026-11-09). No remote fetch/push, merge or publication.
The root plan, handoff and frozen client/LS3DF ledger retain their main-workstream meaning.

Checked research/source items do not mean runtime delivery. Original executables,
DLLs, Proton and Windows fixtures must not be executed by the agent. Heavy Ghidra
and build jobs run sequentially. Temporary audits are removed after verification.

## N0 — inventory and architecture

- [x] Review client source, fingerprint binaries, inventory functions and trace launcher/server architecture — static research. [Evidence](architecture.md).

Completion boundary: both PE manifests/imports/dependencies/version evidence;
separate fingerprinted original-address inventories; established launcher file,
command line, registry and process boundary; server startup/network dependency map;
client recovered/missing/desktop status; explicit unknowns and next source boundary.
Commercial images, decompilations and Ghidra databases remain local and ignored.

## N1 — server startup and ownership

- [x] Reconstruct launcher configuration, launch specification and bounded command tokens — source only. [Contract](configuration-source.md).
- [x] Reconstruct cold server settings and command files — source only. [Contract](server-settings-source.md).
- [ ] Reconstruct startup/shutdown owners and actual network initialization.
- [ ] Reconstruct admission, callbacks, timeout/disconnect and cleanup.

Each item needs assembly-supported original mappings, focused native malformed/
failure/repeat audits and Windows i386 compilation. Unknown calls cannot succeed.
Source-only checkpoints may be committed locally without inventing desktop acceptance.

## N2 — client networking and discovery

- [ ] Recover Internet registration/list/query and compare compatible providers.
- [ ] Restore Internet discovery with provider configuration separate from transport.
- [ ] Verify retained DirectPlay LAN discovery and independent direct connection.

Start with RpR's existing H&D2 master and OpenSpy compatibility. A project master
requires evidence of necessity; Steam discovery is optional future work. Preserve
original application GUID, ports, query records and callback lifetimes. Windows/
Proton runtime behavior requires explicitly approved user-operated tests.

## N3 — session protocol

- [ ] Recover and implement framing, handshake/version, identity and membership.
- [ ] Recover lobby/readiness, information, keepalive, timeout and teardown.
- [ ] Verify genuine controlled H&D2 client/server session establishment and clean termination.

Trace both endpoints. A server-list query does not establish interoperability.
Gameplay replication, synchronized physics and mission simulation are excluded.
If original admission requires an actual world dependency, document and resolve
that boundary without a fake ready world or expansion into gameplay reconstruction.

## N4 — launcher

- [ ] Reconstruct launcher configuration persistence, validation and launch behavior.
- [ ] Deliver separate launcher/server executables with user-verified startup.

Initial acceptance: independently compiled server consumes real launcher settings,
is discoverable through a supported method, and establishes/terminates a genuine
session with an H&D2 client. Interactive commits require user desktop confirmation.

Continue the first open networking checkbox, researching its full boundary before
implementation. Record results in this directory's handoff, evidence and separate
server/launcher ledger; do not add credit to the frozen client baseline.
