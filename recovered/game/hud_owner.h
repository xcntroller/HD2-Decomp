#ifndef HD2_GAME_HUD_OWNER_H
#define HD2_GAME_HUD_OWNER_H

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace hd2::game {

// Initialized state of004A0270's0x94 record. The three embedded sequences are
// empty at construction; their population/animation remain later HUD consumers.
struct HudTrackState {
    void* pointer4 = nullptr;
    void* pointer18 = nullptr;
    std::array<std::uint32_t, 3> values28{};
    void* pointer38 = nullptr;
    std::array<std::uint32_t, 3> values48{};
    void* pointer58 = nullptr;
    std::array<std::uint32_t, 3> values68{};
    std::uint32_t word74 = 0, word7c = 0, word80 = 0, word84 = 0, word88 = 0;
    std::uint8_t byte78 = 0, byte79 = 0;
    std::int32_t word8c = -1;
    std::uint32_t word90 = 1;
};

//0049A420/0049FEF0/004A0410/004A05D0 native construction graph. The constructor
// has no model-loading calls: HUD0049A940 initializes artwork after mission load.
// Keep initialized data and all77 real text/font owners; do not invent contents
// for the large unwritten original buffers or treat this as a drawn HUD.
template<class Text>
class HudOwner {
public:
    struct PositionedText { std::uint32_t original_offset; std::unique_ptr<Text> text; };
    template<class MakeText>
    explicit HudOwner(MakeText make_text) {
        texts_.reserve(77);
        const auto add = [&](std::uint32_t offset) {
            auto text = make_text();
            if (!text) throw std::bad_alloc();
            texts_.push_back({offset, std::move(text)});
        };
        //0049FEF0 is the subobject at+30, not this+00 (decompiler omits ECX).
        constexpr std::uint32_t base = 0x30;
        add(base + 0x20);
        for (std::uint32_t i = 0; i < 12; ++i) { //004A0410: two text owners
            add(base + 0xf0 + i * 0x148);
            add(base + 0xf0 + i * 0x148 + 0xa8);
        }
        for (const auto offset : {0x1050u, 0x1818u}) {
            for (std::uint32_t i = 0; i < 2; ++i) { //004A05D0: six text owners
                const auto start = base + offset + i * 0x3e4;
                add(start);
                for (std::uint32_t j = 0; j < 4; ++j) add(start + 0xc0 + j * 0x9c);
                add(start + 0x344);
            }
        }
        add(base + 0x1fe4);
        for (const auto offset : {0x2080u, 0x24e0u})
            for (std::uint32_t i = 0; i < 7; ++i) add(base + offset + i * 0x9c);
        for (std::uint32_t i = 0; i < 4; ++i) add(base + 0x2b80 + i * 0x9c);
        //00494980 colour defaults and32×0049A5E0 data records are native fields below.
        for (std::uint32_t i = 0; i < 8; ++i) add(0x10bc8 + i * 0x9c);
        add(0x6b85c);
        if (texts_.size() != 77) throw std::logic_error("Invalid HUD construction graph");
    }
    ~HudOwner() { while (!texts_.empty()) texts_.pop_back(); }
    HudOwner(const HudOwner&) = delete;
    HudOwner& operator=(const HudOwner&) = delete;
    bool enabled() const { return (flags & 1u) != 0; } //0049B7D0
    const std::vector<PositionedText>& text_owners() const { return texts_; }
    std::uint32_t flags = 0; //+0C, final constructor write
    std::array<std::uint32_t, 11> panel_words{}; //0049FEF0's explicit null/zero fields
    std::array<std::uint32_t, 7> panel_geometry{}; //004A01F0
    std::array<HudTrackState, 4> tracks{};
    std::array<float, 3> colours{{255.f,255.f,255.f}}; //00494980+00/+04/+08
    std::uint32_t colour_count = 0, colour_period = 50;
    std::uint32_t colour_d36c = 0, colour_d370 = 0x3cb43958, colour_d374 = 0, colour_d378 = 0;
    std::uint8_t colour_d384 = 1;
    struct Item { std::uint32_t first = 0, second = 0; std::uint8_t enabled = 0; };
    std::array<Item, 32> items{}; //0049A5E0 at+10928 +i*0C
    struct Timer { std::uint32_t value = 0; float time = 0; };
    std::array<Timer, 18> timers{}; //+11140+i*10, first/last fields only
private:
    std::vector<PositionedText> texts_;
};

} // namespace hd2::game
#endif
