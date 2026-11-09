# Discovery provider research — 2026-11-09

This is an initial architecture comparison, not N2 implementation or live-service
acceptance. No hosts file, installation, registry or external service was changed.

| Candidate | Primary evidence | Compatibility decision |
| --- | --- | --- |
| RpR H&D2 master | [RpR Play Online](https://www.rprclan.com/hd2/play-online) documents redirects of hd2.available.gamespy.com, hd2.master.gamespy.com and hd2.ms14.gamespy.com to 78.47.255.224, identifying availability, heartbeats and list roles | First interoperability candidate; recover actual original messages and configure endpoints separately; no availability claim |
| OpenSpy | [OpenSpy Core](https://github.com/openspy/openspy-core) publishes a GameSpy replacement implementation and architecture | Compare exact original SDK list/registration variants and game registration support before selecting it; generic replacement availability is not H&D2 compatibility |
| Project master | Would require ownership of list, registration/expiry and query contracts established from both binaries | No demonstrated necessity yet; do not implement before evaluating existing compatible services |
| Steamworks | [Valve game-server documentation](https://partner.steamgames.com/doc/features/multiplayer/game_servers) describes its unified browser and ISteamMatchmakingServers | Optional future provider; no evidence original binaries use it; does not replace the H&D2 session protocol |

RpR also documents manual IP/port and saved ServerList.bin as independent ways
to find a server. The reconstructed client's original GameSpy path is absent;
redirecting DNS alone cannot implement that missing source. LAN EnumHosts and
direct connection remain separate contracts, regardless of Internet provider.

Do not infer live behavior from these webpages. Next N2 work must trace original
availability checks, GameSpy server registration/heartbeats, list query framing,
response conversion and update pumping, with endpoint configuration outside the
core session transport. Preserve original GUID/query/port evidence.
