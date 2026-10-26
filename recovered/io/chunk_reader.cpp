#include "io/chunk_reader.h"

#include <array>
#include <limits>
#include <string>

namespace hd2::recovered {

std::uint64_t ChunkHeader::end_offset() const {
    return offset + total_size;
}

std::uint32_t ChunkHeader::payload_size() const {
    return total_size - header_size;
}

ChunkReader::ChunkReader(std::istream& input) : input_(input) {
    input_.seekg(0, std::ios::end);
    if (!input_) {
        throw ChunkReadError("Cannot seek to the end of the input stream");
    }
    file_size_ = position();
    if (file_size_ > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
        throw ChunkReadError("Input exceeds the supported signed 32-bit offset range");
    }
    seek_to(0);
}

std::optional<ChunkHeader> ChunkReader::enter_chunk() {
    const auto bytes_left = remaining();
    if (bytes_left == 0) {
        return std::nullopt;
    }
    if (bytes_left < ChunkHeader::header_size) {
        throw ChunkReadError("Truncated chunk header at offset " + std::to_string(position()));
    }
    if (chunk_ends_.size() == max_depth) {
        throw ChunkReadError("Chunk nesting exceeds the supported depth");
    }
    const auto offset = position();
    std::array<unsigned char, ChunkHeader::header_size> header_bytes{};
    read_exact(header_bytes.data(), header_bytes.size());
    const auto id = static_cast<std::uint16_t>(
        std::uint32_t{header_bytes[0]} | (std::uint32_t{header_bytes[1]} << 8));
    const auto total_size = std::uint32_t{header_bytes[2]}
        | (std::uint32_t{header_bytes[3]} << 8)
        | (std::uint32_t{header_bytes[4]} << 16)
        | (std::uint32_t{header_bytes[5]} << 24);
    if (total_size < ChunkHeader::header_size || total_size > bytes_left) {
        throw ChunkReadError("Invalid chunk length at offset " + std::to_string(offset));
    }
    const ChunkHeader header{id, total_size, offset};
    chunk_ends_.push_back(header.end_offset());
    return header;
}

void ChunkReader::read_exact(void* destination, std::size_t count) {
    if (count > remaining()) {
        throw ChunkReadError("Read would cross the current chunk boundary");
    }
    if (count == 0) {
        return;
    }
    if (destination == nullptr) {
        throw ChunkReadError("Nonempty read requires a destination buffer");
    }
    input_.read(static_cast<char*>(destination), static_cast<std::streamsize>(count));
    if (!input_ || input_.gcount() != static_cast<std::streamsize>(count)) {
        throw ChunkReadError("Input stream did not return the requested byte count");
    }
}

void ChunkReader::leave_chunk() {
    seek_to(current_end());
    if (!chunk_ends_.empty()) {
        chunk_ends_.pop_back();
    }
}

std::uint64_t ChunkReader::position() const {
    const std::streamoff offset = input_.tellg();
    if (offset < 0) {
        throw ChunkReadError("Cannot determine the input stream position");
    }
    return static_cast<std::uint64_t>(offset);
}

std::uint64_t ChunkReader::remaining() const {
    const auto offset = position();
    const auto end = current_end();
    if (offset > end) {
        throw ChunkReadError("Input stream position is beyond the current chunk");
    }
    return end - offset;
}

std::uint64_t ChunkReader::file_size() const {
    return file_size_;
}

std::size_t ChunkReader::depth() const {
    return chunk_ends_.size();
}

std::uint64_t ChunkReader::current_end() const {
    return chunk_ends_.empty() ? file_size_ : chunk_ends_.back();
}

void ChunkReader::seek_to(std::uint64_t offset) {
    input_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!input_) {
        throw ChunkReadError("Cannot seek to chunk end at offset " + std::to_string(offset));
    }
}

}
