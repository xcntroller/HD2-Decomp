#include "io/chunk_reader.h"

#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using hd2::recovered::ChunkReadError;
using hd2::recovered::ChunkReader;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void expect_error(const std::function<void()>& operation) {
    try {
        operation();
    } catch (const ChunkReadError&) {
        return;
    }
    throw std::runtime_error("Expected an explicit chunk-reader error");
}

std::string header(std::uint16_t id, std::uint32_t size) {
    std::string bytes;
    bytes.push_back(static_cast<char>(id & 0xff));
    bytes.push_back(static_cast<char>((id >> 8) & 0xff));
    for (unsigned shift = 0; shift < 32; shift += 8) {
        bytes.push_back(static_cast<char>((size >> shift) & 0xff));
    }
    return bytes;
}

std::string chunk(std::uint16_t id, const std::string& payload = {}) {
    return header(id, static_cast<std::uint32_t>(payload.size() + 6)) + payload;
}

void header_and_payload() {
    const auto bytes = chunk(0x4c53, chunk(0x0010, "abc"));
    std::istringstream input(bytes);
    ChunkReader reader(input);
    const auto root = reader.enter_chunk();
    require(root && root->id == 0x4c53, "Wrong root ID");
    require(root->offset == 0 && root->total_size == bytes.size(), "Wrong root size");
    require(root->payload_size() == 9 && root->end_offset() == bytes.size(), "Wrong root bounds");
    require(reader.position() == 6 && reader.remaining() == 9, "Wrong payload position");
    const auto child = reader.enter_chunk();
    require(child && child->offset == 6 && child->id == 0x0010, "Wrong child header");
    std::array<char, 3> value{};
    reader.read_exact(value.data(), value.size());
    require(std::string(value.data(), value.size()) == "abc", "Wrong payload bytes");
    require(!reader.enter_chunk(), "End of child must not read another header");
    reader.leave_chunk();
    require(reader.depth() == 1 && reader.remaining() == 0, "Wrong parent boundary");
    require(!reader.enter_chunk(), "End of parent must be normal exhaustion");
    reader.leave_chunk();
    require(reader.depth() == 0 && !reader.enter_chunk(), "Wrong file end");
}

void skip_to_sibling() {
    std::istringstream input(chunk(1, "unread") + chunk(2, "next"));
    ChunkReader reader(input);
    require(reader.enter_chunk()->id == 1, "Wrong first sibling");
    reader.leave_chunk();
    require(reader.position() == 12, "Leaving must skip unread bytes");
    require(reader.enter_chunk()->id == 2, "Wrong second sibling");
    reader.leave_chunk();
    require(reader.remaining() == 0, "Unexpected trailing bytes");
}

void empty_file_and_depth_zero_leave() {
    std::istringstream input("");
    ChunkReader reader(input);
    require(reader.file_size() == 0 && !reader.enter_chunk(), "Empty file must be exhausted");
    reader.read_exact(nullptr, 0);
    reader.leave_chunk();
    require(reader.depth() == 0 && reader.position() == 0, "Empty leave must not underflow");
    std::istringstream nonempty("raw bytes");
    ChunkReader raw(nonempty);
    raw.leave_chunk();
    require(raw.position() == raw.file_size(), "Depth-zero leave must seek to file end");
}

void opening_rewinds_input() {
    std::istringstream input(chunk(0x1234));
    input.seekg(2);
    ChunkReader reader(input);
    require(reader.position() == 0 && reader.file_size() == 6, "Opening must measure then rewind");
    require(reader.enter_chunk()->id == 0x1234, "Wrong little-endian ID");
}

void truncated_headers() {
    for (std::size_t size = 1; size < 6; ++size) {
        std::istringstream input(std::string(size, 'x'));
        ChunkReader reader(input);
        expect_error([&] { reader.enter_chunk(); });
        require(reader.position() == 0 && reader.depth() == 0, "Preflight must not consume input");
    }
}

void invalid_declared_sizes() {
    for (const std::uint32_t size : {0u, 1u, 5u, 7u, 0xffffffffu}) {
        std::istringstream input(header(1, size));
        ChunkReader reader(input);
        expect_error([&] { reader.enter_chunk(); });
        require(reader.depth() == 0, "Invalid header must not push a boundary");
    }
}

void parent_overrun() {
    std::istringstream input(chunk(1, header(2, 7)));
    ChunkReader reader(input);
    reader.enter_chunk();
    expect_error([&] { reader.enter_chunk(); });
    require(reader.depth() == 1, "Child overrun must not push a boundary");
}

void trailing_partial_header() {
    std::istringstream input(chunk(1) + "x");
    ChunkReader reader(input);
    reader.enter_chunk();
    reader.leave_chunk();
    expect_error([&] { reader.enter_chunk(); });
    require(reader.position() == 6, "Trailing byte must not be consumed");
}

void bounded_reads() {
    std::istringstream input(chunk(1, "abcd"));
    ChunkReader reader(input);
    reader.enter_chunk();
    std::array<char, 5> value{'x', 'x', 'x', 'x', 'x'};
    expect_error([&] { reader.read_exact(value.data(), value.size()); });
    expect_error([&] {
        reader.read_exact(value.data(), std::numeric_limits<std::size_t>::max());
    });
    require(reader.position() == 6 && value[0] == 'x', "Rejected read must not touch input/output");
    expect_error([&] { reader.read_exact(nullptr, 1); });
    reader.read_exact(value.data(), 4);
    reader.read_exact(nullptr, 0);
    require(std::string(value.data(), 4) == "abcd" && reader.remaining() == 0, "Wrong exact read");
}

void raw_read_before_entering() {
    std::istringstream input("raw");
    ChunkReader reader(input);
    std::array<char, 3> bytes{};
    reader.read_exact(bytes.data(), bytes.size());
    require(std::string(bytes.data(), bytes.size()) == "raw", "Depth-zero raw read failed");
    expect_error([&] { reader.read_exact(bytes.data(), 1); });
}

void nesting_limit() {
    auto bytes = chunk(1);
    for (std::size_t depth = 0; depth < ChunkReader::max_depth; ++depth) {
        bytes = chunk(1, bytes);
    }
    std::istringstream input(bytes);
    ChunkReader reader(input);
    for (std::size_t depth = 0; depth < ChunkReader::max_depth; ++depth) {
        require(reader.enter_chunk().has_value(), "Valid nesting unexpectedly rejected");
    }
    const auto offset = reader.position();
    expect_error([&] { reader.enter_chunk(); });
    require(reader.position() == offset, "Depth-limit rejection must not consume input");
    for (std::size_t depth = 0; depth < ChunkReader::max_depth; ++depth) {
        reader.leave_chunk();
    }
    require(reader.depth() == 0 && reader.remaining() == 0, "Unwinding must preserve boundaries");
}

class ShortReadBuffer : public std::stringbuf {
public:
    using std::stringbuf::stringbuf;

protected:
    std::streamsize xsgetn(char* destination, std::streamsize count) override {
        return std::stringbuf::xsgetn(destination, count - 1);
    }
};

void short_stream_read() {
    ShortReadBuffer buffer(chunk(1));
    std::istream input(&buffer);
    ChunkReader reader(input);
    expect_error([&] { reader.enter_chunk(); });
    require(reader.depth() == 0, "Short read must not push a boundary");
}

class FailingSeekBuffer : public std::stringbuf {
public:
    using std::stringbuf::stringbuf;
    bool fail_absolute_seeks = false;

protected:
    pos_type seekoff(off_type offset, std::ios::seekdir direction,
                     std::ios::openmode mode) override {
        if (fail_absolute_seeks && direction == std::ios::beg) {
            return pos_type(off_type(-1));
        }
        return std::stringbuf::seekoff(offset, direction, mode);
    }
};

void failed_seek_preserves_depth() {
    FailingSeekBuffer buffer(chunk(1, "data"));
    std::istream input(&buffer);
    ChunkReader reader(input);
    reader.enter_chunk();
    buffer.fail_absolute_seeks = true;
    expect_error([&] { reader.leave_chunk(); });
    require(reader.depth() == 1, "Failed seek must not discard the current boundary");
}

void failed_stream_open() {
    std::istringstream input("data");
    input.setstate(std::ios::badbit);
    expect_error([&] { ChunkReader reader(input); });
}

void externally_moved_cursor_is_rejected() {
    std::istringstream input(chunk(1) + chunk(2));
    ChunkReader reader(input);
    reader.enter_chunk();
    input.seekg(7);
    expect_error([&] { reader.remaining(); });
}

class ReportedSizeBuffer : public std::streambuf {
public:
    explicit ReportedSizeBuffer(off_type size) : size_(size) {}

protected:
    pos_type seekoff(off_type offset, std::ios::seekdir direction,
                     std::ios::openmode) override {
        const auto base = direction == std::ios::end ? size_
            : direction == std::ios::cur ? offset_ : off_type{0};
        offset_ = base + offset;
        return pos_type(offset_);
    }

private:
    off_type size_;
    off_type offset_ = 0;
};

void signed_offset_limit() {
    const auto supported_size = std::numeric_limits<std::int32_t>::max();
    ReportedSizeBuffer valid_buffer(supported_size);
    std::istream valid_input(&valid_buffer);
    ChunkReader reader(valid_input);
    require(reader.file_size() == static_cast<std::uint64_t>(supported_size),
            "Signed 32-bit maximum should be supported");
    ReportedSizeBuffer oversized_buffer(std::streamoff{supported_size} + 1);
    std::istream oversized_input(&oversized_buffer);
    expect_error([&] { ChunkReader oversized(oversized_input); });
}

}

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests{
        {"header and payload", header_and_payload},
        {"skip to sibling", skip_to_sibling},
        {"empty file and depth-zero leave", empty_file_and_depth_zero_leave},
        {"opening rewinds input", opening_rewinds_input},
        {"truncated headers", truncated_headers},
        {"invalid declared sizes", invalid_declared_sizes},
        {"parent overrun", parent_overrun},
        {"trailing partial header", trailing_partial_header},
        {"bounded reads", bounded_reads},
        {"raw read before entering", raw_read_before_entering},
        {"nesting limit", nesting_limit},
        {"short stream read", short_stream_read},
        {"failed seek preserves depth", failed_seek_preserves_depth},
        {"failed stream open", failed_stream_open},
        {"externally moved cursor", externally_moved_cursor_is_rejected},
        {"signed offset limit", signed_offset_limit},
    };
    std::size_t failures = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
            ++failures;
        }
    }
    std::cout << (tests.size() - failures) << '/' << tests.size()
              << " chunk-reader tests passed\n";
    return failures == 0 ? 0 : 1;
}
