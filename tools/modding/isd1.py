"""Bounded rw_data ISD1 checksums, metadata and raw/LZSS payload reconstruction.

Evidence: 10003763 (archive), 1000C110 (member/table), 1000513B (read),
10009E97 (checksum), 10009BF3/10009D24 (XOR), 1000EA20 (shared LZSS).
No original executable code is loaded or called. Sound tags >=8 are unsupported.
"""
from __future__ import annotations
from dataclasses import dataclass
import struct
import sys
from pathlib import Path

_tools=str(Path(__file__).resolve().parent.parent)
if _tools not in sys.path:sys.path.append(_tools)
from extract_dta_member import xor_record, decompress_lzss

MAX_MEMBERS=100_000
MAX_NAME=4096
MAX_PAYLOAD=64*1024*1024
MAX_BLOCKS=262_144
MAX_NAME_BYTES=16*1024*1024
SUPPORTED_TAGS=frozenset(range(8))


class ArchiveError(ValueError):
    kind="invalid"


class ArchiveIntegrityError(ArchiveError):
    kind="integrity_error"


class ArchiveBoundsError(ArchiveError):
    kind="bounds_error"


class ArchiveLimitError(ArchiveError):
    kind="inspection_limit"


class UnsupportedCompression(ArchiveError):
    kind="unsupported_compression"

    def __init__(self,tags):
        self.tags=tuple(sorted(set(tags)))
        super().__init__(f"Unsupported ISD1 compression tags {self.tags}; stored payload checksums verified; no fallback")


@dataclass(frozen=True)
class Block:
    stored_size: int
    checksum: int
    codec: int


@dataclass(frozen=True)
class Member:
    name: str
    size: int
    header_offset: int
    data_offset: int
    payload_end: int
    encrypted: bool
    blocks: tuple[Block,...]

    @property
    def codecs(self):return tuple(sorted({b.codec for b in self.blocks}))

    @property
    def supported(self):return all(b.codec in SUPPORTED_TAGS for b in self.blocks)


def checksum(data,seed=0xff):
    """10009E97: wrapping WORD add byte, then xor its 8-bit complement."""
    state=seed&0xffff
    for byte in data:state=((state+byte)&0xffff)^(byte^0xff)
    return state


def exact(stream,count,limit):
    position=stream.tell()
    if count<0 or not 0<=position<=limit or count>limit-position:
        raise ArchiveBoundsError(f"ISD1 read exceeds bounds at 0x{position:x}")
    data=stream.read(count)
    if len(data)!=count:raise ArchiveBoundsError(f"Truncated ISD1 read at 0x{position:x}")
    return data


def require_checksum(data,expected,context,seed=0xff):
    actual=checksum(data,seed)
    if actual!=expected:
        raise ArchiveIntegrityError(f"ISD1 {context} checksum mismatch: expected {expected:04x}, got {actual:04x}")
    return actual


def read_index(stream,key):
    if len(key)!=8:raise ValueError("ISD1 key requires eight bytes")
    stream.seek(0,2);length=stream.tell();stream.seek(0)
    magic=exact(stream,4,length)
    if magic!=b"ISD1":raise ArchiveBoundsError("Expected ISD1 magic")
    header=xor_record(exact(stream,20,length),key)
    count,offset,size,index_crc,header_crc=struct.unpack("<5I",header)
    require_checksum(magic+header[:16]+b"\0"*4,header_crc,"archive header")
    if count>MAX_MEMBERS:raise ArchiveLimitError("ISD1 member index exceeds inspection limit")
    if size!=count*28 or offset<24 or offset+size!=length:
        raise ArchiveBoundsError("Invalid ISD1 index count/extent")
    stream.seek(offset);stored_index=exact(stream,size,length)
    require_checksum(stored_index,index_crc,"stored index")
    table=xor_record(stored_index,key)
    members=[];seen=set();total_blocks=0;total_names=0
    for index in range(count):
        name_hash,header_offset,data_offset,_=struct.unpack_from("<III16s",table,index*28)
        if not 24<=header_offset<=offset-32:raise ArchiveBoundsError("Invalid ISD1 member header offset")
        stream.seek(header_offset);record=xor_record(exact(stream,32,offset),key)
        record_crc=struct.unpack_from("<I",record,24)[0]
        require_checksum(record[:24]+b"\0"*4+record[28:],record_crc,"member header")
        size,block_count=struct.unpack_from("<II",record,16)
        flags,metadata_crc=struct.unpack_from("<HH",record,28)
        name_length=flags&0x7fff
        if not name_length:raise ArchiveBoundsError("Empty ISD1 member name")
        total_names+=name_length
        total_blocks+=block_count
        if name_length>MAX_NAME or total_names>MAX_NAME_BYTES or total_blocks>MAX_BLOCKS:
            raise ArchiveLimitError("ISD1 metadata exceeds inspection limits")
        # The original indexes 32 KiB decoded blocks, including one zero-length
        # raw block for the empty members present in Patch.dta.
        if block_count!=max(1,(size+32767)//32768):
            raise ArchiveBoundsError("ISD1 block count inconsistent with decoded size")
        raw_name=xor_record(exact(stream,name_length,offset),key)
        if b"\0" in raw_name:raise ArchiveBoundsError("Embedded ISD1 name terminator")
        try:name=raw_name.decode("ascii")
        except UnicodeDecodeError as exc:raise ArchiveBoundsError("Non-ASCII ISD1 name unsupported") from exc
        folded=name.upper().encode("ascii")
        if name_hash!=((len(folded)<<16)|(sum(folded)&0xffff)):
            raise ArchiveIntegrityError("ISD1 index/name hash mismatch")
        normalized=name.replace("\\","/").lower()
        if normalized in seen:raise ArchiveBoundsError("Duplicate ISD1 member path")
        seen.add(normalized)
        block_table=exact(stream,block_count*4,offset)
        stored_types=exact(stream,block_count,offset)
        # Name is plaintext; lengths/checksums are stored directly; type tags are
        # still ciphertext when this chained metadata checksum is computed.
        state=checksum(block_table,checksum(raw_name))
        require_checksum(stored_types,metadata_crc,"name/block-table/type metadata",state)
        tags=xor_record(stored_types,key)
        blocks=tuple(Block(*struct.unpack_from("<HH",block_table,i*4),tags[i]) for i in range(block_count))
        if data_offset!=stream.tell():raise ArchiveBoundsError("ISD1 data offset does not follow member tables")
        end=data_offset+sum(b.stored_size for b in blocks)
        if end>offset:raise ArchiveBoundsError("ISD1 block span crosses archive index")
        if size and any(not b.stored_size for b in blocks):raise ArchiveBoundsError("Empty ISD1 block in nonempty member")
        members.append(Member(name,size,header_offset,data_offset,end,bool(flags&0x8000),blocks))
    # Reject cross-member reads before any payload allocation; padding is fine.
    physical=sorted(members,key=lambda m:m.header_offset)
    for left,right in zip(physical,physical[1:]):
        if left.payload_end>right.header_offset:raise ArchiveBoundsError("Overlapping ISD1 member spans")
    return members


def read_member(stream,member,key,max_payload=MAX_PAYLOAD):
    if len(key)!=8:raise ValueError("ISD1 key requires eight bytes")
    if member.size<0:raise ArchiveBoundsError("Negative ISD1 decoded size")
    if member.size>max_payload:raise ArchiveLimitError("ISD1 decoded size exceeds inspection limit")
    if len(member.blocks)!=max(1,(member.size+32767)//32768):raise ArchiveBoundsError("ISD1 block count mismatch")
    if member.data_offset<24 or member.payload_end!=member.data_offset+sum(b.stored_size for b in member.blocks):
        raise ArchiveBoundsError("Invalid ISD1 payload extent")
    stream.seek(0,2)
    if member.payload_end>stream.tell():raise ArchiveBoundsError("Truncated ISD1 payload extent")
    stream.seek(member.data_offset)
    unsupported={b.codec for b in member.blocks if b.codec not in SUPPORTED_TAGS}
    output=bytearray()
    for index,block in enumerate(member.blocks):
        if not 0<=block.stored_size<=0xffff or not 0<=block.checksum<=0xffff or not 0<=block.codec<=255:
            raise ArchiveBoundsError("Invalid ISD1 block descriptor")
        stored=exact(stream,block.stored_size,member.payload_end)
        require_checksum(stored,block.checksum,f"stored block {index}")
        # Integrity checking remains useful even when sound tags are unsupported.
        if unsupported:continue
        payload=xor_record(stored,key) if member.encrypted else stored
        limit=min(32768,member.size-len(output))
        try:
            decoded=decompress_lzss(payload,limit) if block.codec==1 else payload
        except ValueError as exc:raise ArchiveBoundsError(f"ISD1 LZSS block {index}: {exc}") from exc
        if len(decoded)!=limit:raise ArchiveBoundsError(f"ISD1 decoded block {index} length mismatch")
        output.extend(decoded)
    if unsupported:raise UnsupportedCompression(unsupported)
    if len(output)!=member.size:raise ArchiveBoundsError("ISD1 decoded member size mismatch")
    return bytes(output)
