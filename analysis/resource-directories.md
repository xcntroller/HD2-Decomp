# Original resource directory registration

[`io/resource_directories.h`](../recovered/io/resource_directories.h) recovers
the ordinary directory region of 006DBE60, recursive Win32 enumeration through
006ED760/006ED6B0/006ED570 and directory-list insertion/removal for all five
client cache owners. [Concrete Win32 and graphics forwarding](../recovered/host/resource_directories.h)
compile/link for i386. This is source-only, with no change to live menu input,
resource lookup or rendering.

## Startup ordering and owners

Original 006DBFC0 calls archive setup, then 006DBE60, then input/localization.
The recovered region registers these roots in order:

| Root | Category | Original owner | Add / remove bodies |
| --- | --- | --- | --- |
| Maps | 0 | GetIGraph, +110 | Original LS3DF 1006FF80 |
| Maps_C | 0 | Same graphics list | Same |
| Maps_U | 0 | Same graphics list | Same |
| Models | 1 | 008AEC90 | 006D7590 / 006D7770 |
| Sounds | 2 | 008AEC54 | 006D8550 / 006D8730 |
| Models | 3 | 008AEC18 | 006D9640 / 006D9820 |
| Models | 4 | 008AEBDC | 006D6B80 / 006D6D60 |
| Models | 5 | 008AEBA0 | 006D7590 / 006D7770 |

Categories one and five share method bodies but own DISTINCT state. The source
keeps five separate client lists; it does not merge them by root or function
address. Their directory fields are at original +24 (vector begin +28/end +2C).
Loaded-resource trees/objects in the surrounding cache owners remain separate,
unrecovered work; this is not a full cache reconstruction.

After the eighth call, 006DBE60 performs a protection-state operation through
00A59000, with a fresh graphics/window query and state-table effects. That
postlude remains excluded, just like archive setup's preceding protection
operation. `setup_resource_directory_trees` implements only the eight ordinary
directory operations. No fake successful postlude enables full startup.

## Traversal and path rules

Register each root BEFORE enumeration, preserving its spelling. Registration
does not require the directory to exist; missing searches simply stop. Append
the literal `\*.*` for FindFirstFileA, process its first returned record, then
FindNextFileA until false. Each parent search remains open through nested child
enumeration. Close only valid handles, after all children finish. Order is
depth-first preorder in Windows API order, without sorting.

For each record, construct `parent + '\\' + name` and lowercase the entire
composite before filtering it. Require attribute bit 10 hex (directory), skip
EXACT `.` and `..`, then register and recurse. Other dot-prefixed directories
are included. Reparse directories are not excluded. No deduplication or cycle
policy is invented for traversal. The normal default C-locale CRT branch
lowercases ASCII A..Z and preserves other bytes. Descendants are lowercased even
when their root was supplied with uppercase characters. Trailing separators
are not stripped before constructing a child, so doubled separators persist.

An unknown category registers nowhere but still enumerates and recurses; the
source keeps that default-switch behavior. This is a directory search, not DTA
archive enumeration, and uses Win32 Find APIs rather than CRT _findfirst.

## Native client directory lists

Insertion first dispatches original virtual +1C removal. Final derived vtables
show that this invokes the matching removal implementation in all five owners.
Then insertion constructs its own copy, appends a backslash ONLY if its last
byte is not already a backslash, and appends at the vector end. Forward slash
is not a substitute for backslash; case is not folded at this layer.

Removal independently normalizes that same trailing separator and scans the
list in reverse for the last exact equal-length, case-sensitive byte match.
It removes only one match, shifts the suffix left and retains allocation
capacity. Normal re-add therefore moves that exact path to the tail, while
case variants remain distinct. `ResourceDirectoryList` implements those normal
derived methods using native strings/vectors. The original four add bodies
and four remove bodies compare instruction-for-instruction after normalizing
exception-handler constants and relocated internal branch addresses.

Graphics paths use the actual original +110 method, with flags ZERO for every
root and descendant; client recursion supplies the descendants. GetIGraph is
resolved afresh per registration. The stdcall method takes explicit stack self,
path and recursive flag. Its RVA is 6FF80, delegating to 1006FD60. That graphics
implementation appends a separator and a new entry without the client lists'
remove-before-append rule. Keeping it delegated preserves this distinction and
does not accidentally enumerate twice through a recursive graphics flag.

## Native differences and remaining integration

The source implements normal startup's derived dispatch, not arbitrary subclass
overrides or original virtual/string/allocator ABI. It does not initialize or
close the surrounding loaded-resource caches. Graphics and its module remain
borrowed and must outlive registration. Empty client paths fail explicitly;
the original reads the byte before an empty string. Oversize copied/search
paths throw instead of overflowing the original 1004/512-byte buffers. These
guards may leave a registered root before a subsequent enumeration error, as
registration already occurred. Native exceptions close active searches; the
original code has no equivalent native C++ exception guarantee. Win32 adapter
also closes a newly opened handle if copying the first result throws.

No desktop test is required for this source-only slice. Full application startup
still needs protection-state behavior, complete resource/cache services and
movie/menu state owners, plus concrete graphics input services and remaining
application prerequisites. Root registration does not imply the cache consumers
have been implemented or wired to the live host.

## Validation and accounting

Temporary ASan/UBSan audit checks 10,000 add/remove operations against an
independent list model, all 255 non-NUL lowercase bytes, C-string termination,
separator/case distinctions, eight startup roots, five separate client owners,
root registration when missing, depth-first ordering, exact dot exclusion,
dot-prefixed/reparse inclusion, unknown categories, nested handle lifetime and
exception/oversize cleanup. Strict i386 fixture instantiates the native lists,
full root/recursive setup, Win32 search adapter and actual graphics forwarding
and links successfully. That Windows executable is NOT run.

Existing native checks, all 158 Python tooling tests and unchanged host build
pass. Temporary audit files are deleted after verification. The
[evidence map](resource-directories-map.json), checked by
`python3 -B tools/validate_engine_host.py --map analysis/resource-directories-map.json`,
verifies both fingerprints, 39 instruction/data anchors, 40 client vtable slots,
one engine slot and 112 direct calls. Exact root strings, wildcard and separator
are byte anchors rather than PUSH resources. A temporary negative audit rejects
seven altered hashes/anchors/slots/calls. Read-only exports remain ignored under
`resource-search-directories`, `resource-directory-caches` and
`resource-directory-removal` in analysis/work.

Twelve new partial client entries: 161 total (156 client/five engine), zero
complete, frozen denominator 18,783. No engine credit for delegated directory
storage, no extra constructor/vector-helper credit, and no claim that the
protection-bearing startup wrapper is complete.
