# Existing client networking boundary — 2026-11-09

Review scope: [multiplayer pipeline](../multiplayer-pipeline.md), including its
final desktop acceptance (earlier pending statements are historical),
[function audit](../multiplayer-function-audit.md), shared menu contracts/route
references, `recovered/frontend/multiplayer_{server,settings,maps,discovery,connection}.h`,
`recovered/host/multiplayer_{menu,controls,connection_menu,discovery}.h` and the
actual `multiplayer_discovery.cpp` adapter. Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
No frozen client ledger change is authorized by this server inventory.

| Boundary | Existing implementation/evidence | Remaining |
| --- | --- | --- |
| Browser/Create/filter/Add Server | Reachable frontend accepted 2026-11-01, with later selection fixes accepted 2026-11-03; source owns server rows/settings/map rotation and copied launch requests | Every ready-server UI behavior remains untested |
| Internet | UI explicitly reports unavailable; original 00723250 → 0073A7A0/0073AB30 GameSpy list service, pump 00723240 | No production Internet list adapter; failure is known implementation absence, not measured network failure |
| LAN | Real DirectPlay8Client COM adapter, original 00723A30/00723330/00723AB0/00722180 | No approved Windows/Proton endpoint test; do not infer OS failure or replace DirectPlay |
| Add Server | Numeric IPv4/port validation, initialized manual row, persistence (00637440); default UI 11001 | Unknown/unresponsive rows deliberately cannot be selected for Join; query and transport independence need verification |
| Restore/info | Targeted DirectPlay EnumHosts, query DWORD 2/1; owned per-request callback identities | Real responses and populated information/password screens untested |
| Connect | Request/timer/rejection/password phase model (0068AED0); ordinary and standalone owners distinct | Production join/host return unavailable; original 00723D20 and 00722790 not reconstructed |
| Dedicated launch | Original 007226B0 writes XDServer_%5d.txt via 006ACF50, then CreateProcessA hd2ds.exe -cmd -exec filename | Production UI reports unavailable; never launch original binary |
| Game receiver | Controller status 7/8 separate connection owner; status 9 → 00730A40/0072E890/00731610 → 00698510 | Main P2.2 workstream owns actual world integration; do not modify it here |

DirectPlay application GUID `{2ae5b082-cfa0-11d4-9e1d-0050fc213de6}`,
host/device TCP/IP addresses, LAN port 31567 and four-byte query values 0/1/2
are established. LAN duplicate suppression is by IP. The native adapter copies
callback data and closes/joins before discarding request contexts. Existing
response decoder supports legacy/extended bit records and kind-one information;
retain it as the protocol oracle instead of recoding from recollection.

Important distinction: manual Internet targets retain supplied ports in original
00723D20, while its LAN branch overwrites the connection port with 31567. The
auxiliary UDP owner is then assigned selected port + 1. This is static client
evidence and needs server-side tracing before changing any port policy.

Original 007260A0 refuses enumeration queries until a real 00698500 game owner
exists with progress +28 >= 100. Fake ready-game substitution would contradict
existing evidence. Session interoperability may therefore require a bounded
world/session dependency even though simulation is outside this workstream.

No original executable, DLL, Proton or Windows runtime fixture was executed for
this review. Existing native tests establish copied-data/format/COM-call contracts,
not live discovery, OS compatibility, admission or session interoperability.
