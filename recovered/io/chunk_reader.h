#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <optional>
#include <stdexcept>
#include <vector>

namespace hd2::recovered {

struct ChunkHeader {
    static constexpr std::uint32_t header_size = 6;

    std::uint16_t id;
    std::uint32_t total_size;
    std::uint64_t offset;

    std::uint64_t end_offset() const;
    std::uint32_t payload_size() const;
};

class ChunkReadError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ChunkReader {
public:
    static constexpr std::size_t max_depth = 256;

    explicit ChunkReader(std::istream& input);
    ChunkReader(const ChunkReader&) = delete;
    ChunkReader& operator=(const ChunkReader&) = delete;

    std::optional<ChunkHeader> enter_chunk();
    void read_exact(void* destination, std::size_t count);
    void leave_chunk();

    std::uint64_t position() const;
    std::uint64_t remaining() const;
    std::uint64_t file_size() const;
    std::size_t depth() const;

private:
    void seek_to(std::uint64_t offset);
    std::uint64_t current_end() const;

    std::istream& input_;
    std::uint64_t file_size_ = 0;
    std::vector<std::uint64_t> chunk_ends_;
};

}
