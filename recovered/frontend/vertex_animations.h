#ifndef HD2_FRONTEND_VERTEX_ANIMATIONS_H
#define HD2_FRONTEND_VERTEX_ANIMATIONS_H

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace hd2::frontend {

struct VertexAnimationBlock {
    struct Release { void operator()(std::uint8_t* p) const { ::operator delete(p, std::align_val_t(16)); } };
    std::unique_ptr<std::uint8_t, Release> data;
    std::uint32_t bytes = 0;
    explicit VertexAnimationBlock(std::uint32_t count)
        : data(static_cast<std::uint8_t*>(::operator new(count, std::align_val_t(16)))), bytes(count) {}
};
// 00473700's four explicit zero fields; the 20-byte name is supplied by the file.
struct VertexAnimation {
    std::array<std::uint8_t, 20> name{};
    std::uint32_t last_frame = 0;
    std::vector<std::uint32_t> sizes;
    std::vector<std::vector<VertexAnimationBlock>> frames;
};

// 00473EC0: required module9 startup loader. Missing open preserves old entries
// and still calls close(-1). A valid open clears old entries before any reads.
// Header DWORD is read/ignored. Frames are inclusive 0..last_frame; each group
// block has sizes[group]*32 bytes, allocated at16-byte alignment.
class VertexAnimations {
public:
    const auto& entries() const { return entries_; }
    void clear() { while (!entries_.empty()) entries_.pop_back(); }
    template<class Files>
    void load(Files& files) {
        const auto handle = files.open("Tables\\Vertanim.def", 0);
        if (handle == 0xffffffffu) { files.close(handle); return; }
        struct Guard { Files& files; std::uint32_t handle; ~Guard() { files.close(handle); } } guard{files, handle};
        clear(); // Keep original vector capacity.
        std::uint32_t ignored = 0;
        files.read(handle, &ignored, 4);
        for (;;) {
            auto entry = std::make_unique<VertexAnimation>();
            std::array<std::uint8_t, 4> word{};
            if (!read(files, handle, entry->name.data(), 20) ||
                !read(files, handle, word.data(), 4)) return;
            entry->last_frame = decode(word);
            if (!read(files, handle, word.data(), 4)) return;
            const auto groups = decode(word);
            if (groups > 0x1fffffffu ||
                (entry->last_frame != 0xffffffffu && entry->last_frame > 0x1fffffffu))
                throw std::length_error("Invalid original vertex animation dimensions");
            entry->sizes.resize(groups);
            for (auto& size : entry->sizes) {
                if (!read(files, handle, word.data(), 4)) return;
                size = decode(word);
                if (size > 0x03ffffffu)
                    throw std::length_error("Invalid original vertex animation block");
            }
            for (std::uint32_t frame = 0; entry->last_frame != 0xffffffffu && frame <= entry->last_frame; ++frame) {
                entry->frames.emplace_back();
                auto& blocks = entry->frames.back();
                for (auto size : entry->sizes) {
                    blocks.emplace_back(size * 32);
                    // A zero-byte request returns zero and aborts this record,
                    // as in the original signed-positive read condition.
                    if (!read(files, handle, blocks.back().data.get(), blocks.back().bytes)) return;
                }
            }
            entries_.push_back(std::move(entry));
        }
    }
    template<class Files>
    void startup(const std::uint32_t& gate, Files& files) {
        // 00474A00: original module+0C reset, then load only when gate2 set.
        if (gate & 2) { word0c = 0; load(files); }
    }
    std::uint32_t word0c = 0;
private:
    static std::uint32_t decode(const std::array<std::uint8_t, 4>& b) {
        return std::uint32_t(b[0]) | std::uint32_t(b[1]) << 8 |
               std::uint32_t(b[2]) << 16 | std::uint32_t(b[3]) << 24;
    }
    template<class Files>
    static bool read(Files& files, std::uint32_t handle, void* output, std::uint32_t bytes) {
        const auto count = files.read(handle, output, bytes);
        if (!count || (count & 0x80000000u)) return false;
        // Original consumes partially initialized strings/counts/vertex memory.
        // Reject that undefined path instead of constructing guessed data.
        if (count != bytes) throw std::runtime_error("Partial original vertex animation read");
        return true;
    }
    std::vector<std::unique_ptr<VertexAnimation>> entries_;
};

}
#endif
