#ifndef HD2_GAME_OVERLAY_OWNER_H
#define HD2_GAME_OVERLAY_OWNER_H

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hd2::game {

//00490740/004908B0 and their inline array elements004907E0/00490800/
//00490830/004908A0. Native ownership representation, not binary object layout.
// FrameOwner is an owning engine reference whose deleter calls frame virtual+00.
// Row+20..2F weights are populated by00490C80; their cold zeroes are a defined
// initialization guard, not constructor recovery credit.
// Slot order is significant: frames release forwards, strings release backwards;
// rows and records destruct backwards. No actor or visual is created by this ctor.
template<class FrameOwner>
struct OverlayRow {
    std::array<FrameOwner, 4> frames{};
    std::array<std::shared_ptr<const std::string>, 4> text{};
    std::array<std::uint32_t,4> weights{}; //00490C80 raw DWORDs; animation consumers assign their interpretation.
    ~OverlayRow() {
        for (auto& frame : frames) frame.reset();
        for (auto i = text.size(); i-- != 0;) text[i].reset();
    }
};

template<class FrameOwner>
struct OverlayRecord { std::array<OverlayRow<FrameOwner>, 13> rows; };

template<class FrameOwner>
class OverlayOwner {
public:
    using FrameOwnerType=FrameOwner;
    OverlayOwner() = default;
    OverlayOwner(const OverlayOwner&) = delete;
    OverlayOwner& operator=(const OverlayOwner&) = delete;
    ~OverlayOwner() {
        // std::array tears down records/rows in reverse after this body. The
        // original frees the vector after those records, so do so explicitly.
        records_.reset();
        std::vector<std::uint32_t>().swap(words84);
    }
    OverlayRecord<FrameOwner>& record(std::size_t index) { return records_->at(index); }
    const OverlayRecord<FrameOwner>& record(std::size_t index) const { return records_->at(index); }
    static constexpr std::size_t capacity = 500;
    std::uint32_t word38 = 0;
    std::uint8_t byte3c = 0;
    std::array<std::uint32_t, 4> words40{};
    std::array<std::uint8_t, 3> bytes58{};
    std::array<std::uint32_t, 4> words70{};
    std::vector<std::uint32_t> words84;
private:
    // Heap storage avoids a several-hundred-KB controller stack frame.
    std::unique_ptr<std::array<OverlayRecord<FrameOwner>, capacity>> records_ =
        std::make_unique<std::array<OverlayRecord<FrameOwner>, capacity>>();
};

} // namespace hd2::game
#endif
