# First frame: preserve the original input allocation — 2026-11-17

Base e341c06, feature/p22-frame. The user's cb8b636c run reached actual
loaded1/status5 activation, SAME-World FRAME binding, then completed Scene78
render and fixed-update calls. Pressing Space exposed a handled input error:
`Game input requires the actual two mutable64-word controller views`. The process
retired owners and exited result1; this run has no access-violation trace. Its
startup log reports63 binding records and63/63 logical output words. Exact binary
and logs are retained in WORK/loading-first-frame-input-return. That evidence
does not establish accepted scene presentation or stable controllable gameplay.
[Receipt](checkpoints/p22-frame-input-allocation-20261117.json).

Original006C7480 repeatedly appends binding records and then zero output words
until each logical size covers the installed index.0045CCC0 end insertion grows
allocation by size+max(size,insertion count), copying only the logical prefix.
The original mode2 defaults install indexes0..62, producing63 logical words in
an allocation of64 words.006C72B0 reports63 and returns the SAME begin pointer;
006C31D0 forwards those pointers when both counts are nonnegative. Original
0049C1D0 and0053BF90 can write all64 words. Native growth leaves the allocated
spare word's contents unwritten until an actual producer/consumer touches it.
The existing raw input::client::Vector/installer already preserved that growth;
the portable binding manager's bulk std::vector resize did not preserve the
allocated-object extent that gameplay requires. Simply accepting std::vector
capacity would expose words outside its live element range.

BindingOutputWords now owns an actual Word array with distinct logical size and
allocated extent, using native repeated end-insertion growth. Only appended
logical words are initialized. Spare bytes preserve their real allocation
contents; no neutral word, fake binding63, padded input copy or new polling pass
is introduced. Clear retains storage/bytes, and destruction releases second,
first, binding and joystick allocations in the existing order. Allocation failure
leaves the last published prefix owned, with no successful missing allocation.

The controller still checks exact pointer provenance and requires64 live owned
word objects. Only this explicit output owner exposes its allocated extent;
ordinary std::vector buffers must still have64 logical elements. Query results
remain logical counts and empty queries still leave the output untouched. The
manager-selected BindingInputViews likewise uses the real allocated extent;
explicit caller buffers retain their supplied bounds. Thus Human74/78 loans,
HUD64-word consumers, binding producer and controller all refer to the SAME two
owned allocations. A later genuine mode3 installation creates binding63 and
initializes that logical word in place. The complete raw006C7480 ABI body and
its source/file fingerprints are unchanged and receive no new completion credit.

Targeted native ASan/UBSan/leak checks compare growth/capacity/logical words with
the existing raw vector source, apply both original profile layouts through the
actual installer/InputManager, poll actual producer policy with explicit device
endpoints, query/validate the actual controller views, mutate through actual
HumanFrameInputBuffer and BindingInputViews, and verify spare retention, clear,
mode3, allocation failure and foreign/short-buffer rejection. The temporary audit
uses deterministic allocator endpoints to observe untouched bytes; production
has no such filling hook.34,209 checks passed. Strict i386 compile checks include
the unchanged original manager offsets and installer plus the new controller
composition. Full actual application/probe i386 O2/Werror compile/link passed;
revised stage-only hashes are in the receipt. Temporary fixtures were fingerprinted
and removed. No Windows/original/DLL/Proton code was executed by the agent.

Next is the user's revised Space-to-continue test, then the first actual frame
failure. OWN dirty source/artifact and the isolated uncommitted interactive
overlay remain preserved. No input/update/stable scene/gameplay acceptance,
FRAME/HUM/OWN promotion, function-count change or original allocator/SEH/ABI
identity is claimed. Positive decoration bending/wind boundaries remain explicit
as recorded in [the loading contract](p22-frame-loading-decoration-source.md).

WORK=analysis/work/p22-frame-closure-20261117. Research index preceded the new
0045CCC0 read-only export; existing006C7480/006C72B0/006C31D0 C/ABI/source contracts
were reused. The receipt fingerprints both original evidence and exact sources.
