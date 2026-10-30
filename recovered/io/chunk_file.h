#ifndef HD2_IO_CHUNK_FILE_H
#define HD2_IO_CHUNK_FILE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace hd2::io {

// Original object state, independent of native pointers. Index0 is root end;
// deeper entries hold read ends OR writer header starts. Close/open deliberately
// do not reset depth or mode. Native initial zeros describe normal fresh use.
struct ChunkFileState {
    std::uint8_t opened = 0;
    std::array<std::uint8_t, 3> padding{};
    std::uint32_t handle = 0xffffffffu;
    std::uint8_t write_mode = 0;
    std::array<std::uint8_t, 3> padding2{};
    std::array<std::uint32_t, 257> offsets{};
    std::uint32_t depth = 0;
};
static_assert(offsetof(ChunkFileState, offsets) == 0x0c);
static_assert(offsetof(ChunkFileState, depth) == 0x410);
static_assert(sizeof(ChunkFileState) == 0x414);

// Client 00405620/5690/5750/5820/5890/5940/59b0/59f0 and 00415c90.
// These are original DTA operations, separate from the early strict portable
// ChunkReader. No automatic file-size/parent validation is invented here.
// Files must outlive this owner. Actual DLL callbacks do not throw or mutate
// this owner's state. Native destructor performs the original close phase.
template<class Files>
class ChunkFile {
public:
    explicit ChunkFile(Files& files) : files_(files) {}
    ~ChunkFile() { close(); }
    ChunkFile(const ChunkFile&) = delete;
    ChunkFile& operator=(const ChunkFile&) = delete;
    const ChunkFileState& state() const { return state_; }

    bool open(const char* name, std::uint8_t write_mode = 0) {
        if (state_.opened) return false;
        state_.offsets[0] = 0xffffffffu;
        state_.handle = write_mode ? files_.open_write(name, 0x10000) : files_.open(name, 0);
        if (state_.handle == 0xffffffffu) return false; // Other high-bit handles valid.
        state_.write_mode = write_mode; // Preserve actual nonzero byte.
        if (!write_mode) state_.offsets[0] = files_.seek(state_.handle, 0, 2);
        files_.seek(state_.handle, 0, 0); // Result ignored, even failure.
        state_.opened = 1;
        return true;
    }

    bool close() {
        if (!state_.opened) return false;
        if (state_.write_mode)
            while (state_.depth) leave(); // Backpatch all open writer chunks.
        files_.close(state_.handle); // Read mode does NOT unwind its stack.
        state_.handle = 0xffffffffu;
        state_.opened = 0;
        return true;
    }

    // 00415c90 reads raw six-byte header directly (not bounded read()). Failed
    // header reads leave output/depth alone, but can consume partial file bytes.
    // No mode, end-of-parent, minimum-size or seek-success gate in original.
    bool enter(std::uint16_t& id) {
        if (!state_.opened) return false;
        require_push_capacity();
        std::array<unsigned char, 6> bytes{};
        if (files_.read(state_.handle, bytes.data(), 6) != 6) return false;
        const auto position = files_.seek(state_.handle, 0, 1);
        state_.offsets[state_.depth + 1] = position - 6u + load32(bytes.data() + 2);
        ++state_.depth;
        id = static_cast<std::uint16_t>(bytes[0] | (std::uint16_t{bytes[1]} << 8));
        return true;
    }

    // 00405820 has no write-mode gate. Push header start BEFORE attempting the
    // six-byte write; a short/failed write still leaves that stack entry live.
    bool begin(std::uint16_t id) {
        if (!state_.opened) return false;
        require_push_capacity();
        std::array<unsigned char, 6> bytes{};
        bytes[0] = static_cast<unsigned char>(id);
        bytes[1] = static_cast<unsigned char>(id >> 8);
        state_.offsets[state_.depth + 1] = files_.seek(state_.handle, 0, 1);
        ++state_.depth;
        return files_.write(state_.handle, bytes.data(), 6) == 6;
    }

    bool leave(std::uint32_t* size_output = nullptr) {
        if (!state_.opened) return false;
        require_depth();
        if (state_.write_mode) {
            const auto position = files_.seek(state_.handle, 0, 1);
            const auto size = position - state_.offsets[state_.depth];
            files_.seek(state_.handle, state_.offsets[state_.depth] + 2u, 0);
            std::array<unsigned char, 4> bytes{};
            store32(bytes.data(), size);
            files_.write(state_.handle, bytes.data(), 4); // Result ignored.
            files_.seek(state_.handle, position, 0); // Result ignored.
            if (size_output) *size_output = size; // Write-mode only.
        } else {
            files_.seek(state_.handle, state_.offsets[state_.depth], 0);
        }
        if (state_.depth) --state_.depth;
        return true; // Includes depth zero and failed underlying operations.
    }

    bool read(void* output, std::uint32_t count) {
        if (!state_.opened || state_.write_mode) return false;
        if (count > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()))
            throw std::length_error("Negative original chunk read count");
        require_depth();
        if (state_.depth) {
            const auto position = files_.seek(state_.handle, 0, 1);
            const auto remaining = signed_word(state_.offsets[state_.depth] - position);
            const auto available = remaining < 0 ? 0 : remaining;
            if (static_cast<std::int32_t>(count) > available) return false;
        }
        if (count && !output) throw std::invalid_argument("Null chunk read output");
        // Original signed >= comparison accepts oversized successful returns.
        return signed_word(files_.read(state_.handle, output, count)) >= static_cast<std::int32_t>(count);
    }

    bool write(const void* input, std::uint32_t count) {
        if (!state_.opened || !state_.write_mode) return false;
        if (count && !input) throw std::invalid_argument("Null chunk write input");
        return files_.write(state_.handle, input, count) == count;
    }

    // 004059f0's one-shot chunk: ignore begin/data/leave operation results.
    // Like original, begin runs even in read mode; write data is mode-gated.
    void write_chunk(std::uint16_t id, const void* input, std::uint32_t count) {
        if (!state_.opened) return;
        begin(id);
        if (state_.opened && state_.write_mode) write(input, count);
        if (state_.opened) leave();
    }

private:
    static std::int32_t signed_word(std::uint32_t word) {
        std::int32_t result;
        std::memcpy(&result, &word, sizeof(result));
        return result;
    }
    static std::uint32_t load32(const unsigned char* p) {
        return std::uint32_t{p[0]} | (std::uint32_t{p[1]} << 8) |
               (std::uint32_t{p[2]} << 16) | (std::uint32_t{p[3]} << 24);
    }
    static void store32(unsigned char* p, std::uint32_t value) {
        for (unsigned i = 0; i != 4; ++i) p[i] = static_cast<unsigned char>(value >> (i * 8));
    }
    void require_depth() const {
        if (state_.depth > 256) throw std::length_error("Invalid original chunk stack depth");
    }
    void require_push_capacity() const {
        if (state_.depth >= 256) throw std::length_error("Original chunk stack capacity exceeded");
    }
    Files& files_;
    ChunkFileState state_;
};

}
#endif
