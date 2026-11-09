# Startup module registry — source contract

[StartupModuleRegistry](../../recovered/networking/startup_module_registry.h)
reconstructs the separate HD2DS registry algorithm, not its concrete game modules.
No engine callback is replaced by a default successful body. This remains source
work under the open N1 startup/shutdown checkbox.

`004A3950` owns the supplied name/index and stores the object in the index at
008D5058. The19 DWORD slots end at008D50A0; they begin in zero-filled BSS.
Registration overwrites a prior slot without rejecting duplicates. `004A3A70`
clears its recorded index before releasing its name, even if a newer object has
overwritten that slot. Native array bounds checking replaces original unchecked
indexing. Receivers are borrowed; the registry must outlive registrations and
the active receiver must survive its callback. Native string storage replaces COW.

`004A3AB0` visits nonnull entries ascending and invokes virtual+04. `004A3AE0`
also visits ascending and invokes+08. Both read the live table for each iteration;
they do not snapshot, reverse the cleanup order, or interpret a success boolean.
Required receiver exceptions propagate. These four original functions receive
partial source mappings; no concrete module constructor or delegated body does.

Static research follows base constructor004A3950 through thunk0040A9F7, the
derived constructors and their CRT initializer callers. Most reached lifecycle
hooks resolve to actual RET bodies such as0041FA40/0041FA60,0042DA50/0042DA70,
00462510/00462530,00490AE0/00490B00,00491A50/00491A70 and004939C0/004939E0.
These absent/trivial helpers receive no new credit. Constructors still own real
data; empty lifecycle hooks do not permit successful replacement of their other
virtual methods or omission of required containing owners.
The [target matrix](startup-module-targets.json) records the traced slot/name/base
offset and both resolved methods directly from the fingerprinted PE.

Careful base adjustment matters:0048D830 passes this+4 to004A3950 at0048D85C..69.
The registered vtable is0084EC18 (+04/+08 resolve to0041FA40/0041FA60), not its
outer vtable0084EC64. Derived00481EB0 and004A1A40 also replace their base tables.
Do not take the first constructor vtable or adjacent entries as the final one.

Items registers at slot3 via008113D0→008178D0, final vtable00857048. Its real
startup00817C90 sets+7E0=-1 and clears the eight-byte-record deque at+80C..834.
Shutdown00817DB0 calls actual deleting destructors on up to500 pointers and
mutates a later pointer vector using a shrinking forward loop. That loop must
not be replaced with an assumed complete vector clear. Cold construction starts
those pointers/vector/deque empty; actual Items consumers and populated cleanup
remain required containing work. No Items body is implemented/credited here.

Read-only exports are server-module-registration, server-registered-modules,
server-module-startup and server-module-derived-registration (20261109); registry
callbacks also retain their earlier server-bootstrap-deps exports. Ignored refs
files trace registry addresses, canonical/thunk constructor callers and statics.
All used JSON/C pairs are fingerprinted and hashed in [evidence](evidence.json).

[Focused receipt](startup-modules-verification.json): native ASan/UBSan/LSan,
null slots, exact ascending init/cleanup sequence, mutation of later entries,
overwrite/old-registration removal, actual receiver exception propagation,
bounds guard and1000 registration lifetimes. Strict Windows i386 fixture linked,
not executed. Temporary audits hashed/removed. The original module owners,
bootstrap engine services and live session remain unintegrated.
