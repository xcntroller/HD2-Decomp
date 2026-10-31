#ifndef HD2_FRONTEND_CINEMATICS_H
#define HD2_FRONTEND_CINEMATICS_H
#include "frontend/opening_playback.h"
#include "frontend/profile_payload.h"
#include <algorithm>
#include <cmath>
#include <optional>

namespace hd2::frontend {
// 00614140/00615820: original ten identities remain allocated while the active
//  list contains only unlocked rows. No placeholder/unlocked movie is invented.
class CinematicsList {
  public:
    static constexpr std::uint32_t identifier = 0x1c700000;
    static constexpr unsigned page_size = 17;
    static std::uint32_t row_identifier(unsigned index) {
        if (index >= 10)
            throw std::out_of_range("Original Cinematics row index");
        return identifier | ((index + 1) << 12);
    }
    void clear() {
        active_.clear();
        focused_.reset();
        selected_.reset();
    }
    bool include(std::uint32_t target) {
        for (unsigned i = 0; i < 10; ++i)
            if (row_identifier(i) == target) {
                if (std::find(active_.begin(), active_.end(), i) == active_.end()) {
                    active_.push_back(i);
                    std::sort(active_.begin(), active_.end());
                }
                return true;
            }
        return false;
    }
    const auto &active() const { return active_; }
    bool contains(unsigned index) const {
        return std::find(active_.begin(), active_.end(), index) != active_.end();
    }
    std::int32_t range() const {
        return static_cast<std::int32_t>(active_.size()) - static_cast<std::int32_t>(page_size) + 1;
    }
    std::optional<unsigned> focused() const { return focused_; }
    std::optional<unsigned> selected() const { return selected_; }
    bool focus(unsigned index) {
        if (!contains(index))
            return false;
        focused_ = index;
        return true;
    }
    void select(unsigned index) {
        if (!focus(index))
            throw std::logic_error("Unavailable Cinematics row");
        selected_ = index;
    }
    void leave() { focused_.reset(); }
    std::optional<unsigned> target_index(std::uint32_t target) const {
        for (auto index : active_)
            if ((target & 0x000ff000) == (row_identifier(index) & 0x000ff000))
                return index;
        return {};
    }
    std::optional<unsigned> adjacent(bool forward) const {
        if (active_.empty())
            return {};
        if (!focused_)
            return forward ? active_.front() : active_.back();
        const auto found = std::find(active_.begin(), active_.end(), *focused_);
        if (found == active_.end())
            return {};
        if (forward)
            return found + 1 == active_.end() ? std::optional<unsigned>{} : *(found + 1);
        return found == active_.begin() ? std::optional<unsigned>{} : *(found - 1);
    }

  private:
    std::vector<unsigned> active_;
    std::optional<unsigned> focused_, selected_;
};

// 00667C90/00667D50: frame world bounds, exact float4/3 projection and fit.
//  Width-limited previews align to the top; only the height-limited X centers.
inline VideoRectangle cinematics_preview_rectangle(const std::array<float, 8> &bounds, int width, int height,
                                                   int movie_width, int movie_height) {
    if (width <= 0 || height <= 0 || movie_width <= 0 || movie_height <= 0)
        throw std::invalid_argument("Invalid preview dimensions");
    for (auto value : bounds)
        if (!std::isfinite(value))
            throw std::invalid_argument("Nonfinite preview bounds");
    const float left = static_cast<float>(static_cast<long double>(width) * bounds[0]);
    const float right = static_cast<float>(static_cast<long double>(width) * bounds[4]);
    const float bottom = static_cast<float>(static_cast<long double>(height) * bounds[1] * (-4.0f / 3));
    const float top = static_cast<float>(static_cast<long double>(height) * bounds[5] * (-4.0f / 3));
    const float span = right - left, tall = bottom - top;
    if (span <= 0 || tall <= 0)
        throw std::invalid_argument("Empty preview frame");
    if (static_cast<long double>(movie_height) / movie_width <= static_cast<long double>(tall) / span)
        return {static_cast<int>(left), static_cast<int>(top), static_cast<int>(span),
                static_cast<int>(static_cast<long double>(span) / movie_width * movie_height)};
    const auto fitted = static_cast<int>(static_cast<long double>(tall) / movie_height * movie_width);
    return {static_cast<int>((static_cast<long double>(span) - fitted) * .5f + left), static_cast<int>(top),
            fitted, static_cast<int>(tall)};
}
} // namespace hd2::frontend
#endif
