"""Compare the recovered last-dot rule with Python and optional real mission references."""

import argparse
from pathlib import Path
import random
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from inspect_scene2 import decode_objects, model_resource_key


def compare(normalizer: Path, name: str) -> None:
    result = subprocess.run([str(normalizer), name], capture_output=True, text=True)
    if result.returncode != 0:
        raise ValueError(f"C++ model-key tool failed: {result.stderr}")
    if result.stdout != model_resource_key(name) + "\n":
        raise ValueError("C++/Python model resource key mismatch")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--normalizer", type=Path, required=True)
    parser.add_argument("--scene", type=Path)
    args = parser.parse_args()
    normalizer = args.normalizer.resolve(strict=True)
    names = ["", ".hidden", "folder.name/object", "folder\\thing.4ds", "thing..4ds", ".", ".."]
    generator = random.Random(0x2012)
    for _ in range(64):
        names.append("".join(generator.choices("abCD09./\\ _-", k=generator.randrange(48))))
    for name in names:
        compare(normalizer, name)
    print(f"Matched {len(names)} synthetic model resource keys.")
    if args.scene is not None:
        records = decode_objects(args.scene.read_bytes())["records"]
        resources = [
            field for record in records for field in record["fields"]
            if field["id"] == "0x2012"
        ]
        unique = {field["value"] for field in resources}
        for name in sorted(unique):
            compare(normalizer, name)
        print(f"Matched {len(unique)} unique keys used by {len(resources)} real mission resource fields.")


if __name__ == "__main__":
    main()
