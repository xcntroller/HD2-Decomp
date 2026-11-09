"""Refresh or check networking/README.md from the validated server/launcher ledger."""
import argparse
import json
from pathlib import Path

from validate_status import DIRECTORY, ROOT, validate

START = "<!-- networking-progress:start -->"
END = "<!-- networking-progress:end -->"
README = ROOT / "networking/README.md"


def block():
    counts = validate()
    ledger = json.loads((DIRECTORY / "function-status.json").read_text())
    lines = [START, "", "| Executable | Inventoried functions | Complete | Partial | No completion claim | Complete % |",
             "| --- | ---: | ---: | ---: | ---: | ---: |"]
    for role, title in (("server", "HD2DS.exe"), ("launcher", "ServerLauncher.exe")):
        total = ledger["programs"][role]["internal_non_thunk"]
        complete, partial = counts[role]["complete"], counts[role]["partial"]
        remaining = total - complete - partial
        lines.append(f"| {title} | {total:,} | {complete:,} | {partial:,} | {remaining:,} | {100 * complete / total:.2f}% |")
    for role, title in (("server", "HD2DS.exe"), ("launcher", "ServerLauncher.exe")):
        lines += ["", f"### {title} mapped bodies", "",
                  "| Original address | Status | Implemented contract |",
                  "| --- | --- | --- |"]
        for row in sorted((r for r in ledger["functions"] if r["program"] == role), key=lambda r: r["entry"]):
            source = row["sources"][-1]
            contract = row["contract"].replace("|", "\\|").replace("\n", " ")
            lines.append(f"| `{row['entry'].upper()}` | {row['status']} | [{contract}](../{source}) |")
    lines += ["", END]
    return "\n".join(lines)


def update(check=False):
    text = README.read_text()
    if text.count(START) != 1 or text.count(END) != 1:
        raise ValueError("README must contain exactly one networking progress block")
    begin, end = text.index(START), text.index(END) + len(END)
    if begin >= end - len(END):
        raise ValueError("Networking progress markers are reversed")
    expected = text[:begin] + block() + text[end:]
    if check:
        if expected != text:
            raise SystemExit("Networking README progress is stale; run tools/networking/update_progress.py")
    elif expected != text:
        README.write_text(expected)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    update(args.check)
    print("Networking README progress is current")
