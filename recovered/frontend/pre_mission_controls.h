#ifndef HD2_FRONTEND_PRE_MISSION_CONTROLS_H
#define HD2_FRONTEND_PRE_MISSION_CONTROLS_H

#include "frontend/control_runtime.h"
#include <limits>
#include <vector>

namespace hd2::frontend {

// 00650A10, kind0C. Invalid selection hides the old image but retains its
// index. Clearing the image also retains the index; neither operation removes
// registrations. Missing named children are ignored by the original receiver.
template <class Backend> class PreMissionImageSelector {
  public:
    explicit PreMissionImageSelector(Backend &backend) : backend_(backend) {}
    bool dispatch(ControlEvent &event) {
        if (event.code & 0x040ff000u)
            return false;
        switch (event.code & 0x03000fffu) {
        case 0x01000032:
            if (event.text)
                if (auto *frame = backend_.find_image(event.text->c_str())) {
                    backend_.show_image(frame, false);
                    images_.push_back(frame);
                }
            return true;
        case 0x02000019:
            hide();
            if (event.argument < images_.size()) {
                backend_.show_image(images_[event.argument], true);
                selected_ = event.argument;
            }
            return true;
        case 0x0200001a:
            hide();
            return true;
        case 0x02000025:
            return true;
        default:
            return false;
        }
    }
    std::size_t size() const { return images_.size(); }
    std::uint32_t selected() const { return selected_; }

  private:
    void hide() {
        if (selected_ < images_.size())
            backend_.show_image(images_[selected_], false);
    }
    Backend &backend_;
    std::vector<typename Backend::Image *> images_;
    std::uint32_t selected_ = 0xffffffffu;
};

// 00650350/0064FEA0/0064FF80, kind25. A linked control participates through
// synchronous70/71 messages: a refund increments the counter before lowering
// this delta, and an increase decrements it before raising this delta.
// Programmatic72/73 assignments do not debit the linked counter.
class PreMissionStatValue {
  public:
    std::int32_t base() const { return base_; }
    std::int32_t delta() const { return delta_; }
    std::int32_t value() const { return base_ + delta_; }
    void base(std::int32_t value) {
        base_ = value;
        delta_ = 0;
    }
    void value(std::int32_t value) {
        const auto difference = std::int64_t(value) - base_;
        if (difference > std::numeric_limits<std::int32_t>::max())
            throw std::runtime_error("Invalid hero stat delta");
        delta_ = difference < 0 ? 0 : static_cast<std::int32_t>(difference);
    }
    template <class Linked> bool increase(Linked &&linked) {
        if (std::int64_t(base_) + delta_ >= 100 || delta_ == std::numeric_limits<std::int32_t>::max() ||
            !linked(0x01000071u))
            return false;
        ++delta_;
        return true;
    }
    template <class Linked> bool decrease(Linked &&linked) {
        if (delta_ <= 0 || !linked(0x01000070u))
            return false;
        --delta_;
        return true;
    }

  private:
    std::int32_t base_ = 0, delta_ = 0;
};

} // namespace hd2::frontend
#endif
