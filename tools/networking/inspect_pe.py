"""Static PE metadata only; never loads or executes a Windows image.

Requires pefile, already used in the local research environment.
"""
import argparse
import hashlib
import json
from pathlib import Path

import pefile


def inspect(path: Path) -> dict:
    data = path.read_bytes()
    pe = pefile.PE(data=data)
    versions = []
    for fixed in getattr(pe, "VS_FIXEDFILEINFO", []):
        versions.append({
            "file": [fixed.FileVersionMS >> 16, fixed.FileVersionMS & 65535,
                     fixed.FileVersionLS >> 16, fixed.FileVersionLS & 65535],
            "product": [fixed.ProductVersionMS >> 16, fixed.ProductVersionMS & 65535,
                        fixed.ProductVersionLS >> 16, fixed.ProductVersionLS & 65535],
        })
    return {
        "name": path.name, "sha256": hashlib.sha256(data).hexdigest(), "size": len(data),
        "machine": hex(pe.FILE_HEADER.Machine), "optional_header_magic": hex(pe.OPTIONAL_HEADER.Magic),
        "image_base": hex(pe.OPTIONAL_HEADER.ImageBase),
        "entry": hex(pe.OPTIONAL_HEADER.ImageBase + pe.OPTIONAL_HEADER.AddressOfEntryPoint),
        "subsystem": pe.OPTIONAL_HEADER.Subsystem, "coff_timestamp": pe.FILE_HEADER.TimeDateStamp,
        "version_resources": versions,
        "imports": [{"dll": d.dll.decode("ascii"), "symbols": [
            {"name": i.name.decode("ascii") if i.name else None, "ordinal": i.ordinal,
             "iat": hex(i.address)} for i in d.imports]} for d in pe.DIRECTORY_ENTRY_IMPORT],
        "sections": [{"name": s.Name.rstrip(b"\0").decode("ascii"),
                      "rva": hex(s.VirtualAddress), "raw_size": s.SizeOfRawData,
                      "virtual_size": s.Misc_VirtualSize} for s in pe.sections],
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("images", nargs="+", type=Path)
    args = parser.parse_args()
    print(json.dumps({"schema_version": 1, "method": "static pefile read", "images": [
        inspect(p) for p in args.images]}, indent=2))
