#ifndef HD2_HOST_PRE_MISSION_SOLDIER_SELECTION_H
#define HD2_HOST_PRE_MISSION_SOLDIER_SELECTION_H
#include "host/pre_mission_controls.h"

namespace hd2::host {

// 00691840/00675F30: shared authored four-photo selection screen. The caller
// owns0200006A semantics (team leader swap or the single active mission actor).
class PreMissionSoldierSelection {
  public:
    PreMissionSoldierSelection(MenuScene &scene, MenuDriver &driver, HMODULE module,
                               frontend::MenuEventQueue &queue)
        : scene_(scene), module_(module), queue_(queue), clone_{driver, module},
          model_(scene.load_screen_model(module, "selection screen")) {}
    EngineFrame *model() const { return model_.get(); }
    std::size_t selected() const { return selected_; }
    frontend::MenuScreenDefinition definition() {
        using namespace frontend;
        MenuScreenDefinition out{"SELECTION SCREEN", 0x08000000, "selection screen", {}};
        out.controls.push_back(
            text_definition(0x08100000, "text_select one soldier", {caption_definition(12, 0xab2)}));
        for (unsigned i = 0; i < 4; ++i) {
            photos_[i] = scene_.control_frame(names[i], module_, false, model_.get());
            out.controls.push_back({0x08200000 + i * 0x100000,
                                    names[i],
                                    MenuControlKind::image,
                                    nullptr,
                                    nullptr,
                                    MenuControlRole::none,
                                    {{0, 0x01000009, i, 0, 0, {}},
                                     PreMissionImage::texture_event(0, nullptr),
                                     observer_control_event(0, 0x04001000, this, &choose)}});
        }
        out.controls.push_back(button_definition(
            0x08600000, "button01", "normal*", "actived*", MenuControlRole::single_player,
            {caption_definition(13, 0x7d1), {0, 0x0600106a, 0, 0, 0, {}}, {0, 0x06001005, 0, 0, 0, {}}}));
        out.controls.push_back(
            button_definition(0x08700000, "button00", "normal*", "actived*", MenuControlRole::single_player,
                              {caption_definition(13, 0x7d8), {0, 0x06001005, 0, 0, 0, {}}}));
        selector_ = scene_.control_frame("selector", module_, false, model_.get());
        return out;
    }
    void open(const std::vector<const frontend::HumanRecord *> &humans) {
        if (humans.empty() || humans.size() > 4)
            throw std::logic_error("Selection screen requires one to four real soldiers");
        count_ = humans.size();
        selected_ = 0;
        for (unsigned i = 0; i < 4; ++i)
            queue_.enqueue(PreMissionImage::texture_event(
                0x08200000 + i * 0x100000,
                i < humans.size() && humans[i] ? static_cast<EngineFrame *>(humans[i]->portrait) : nullptr));
        position_selector();
        queue_.enqueue({0, 0x02000002, 2, 0, 0, {}});
    }

  private:
    void position_selector() {
        clone_.link(selector_, photos_.at(selected_), 0);
        scene_.set_frame_local_position(selector_, {0, 0, -.1f});
        scene_.set_screen_visible(module_, selector_, true);
        scene_.update_screen_frame(module_, selector_);
    }
    static bool choose(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionSoldierSelection *>(owner);
        const auto selected = static_cast<PreMissionImage *>(sender)->user_data();
        if (selected < self.count_) {
            self.selected_ = selected;
            self.position_selector();
        }
        return true;
    }
    inline static constexpr const char *names[4] = {"photo03", "photo04", "photo05", "photo06"};
    MenuScene &scene_;
    HMODULE module_;
    frontend::MenuEventQueue &queue_;
    CinematicsCloneBackend clone_;
    MenuDriver::Owner model_;
    std::array<EngineFrame *, 4> photos_{};
    EngineFrame *selector_ = nullptr;
    std::size_t selected_ = 0, count_ = 0;
};
} // namespace hd2::host
#endif
