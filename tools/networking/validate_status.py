"""Validate the independent networking inventories, ledger and optional local evidence."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DIRECTORY = ROOT / "analysis/networking"
# Headless server reuse requires separate HD2DS-address contract verification.
# Keep shared sources explicit; the client ledger stays frozen.
SHARED_SOURCES = {ROOT / ("recovered/frontend/" + name) for name in (
    "human_store.h", "game_data.h", "profile_directories.h", "profile_settings.h",
    "profile_payload.h", "profile_load.h")}


def validate(local_evidence=False):
    ledger = json.loads((DIRECTORY / "function-status.json").read_text())
    inventories = {}
    for role, record in ledger["programs"].items():
        path = ROOT / record["path"]
        assert path.parent == DIRECTORY and path.name == f"{role}-inventory.json"
        data = path.read_bytes()
        assert hashlib.sha256(data).hexdigest() == record["sha256"], path
        inventory = json.loads(data)
        assert inventory["sha256"] == record["binary_sha256"]
        rows = inventory["functions"]
        entries = {row["entry"]: row for row in rows}
        assert len(entries) == len(rows) == record["total"]
        assert sum(not r["external"] and not r["thunk"] for r in rows) == record["internal_non_thunk"]
        inventories[role] = entries
    seen = set()
    counts = {role: {"partial": 0, "complete": 0} for role in inventories}
    for row in ledger["functions"]:
        role, entry = row["program"], row["entry"]
        assert (role, entry) not in seen
        seen.add((role, entry))
        original = inventories[role][entry]
        assert not original["external"] and not original["thunk"]
        assert row["status"] in ("partial", "complete")
        assert row["contract"] and row["exclusions"] and row["evidence"]
        for source in row["sources"]:
            path = (ROOT / source).resolve()
            assert (path.is_relative_to(ROOT / "recovered/networking") or
                    (role == "server" and path in SHARED_SOURCES)) and path.is_file()
        counts[role][row["status"]] += 1
    if local_evidence:
        receipt = json.loads((DIRECTORY / "evidence.json").read_text())
        for row in receipt["exports"]:
            path = (ROOT / row["metadata"]).resolve()
            assert path.is_relative_to(ROOT / "analysis/work/networking")
            assert hashlib.sha256(path.read_bytes()).hexdigest() == row["metadata_sha256"]
            assert hashlib.sha256(path.with_suffix(".c").read_bytes()).hexdigest() == row["decompilation_sha256"]
            metadata = json.loads(path.read_text())
            assert metadata["decompiled"] and metadata["entry"] == row["entry"]
            assert metadata["sha256"] == row["binary_sha256"]
    return counts


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--local-evidence", action="store_true")
    args = parser.parse_args()
    print(json.dumps(validate(args.local_evidence), indent=2))
