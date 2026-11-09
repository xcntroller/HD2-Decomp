# Modding-tools handoff — 2026-11-09

First milestone is implemented as a source-only standalone native CLI. Build and
usage, original addresses, JSON contract and limits: [README](README.md).
Branch/worktree already existed clean at 89d3d2f, equal to latest local master;
remote-tracking master was older. No remote fetch/push/merge was performed.
Main HANDOFF.md, plan.md, function ledger and all recovered runtime files remain
untouched. Work is entirely under tools/modding.

Verification: warning-free C++17 build; catalog cardinalities 275/33; 18 synthetic
inputs cover declarations, dynamic/literal references, If, signal events,
Whenever, comments, strings, empty input, unknown commands, duplicate declarations,
unresolved labels, unclosed braces, unsupported Bool, arity, token/source bounds,
NUL and includes. Same temporary Python harness passed native ASan/UBSan/LSan
outside sandbox (sandbox ptrace prevented LeakSanitizer). System libasan linker
path was broken; used existing verified sanitizer libraries from master worktree.
Temporary harness, catalogs and binaries removed after verification. No original
executable/DLL/Proton, game build, desktop test or runtime compatibility claim.

Next independent work is in PLAN.md: source spans/structured diagnostics, numeric
event records and evidence-tagged references. Include resolution is intentionally
unsupported until caller policy and faithful original preprocessing are explicit.
