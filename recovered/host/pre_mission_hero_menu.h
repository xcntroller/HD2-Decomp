#ifndef HD2_HOST_PRE_MISSION_HERO_MENU_H
#define HD2_HOST_PRE_MISSION_HERO_MENU_H

#include "host/pre_mission_controls.h"

namespace hd2::host {

// 00697600/00697710/00697A50/00697F60's actual hero model augmentation.
// The runtime supplies its named confirmation and controller status callbacks;
// this content owner shares the live preparation session with the next screen.
class PreMissionHeroMenu {
  public:
    PreMissionHeroMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                       EngineFrame *model, frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), module_(module), model_(model), queue_(queue),
          clone_{driver, module} {}
    void transitions(std::function<void()> advance, std::function<void()> back) {
        advance_ = std::move(advance);
        back_ = std::move(back);
    }
    frontend::MenuScreenDefinition definition() {
        if (built_)
            throw std::logic_error("Hero model augmented twice");
        built_ = true;
        using namespace frontend;
        MenuScreenDefinition result{"HERO STATS", 0x04000000, "heros_screen", {}};
        result.controls.push_back(
            text_definition(0x04100000, "text_screen", {caption_definition(10, 0xa29)}));
        result.controls.push_back(
            text_definition(0x04200000, "text_specialization", {caption_definition(11, 0xa2d)}));
        MenuControlDefinition counter{0x04300000,
                                      "skill00",
                                      MenuControlKind::hero_stat,
                                      nullptr,
                                      nullptr,
                                      MenuControlRole::none,
                                      {caption_definition(11, 0xa2e)}};
        counter.children.push_back(text_definition(0x04300001, "skill_value*"));
        result.controls.push_back(std::move(counter));
        auto *specializations = scene_.create_group_frame(module_, model_, "GRSkillSpec", {0, 0, 0});
        for (const auto *name : {"bsoldier", "bheavysoldier", "bsniper", "bmedic", "bengineer"}) {
            auto *frame = scene_.control_frame(name, module_, false, model_);
            clone_.link(frame, specializations, 1);
        }
        result.controls.push_back({0x04400000,
                                   "GRSkillSpec",
                                   MenuControlKind::binding_list,
                                   nullptr,
                                   nullptr,
                                   MenuControlRole::none,
                                   {{0, 0x01000028, 20, 0, 0, {}}}});
        auto *prototype = scene_.control_frame("skill01", module_, false, model_);
        auto *group = scene_.create_group_frame(module_, model_, "GRSkills", {0, 0, 0});
        clone_.link(prototype, group, 1);
        std::array<float, 3> origin{};
        std::memcpy(origin.data(), reinterpret_cast<const char *>(prototype) + 0xb0, 12);
        scene_.set_screen_visible(module_, prototype, false);
        MenuControlDefinition skills{0x04500000, "GRSkills", MenuControlKind::binding_list,
                                     nullptr,    nullptr,    MenuControlRole::none,
                                     {}};
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        for (std::size_t row = 0; row < names.size(); ++row) {
            auto frame = clone_menu_frame(clone_, prototype, group);
            cinematics_engine_method<Name>(frame.get(), module_, 0x34)(frame.get(), names[row]);
            auto position = origin;
            position[1] -= static_cast<float>(row) * .02f; // 00697DA2 immediate3CA3D70A.
            scene_.set_frame_local_position(frame.get(), position);
            scene_.update_screen_frame(module_, frame.get());
            scene_.set_screen_visible(module_, frame.get(), true);
            const auto id = 0x04500000u | static_cast<unsigned>((row + 1) * 0x1000);
            ControlEvent caption{0, 0x01000014, 11, 0, 0, {}};
            caption.text = std::make_shared<const std::string>(names[row]);
            MenuControlDefinition stat{
                id,
                names[row],
                MenuControlKind::hero_stat,
                nullptr,
                nullptr,
                MenuControlRole::none,
                {caption,
                 {0, 0x0100000b, 0x04300000, 0, 0, {}},
                 {0, 0x01000072, static_cast<unsigned>(PreMissionHero::bases[row]), 0, 0, {}}}};
            stat.children.push_back(text_definition(id | 1, "skill_value*"));
            skills.children.push_back(std::move(stat));
            frames_.push_back(std::move(frame));
        }
        result.controls.push_back(std::move(skills));
        result.controls.push_back(button_definition(
            0x04600000, "baccept", "normal*", "actived*", MenuControlRole::single_player,
            {caption_definition(13, 0xa2a), observer_control_event(0, 0x04001000, this, &accept)}));
        result.controls.push_back(button_definition(
            0x04700000, "bback", "normal*", "actived*", MenuControlRole::single_player,
            {caption_definition(13, 0xa2b), observer_control_event(0, 0x04001000, this, &back)}));
        return result;
    }
    void shown() {
        if (!counter_initialized_) {
            queue_.enqueue({0x04300000, 0x01000073, 20, 0, 0, {}});
            counter_initialized_ = true;
        }
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &definition,
                                              EngineFrame *frame, MenuTextObject &text,
                                              MenuWidgetContext &widgets,
                                              std::function<MenuTextObject *(unsigned)> lookup) {
        using namespace frontend;
        if (definition.kind == MenuControlKind::hero_stat)
            return std::make_unique<PreMissionStat>(scene_, module_, definition, frame, text,
                                                    *lookup(definition.identifier | 1), widgets);
        if (definition.kind == MenuControlKind::binding_list)
            return std::make_unique<PreMissionList>(
                scene_, module_, definition, frame, text, widgets, lookup,
                [this, &widgets, lookup](const MenuControlDefinition &child, EngineFrame *child_frame) {
                    return make_control(child, child_frame, *lookup(child.identifier), widgets, lookup);
                });
        return {};
    }

  private:
    static bool accept(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionHeroMenu *>(owner);
        std::array<std::int32_t, 18> values{};
        for (std::size_t row = 0; row < values.size(); ++row) {
            frontend::ControlEvent query{
                0x04500000u | static_cast<unsigned>((row + 1) * 0x1000), 0x01000079, 0, 0, 0, {}};
            self.queue_.query_control(query);
            values[row] = frontend::widget_signed_word(query.argument);
        }
        self.startup_.pre_mission().hero().values(values);
        self.startup_.pre_mission().accept_hero();
        if (!self.advance_)
            throw std::logic_error("Hero controller continuation is unbound");
        self.advance_();
        return true;
    }
    static bool back(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionHeroMenu *>(owner);
        if (!self.back_)
            throw std::logic_error("Hero named confirmation is unbound");
        self.back_();
        return true;
    }
    inline static constexpr std::array<const char *, 18> names{
        {"Health", "Might", "Reactions", "Inteligence", "Endurance", "Stealth", "Light Weapons",
         "Heavy Weapons", "Shooting Rifle", "Throwing", "Morale", "Driving", "Swimming", "Traps", "First Aid",
         "Pick Lock", "Pilot", "German"}};
    StartupSession &startup_;
    MenuScene &scene_;
    HMODULE module_;
    EngineFrame *model_;
    frontend::MenuEventQueue &queue_;
    CinematicsCloneBackend clone_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> frames_;
    std::function<void()> advance_, back_;
    bool built_ = false, counter_initialized_ = false;
};

} // namespace hd2::host
#endif
