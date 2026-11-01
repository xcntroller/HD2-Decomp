#ifndef HD2_HOST_PRE_MISSION_DESCRIPTION_H
#define HD2_HOST_PRE_MISSION_DESCRIPTION_H
#include "host/pre_mission_controls.h"

namespace hd2::host {

// Nonnumbered description owner at front-end+FC.006695C0 binds the actual
// text/description frames,006697A0 shows only after pointer idle>=500 with no
// held button, and006698F0 renders its text. It never enters the screen stack.
class PreMissionDescription {
  public:
    PreMissionDescription(MenuScene &scene, HMODULE module, bool medal = false)
        : scene_(scene), module_(module), model_(scene.load_screen_model(module, "description")),
          definition_{"DESCRIPTION",
                      0xfc000000,
                      "description",
                      {frontend::text_definition(0xfc100000, medal ? "text01" : "text")}} {
        if (medal)
            definition_.controls.push_back(frontend::text_definition(0xfc200000, "text02"));
        background_ =
            scene_.control_frame(medal ? "description01" : "description", module_, false, model_.get());
        scene_.set_screen_visible(
            module_,
            scene_.control_frame(medal ? "description" : "description01", module_, false, model_.get()),
            false);
        scene_.position_menu_model(module_, background_, .5f, -.375f, -.5f);
        scene_.set_screen_visible(module_, background_, false);
        scene_.set_screen_visible(module_, model_.get(), true);
    }
    const frontend::MenuScreenDefinition &definition() const { return definition_; }
    void bind(MenuTextObject &text, MenuTextObject *second = nullptr) {
        text_ = &text;
        text_->bind_frame(
            scene_.control_frame(definition_.controls.front().frame, module_, false, model_.get()));
        text_->initialize_static_frame();
        text_->visibility_query([this] { return visible_; });
        second_ = second;
        if (second_) {
            second_->bind_frame(scene_.control_frame("text02", module_, false, model_.get()));
            second_->initialize_static_frame();
            second_->visibility_query([this] { return visible_; });
        }
    }
    void request(frontend::SharedText text) {
        if (!text_)
            throw std::logic_error("Description text is not bound");
        text_->object().clear();
        text_->object().append(std::move(text), 2);
        requested_ = true;
    }
    void request_medal(unsigned index) {
        request(text_->localize(0x2774 + index * 2));
        if (second_) {
            second_->object().clear();
            second_->object().append(second_->localize(0x2775 + index * 2), 3);
        }
    }
    void hide() {
        requested_ = false;
        visible_ = false;
        scene_.set_screen_visible(module_, background_, false);
    }
    void update(const MenuInputSession &input, bool primary) {
        const bool show = requested_ && primary && input.pointer.idle() >= 500 && !input.dispatch.dragging &&
                          !input.input.held_mask3;
        visible_ = show;
        scene_.set_screen_visible(module_, background_, show);
        if (show) {
            const auto &cursor = input.pointer.cursor();
            const auto position = scene_.frame_world_position(module_, background_);
            scene_.position_menu_model(module_, background_, cursor.x() / 800.f + .15f,
                                       -cursor.y() / 800.f + .015f, position[2]);
            scene_.update_screen_frame(module_, background_);
            text_->invalidate_bounds();
            text_->layout_text_on_frame();
            if (second_) {
                second_->invalidate_bounds();
                second_->layout_text_on_frame();
            }
        }
    }
    bool visible() const { return visible_; }

  private:
    MenuScene &scene_;
    HMODULE module_;
    MenuDriver::Owner model_;
    EngineFrame *background_ = nullptr;
    frontend::MenuScreenDefinition definition_;
    MenuTextObject *text_ = nullptr, *second_ = nullptr;
    bool requested_ = false, visible_ = false;
};
} // namespace hd2::host
#endif
