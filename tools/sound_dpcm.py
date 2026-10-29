"""Bounded rw_data 100097f7 reconstruction for codec-32 PCM16 mono/stereo.

Only canonical 44-byte PCM WAV headers are supported. Other sound quality
codes, sample widths, metadata chunks and non-PCM formats remain unsupported.
"""

import struct


def pcm16_channels(header: bytes) -> int:
    if (len(header) != 44 or header[:4] != b"RIFF" or header[8:16] != b"WAVEfmt "
            or header[36:40] != b"data"):
        raise ValueError("Codec 32 requires a canonical PCM WAV header")
    riff_size, = struct.unpack_from("<I", header, 4)
    fmt_size, encoding, channels, rate, byte_rate, alignment, bits = struct.unpack_from(
        "<IHHIIHH", header, 16
    )
    data_size, = struct.unpack_from("<I", header, 40)
    if (fmt_size != 16 or encoding != 1 or channels not in (1, 2) or bits != 16
            or rate == 0 or alignment != channels * 2 or byte_rate != rate * alignment
            or data_size % alignment != 0 or riff_size != data_size + 36):
        raise ValueError("Unsupported codec-32 PCM format or inconsistent WAV header")
    return channels


def decompress_sound32(data: bytes, limit: int, header: bytes | None = None) -> bytes:
    """Decode a first/header block or continuation with fresh per-channel seeds.

    Original row-six magnitude table equals floor(i*i*32767/(127*127)) for
    every entry. Sign is bit seven; sample stores wrap to sixteen bits rather
    than saturating. Host bounds checks reject truncated/overlong blocks.
    """
    if not data or data[0] != 32 or not 0 < limit <= 32768:
        raise ValueError("Invalid codec-32 block or decoded size")
    position = 1
    output = bytearray()
    if header is None:
        header = data[1:45]
        pcm16_channels(header)
        output.extend(header)
        position += 44
    channels = pcm16_channels(header)
    seed_size = channels * 2
    if limit < len(output) + seed_size or (limit - len(output)) % seed_size:
        raise ValueError("Codec-32 output is not a whole seeded PCM frame")
    delta_count = (limit - len(output) - seed_size) // 2
    if len(data) != position + seed_size + delta_count:
        raise ValueError("Codec-32 stored length does not match decoded size")
    seeds = data[position:position + seed_size]
    output.extend(seeds)
    previous = list(struct.unpack("<" + "H" * channels, seeds))
    for index, code in enumerate(data[position + seed_size:]):
        magnitude = (code & 127) ** 2 * 32767 // (127 * 127)
        channel = index % channels
        sample = (previous[channel] + (-magnitude if code & 128 else magnitude)) & 0xffff
        output.extend(struct.pack("<H", sample))
        previous[channel] = sample
    return bytes(output)
