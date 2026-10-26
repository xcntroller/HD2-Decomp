"""Compare every preserved record field and boundary against Python, without printing asset names."""

import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from inspect_model_table import summarize_model_table
from test_model_table import LAYOUT_BITS, model, record


def compare(scanner: Path, path: Path) -> None:
    expected = summarize_model_table(path.read_bytes(), include_records=True)
    result = subprocess.run([str(scanner), str(path), "--records"], capture_output=True, text=True)
    if result.returncode != 0:
        raise ValueError(f"C++ first-table scan failed: {result.stderr}")
    if json.loads(result.stdout) != expected:
        raise ValueError("C++/Python first-table field or boundary mismatch")
    aggregate = subprocess.run([str(scanner), str(path)], capture_output=True, text=True)
    if aggregate.returncode != 0:
        raise ValueError(f"C++ aggregate scan failed: {aggregate.stderr}")
    del expected["record_data"]
    if json.loads(aggregate.stdout) != expected:
        raise ValueError("C++/Python aggregate first-table mismatch")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scanner", type=Path, required=True)
    parser.add_argument("--model", type=Path, action="append", default=[])
    args = parser.parse_args()
    scanner = args.scanner.resolve(strict=True)
    records = []
    for mask in range(64):
        flags = 0x00800001 | sum(bit for index, bit in enumerate(LAYOUT_BITS) if mask & (1 << index))
        for primary in [b"", b"\0\xffprimary"]:
            records.append(record(flags, primary, b"extra", b"secondary"))
    valid = [model([]), model(records), model([record(0x44080000, bytes(255), bytes(255), bytes(255))]),
             model([record(0, b"")] * 6000)]
    minimal = model([record(0, b"")])
    maximal = valid[2]
    cuts = sorted(set(range(len(minimal))) | {
        16 + size for size in [4, 79, 80, 81, 335, 336, 337, 591, 592, 593, 847, 848, 849, 865]
        if 16 + size < len(maximal)
    })
    invalid = [minimal[:size] for size in range(len(minimal))]
    invalid += [maximal[:size] for size in cuts if len(minimal) <= size < len(maximal)]
    invalid += [model([record(0)])[:14] + b"\x71\x17"]
    with tempfile.TemporaryDirectory(prefix="hd2-model-table-") as directory:
        path = Path(directory) / "synthetic.4ds"
        for data in valid:
            path.write_bytes(data + b"unread")
            compare(scanner, path)
        for data in invalid:
            path.write_bytes(data)
            try:
                summarize_model_table(data)
            except ValueError:
                pass
            else:
                raise ValueError("Python accepted a malformed first-table fixture")
            result = subprocess.run([str(scanner), str(path)], capture_output=True, text=True)
            if result.returncode != 1 or result.stdout or not result.stderr.strip():
                raise ValueError("C++ must reject malformed first-table input without success JSON")
    print(f"Matched {len(valid)} synthetic tables (including 128 flag/name combinations and 6000 records); "
          f"both readers rejected {len(invalid)} malformed tables.")
    for path in args.model:
        compare(scanner, path)
        summary = summarize_model_table(path.read_bytes())
        print(f"Matched {summary['records']} real first-table records ending at byte {summary['table_end']}.")


if __name__ == "__main__":
    main()
