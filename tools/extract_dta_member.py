"""Extract one ISD0 archive member using raw or LZSS/RLE blocks; no game DLLs."""

import argparse
from dataclasses import dataclass
from pathlib import Path
import struct
from typing import BinaryIO


@dataclass(frozen=True)
class Member:
    name: str
    data_offset: int
    size: int
    blocks: int
    encrypted: bool


def read_exact(stream: BinaryIO, size: int) -> bytes:
    data = stream.read(size)
    if len(data) != size:
        raise ValueError(f"Truncated archive: expected {size} bytes, got {len(data)}")
    return data


def xor_record(data: bytes, key: bytes) -> bytes:
    if len(key) != 8:
        raise ValueError("ISD0 XOR keys must contain exactly 8 bytes")
    return bytes(value ^ key[index % 8] for index, value in enumerate(data))


def read_index(stream: BinaryIO, key: bytes) -> list[Member]:
    stream.seek(0, 2)
    archive_size = stream.tell()
    stream.seek(0)
    if read_exact(stream, 4) != b"ISD0":
        raise ValueError("Only ISD0 archives are supported")
    count, offset, table_size, _ = struct.unpack(
        "<4I", xor_record(read_exact(stream, 16), key)
    )
    if count > 100_000 or table_size != count * 28 or offset < 20:
        raise ValueError("Invalid archive index or incorrect keys")
    if offset + table_size > archive_size:
        raise ValueError("Archive index extends beyond the file")
    stream.seek(offset)
    table = xor_record(read_exact(stream, table_size), key)
    members = []
    seen = set()
    for index in range(count):
        _, header_offset, data_offset, _ = struct.unpack_from("<III16s", table, index * 28)
        if not 20 <= header_offset <= archive_size - 32:
            raise ValueError(f"Invalid member header offset at index {index}")
        stream.seek(header_offset)
        header = xor_record(read_exact(stream, 32), key)
        name_length = header[24]
        if not name_length or data_offset < header_offset + 32 + name_length:
            raise ValueError(f"Invalid member layout at index {index}")
        if data_offset > archive_size:
            raise ValueError(f"Member data extends beyond archive at index {index}")
        name = xor_record(read_exact(stream, name_length), key).decode("ascii")
        normalized = name.replace("\\", "/").casefold()
        if "\0" in name or normalized in seen:
            raise ValueError(f"Invalid or duplicate archive member: {name!r}")
        seen.add(normalized)
        size, blocks = struct.unpack_from("<II", header, 16)
        if size > 256 * 1024 * 1024 or blocks > 1_000_000:
            raise ValueError(f"Member exceeds research-tool limits: {name}")
        members.append(Member(name, data_offset, size, blocks, bool(header[25] & 0x80)))
    return members


def decompress_lzss(data: bytes, limit: int) -> bytes:
    output = bytearray()
    position = 0
    while position < len(data):
        if position + 2 > len(data):
            raise ValueError("Truncated LZSS flag word")
        flags = int.from_bytes(data[position:position + 2], "big")
        position += 2
        for bit in range(15, -1, -1):
            if position == len(data):
                break
            if flags & (1 << bit):
                if position + 2 > len(data):
                    raise ValueError("Truncated LZSS match")
                token = int.from_bytes(data[position:position + 2], "big")
                position += 2
                distance, length = token >> 4, token & 15
                if distance == 0:
                    if position + 2 > len(data):
                        raise ValueError("Truncated RLE token")
                    length = (length << 8) + data[position] + 16
                    value = data[position + 1]
                    position += 2
                    if len(output) + length > limit:
                        raise ValueError("RLE output exceeds expected block size")
                    output.extend(bytes([value]) * length)
                else:
                    length += 3
                    if distance > len(output):
                        raise ValueError("LZSS reference precedes this block's output")
                    if len(output) + length > limit:
                        raise ValueError("LZSS output exceeds expected block size")
                    for _ in range(length):
                        output.append(output[-distance])
            else:
                if len(output) == limit:
                    raise ValueError("Literal exceeds expected block size")
                output.append(data[position])
                position += 1
    return bytes(output)


def extract_member(stream: BinaryIO, member: Member, key: bytes) -> bytes:
    stream.seek(member.data_offset)
    output = bytearray()
    previous_size = 0
    for _ in range(member.blocks):
        stored_size, previous_field = struct.unpack("<HH", read_exact(stream, 4))
        if stored_size == 0 or previous_field != previous_size:
            raise ValueError("Invalid block size or unsupported previous-block field")
        previous_size = stored_size
        block = read_exact(stream, stored_size)
        if member.encrypted:
            block = xor_record(block, key)
        limit = min(32768, member.size - len(output))
        if limit < 0:
            raise ValueError("Member exceeds declared size")
        if block[0] == 0:
            decoded = block[1:]
        elif block[0] == 1:
            decoded = decompress_lzss(block[1:], limit)
        else:
            raise ValueError(f"Unsupported block type: {block[0]}")
        if len(decoded) != limit:
            raise ValueError(f"Decoded block size {len(decoded)} does not match {limit}")
        output.extend(decoded)
    if len(output) != member.size:
        raise ValueError(f"Decoded member size {len(output)} does not match {member.size}")
    return bytes(output)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("member", help="Exact archive path, matched case-insensitively")
    parser.add_argument("destination", type=Path)
    parser.add_argument("--key1", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--key2", type=lambda value: int(value, 0), required=True)
    args = parser.parse_args()
    if not all(0 <= key <= 0xFFFFFFFF for key in (args.key1, args.key2)):
        parser.error("Keys must be unsigned 32-bit integers")
    if args.destination.exists():
        parser.error("Destination already exists; choose a new output file")
    key = struct.pack("<II", args.key2, args.key1)
    with args.archive.open("rb") as stream:
        members = read_index(stream, key)
        target = args.member.replace("\\", "/").casefold()
        matches = [
            member for member in members
            if member.name.replace("\\", "/").casefold() == target
        ]
        if len(matches) != 1:
            parser.error(f"Expected one matching member, found {len(matches)}")
        data = extract_member(stream, matches[0], key)
    with args.destination.open("xb") as output:
        output.write(data)
    print(f"Extracted {matches[0].name}: {len(data)} bytes -> {args.destination}")


if __name__ == "__main__":
    main()
