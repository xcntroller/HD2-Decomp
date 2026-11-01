#ifndef HD2_HOST_PRE_MISSION_TEAM_MENU_H
#define HD2_HOST_PRE_MISSION_TEAM_MENU_H

#include "host/pre_mission_controls.h"
#include "host/pre_mission_description.h"
#include "host/pre_mission_soldier_selection.h"
#include "host/frame_name.h"
#include "host/profiles_menu.h"
#include <deque>
#include <random>

namespace hd2::host {

// 00691CB0's team owner. Candidate indices remain HumanStore indices; the
// portrait strip is filtered independently of the ordered four live team slots.
class PreMissionTeamMenu {
  public:
    PreMissionTeamMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                       EngineFrame *model, frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), module_(module), model_(model), queue_(queue),
          descriptor_(startup.pre_mission().descriptor()), clone_{driver, module}, selection_(scene, driver, module, queue), random_(std::random_device{}()) {}
    void transitions(std::function<void()> advance, std::function<void()> back) {
        advance_ = std::move(advance);
        back_ = std::move(back);
    }
    frontend::MenuScreenDefinition definition() {
        using namespace frontend;
        MenuScreenDefinition out{"TEAM", 0x04000000, "team members selection", {}};
        MenuControlDefinition panel = group(0x04100000, "Panel01");
        auto *parent = scene_.control_frame("Panel01", module_, false, model_);
        auto *prototype = scene_.control_frame("photo03", module_, false, model_);
        auto *unavailable = scene_.control_frame("photo04", module_, false, model_);
        auto *strip = scene_.create_group_frame(module_, parent, "GB_Soldiers", {0, 0, 0});
        // 006920E8 rebases the prototype before 0069233A saves its local anchor.
        clone_.link(prototype, strip, 1);
        scene_.update_screen_frame(module_, prototype);
        std::array<float, 3> origin{};
        std::memcpy(origin.data(), reinterpret_cast<const char *>(prototype) + 0xb0, 12);
        auto list = group(0x04101000, "GB_Soldiers", MenuControlKind::movie_list);
        list.initialization = {{0, 0x0100000b, 0x04102000, 0, 0, {}},
                               {0, 0x01000028, 5, 0, 0, {}},
                               {0, 0x01000029, 0, 0x3db367a1, 0, {}},
                               {0, 0x0100006b, 0, 0, 0, std::make_shared<std::array<float, 3>>(origin)}};
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        for (unsigned i = 0; i < 30; ++i) {
            const auto *human = startup_.pre_mission_humans().record(i);
            if (!human)
                break;
            char name[32];
            std::snprintf(name, sizeof(name), "Soldier_%02u", i);
            names_.push_back(name);
            candidate_in_list_.push_back(true);
            auto frame = clone_menu_frame(clone_, (human->tail[1] & 255) ? unavailable : prototype, strip);
            cinematics_engine_method<Name>(frame.get(), module_, 0x34)(frame.get(), names_.back().c_str());
            scene_.set_screen_visible(module_, frame.get(), true);
            const unsigned id = candidate_id(i);
            auto image = group(id, names_.back().c_str(), MenuControlKind::image);
            image.initialization.push_back({0, 0x01000009, i, 0, 0, {}});
            image.initialization.push_back(
                PreMissionImage::texture_event(0, static_cast<EngineFrame *>(human->portrait)));
            observe_image(image, !(human->tail[1] & 255));
            list.children.push_back(std::move(image));
            frames_.push_back(std::move(frame));
        }
        // The original hides its authored, five-position strip after cloning.
        for (const char *name : {"photo03", "photo04", "photo05", "photo06", "photo07", "death"})
            scene_.set_screen_visible(module_, scene_.control_frame(name, module_, false, model_), false);
        selector_ = scene_.control_frame("selector", module_, false, model_);
        scene_.set_screen_visible(module_, selector_, false);
        panel.children.push_back(std::move(list));
        panel.children.push_back({0x04102000,
                                  "scroll00",
                                  MenuControlKind::slider,
                                  nullptr,
                                  nullptr,
                                  MenuControlRole::none,
                                  {{0, 0x0100000b, 0x04101000, 0, 0, {}}}});
        out.controls.push_back(std::move(panel));
        auto stats = group(0x04200000, "Panel02");
        stats.initialization.push_back({0, 0x01000005, 0, 0, 0, {}});
        for (unsigned i = 0; i < 7; ++i) {
            const unsigned id = 0x04200000 + (i * 3 + 1) * 0x1000;
            stats.children.push_back(text_definition(
                id, stat_names[i][0], {caption_definition(11, 0xa6e + i), {0, 0x02000073, 0, 0, 0, {}}}));
            stats.children.push_back({id + 0x1000,
                                      stat_names[i][1],
                                      MenuControlKind::meter,
                                      nullptr,
                                      nullptr,
                                      MenuControlRole::none,
                                      {{0, 0x01000093, 2, 0, 0, {}}, {0, 0x01000092, 0, 0, 0, {}}}});
            stats.children.push_back(
                text_definition(id + 0x2000, stat_names[i][2],
                                {caption_definition(11, 0x7d2), {0, 0x02000073, 0, 0, 0, {}}}));
        }
        out.controls.push_back(std::move(stats));
        auto biography = group(0x04300000, "Panel03");
        biography.initialization.push_back({0, 0x01000005, 0, 0, 0, {}});
        biography.initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
        biography.children.push_back(text_definition(0x04301000, "text_05", {caption_definition(1, 0x7d2)}));
        out.controls.push_back(std::move(biography));
        auto *medals = scene_.create_group_frame(module_, model_, "medals_panel", {0, 0, 0});
        auto medal_group = group(0x04400000, "medals_panel");
        medal_group.initialization.push_back({0, 0x01000005, 0, 0, 0, {}});
        for (unsigned i = 0; i < medal_names.size(); ++i) {
            auto *f = scene_.control_frame(medal_names[i], module_, false, model_);
            clone_.link(f, medals, 1);
            medal_frames_[i] = f;
            auto medal = group(0x04400000 + (i + 1) * 0x1000, medal_names[i], MenuControlKind::image);
            medal.initialization = {{0, 0x01000009, i % 12, 0, 0, {}},
                                    observer_control_event(0, 0x04003000, this, &medal_hover),
                                    observer_control_event(0, 0x04004000, this, &medal_hover)};
            medal_group.children.push_back(std::move(medal));
        }
        out.controls.push_back(std::move(medal_group));
        out.controls.push_back({0x04500000,
                                "bnamexx",
                                MenuControlKind::edit,
                                "normal*",
                                "actived*",
                                MenuControlRole::none,
                                {caption_definition(12, 0x7d2),
                                 {0, 0x02000073, 1, 0, 0, {}}, //00691CB0 overrides the edit's inset.
                                 {0, 0x0100006e, 17, 0, 0, {}},
                                 {0, 0x01000001, 0, 0, 0, {}},
                                 observer_control_event(0, 0x04016000, this, &rename)}});
        out.controls.push_back(button(0x04600000, "bselected soldier info", 0xa69));
        out.controls.push_back(
            text_definition(0x04700000, "text_selected soldier stats", {caption_definition(11, 0xa6b)}));
        auto *selected = scene_.create_group_frame(module_, model_, "P_SelectedSol", {0, 0, 0});
        auto squad = group(0x04800000, "P_SelectedSol");
        for (unsigned slot = 0; slot < 4; ++slot) {
            for (unsigned part = 0; part < 3; ++part) {
                auto *f = scene_.control_frame(slot_names[slot][part], module_, false, model_);
                clone_.link(f, selected, 1);
                const unsigned id = 0x04800000 + (slot * 3 + part + 1) * 0x1000;
                if (part < 2)
                    squad.children.push_back(
                        text_definition(id, slot_names[slot][part], {caption_definition(11, 0x7d2)}));
                else {
                    slot_frames_[slot] = f;
                    auto image = group(id, slot_names[slot][part], MenuControlKind::image);
                    image.initialization = {{0, 0x01000009, 0xffffffff, 0, 0, {}},
                                            PreMissionImage::texture_event(0, nullptr)};
                    observe_image(image, true);
                    squad.children.push_back(std::move(image));
                }
            }
        }
        out.controls.push_back(std::move(squad));
        out.controls.push_back(button(0x04900000, "bassign - detach", 0xa61));
        out.controls.push_back(button(0x04a00000, "baccept and continue", 0xa5e));
        out.controls.push_back(button(0x04b00000, "bauto assign", 0xa5d));
        out.controls.push_back(button(0x04c00000, "bback", 0xa5f));
        out.controls.push_back(text_definition(
            0x04d00000, "text_screen",
            {caption_definition(10, startup_.pre_mission().descriptor()->word10 == 2 ? 0xa5b : 0xa5c)}));
        out.controls.push_back(
            text_definition(0x04e00000, "text_h&dversion", {caption_definition(40, 0x7d6)}));
        out.controls.push_back(
            text_definition(0x04f00000, "text_military service draft", {caption_definition(11, 0xa63)}));
        out.initialization.push_back(observer_control_event(0, 0x0400c000, this, &hidden));
        return out;
    }
    void description(std::shared_ptr<PreMissionDescription> value) { description_ = std::move(value); }
    PreMissionSoldierSelection &selection() { return selection_; }
    std::optional<MenuRectangle> drag_rectangle() const {
        if (!drag_frame_) return std::nullopt;
        const auto& cursor=context_->input.pointer.cursor();
        const float x=cursor.x()-drag_pointer_[0],y=cursor.y()-drag_pointer_[1];
        return MenuRectangle{drag_rectangle_.left+x,drag_rectangle_.top+y,
                             drag_rectangle_.right+x,drag_rectangle_.bottom+y};
    }
    bool receive(frontend::ControlEvent &event) {
        switch (event.code & 0x03000fffu) {
        case 0x0200000a:
            if (queue_.screens().current() && queue_.screens().current()->identifier == 0x08000000)
                queue_.enqueue({0, 0x02000005, 0, 0, 0, {}});
            else
                back_();
            return true;
        case 0x0200006a:
            startup_.pre_mission().team().reorder(0, selection_.selected());
            queue_.enqueue({0, 0x02000008, 0, 0, 0, {}});
            startup_.pre_mission().accept_team();
            return true;
        default:
            return false;
        }
    }
    void shown() {
        // The selection dialog pops before queued02000008 clears the stack.
        // Acceptance may already have advanced the borrowed descriptor owner.
        if (startup_.pre_mission().descriptor() != descriptor_)
            return;
        refresh();
        if (selected_ < 0)
            select_candidate(0); //00691280 ->00694AE0(0).
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &d, EngineFrame *frame,
                                              MenuTextObject &text, MenuWidgetContext &context,
                                              std::function<MenuTextObject *(unsigned)> lookup) {
        context_ = &context;
        using K = frontend::MenuControlKind;
        auto extra = [this, &context, lookup](const frontend::MenuControlDefinition &child, EngineFrame *f) {
            return make_control(child, f, *lookup(child.identifier), context, lookup);
        };
        if (d.kind == K::image) {
            const bool member=(d.identifier&0xfffff000u)==0x04101000u ||
                              (d.identifier&0xfff00000u)==0x04800000u;
            return std::make_unique<PreMissionImage>(scene_, module_, d, frame, text, context, !member);
        }
        if (d.kind == K::meter)
            return std::make_unique<PreMissionMeter>(scene_, module_, d, frame, text, context.queue);
        if (d.kind == K::edit) {
            auto edit = std::make_unique<ProfileEdit>(scene_, module_, d, frame, text, context);
            edit_ = edit.get();
            return edit;
        }
        if (d.kind == K::movie_list)
            return std::make_unique<PreMissionList>(scene_, module_, d, frame, text, context, lookup, extra);
        if (d.kind == K::container)
            return std::make_unique<OptionsGroup>(scene_, module_, d, frame, text, context, lookup, extra);
        return {};
    }

  private:
    static frontend::MenuControlDefinition
    group(unsigned id, const char *name,
          frontend::MenuControlKind kind = frontend::MenuControlKind::container) {
        return {id, name, kind, nullptr, nullptr, frontend::MenuControlRole::none, {}};
    }
    frontend::MenuControlDefinition button(unsigned id, const char *name, unsigned label) {
        return frontend::button_definition(id, name, "normal*", "actived*",
                                           frontend::MenuControlRole::single_player,
                                           {frontend::caption_definition(13, label),
                                            frontend::observer_control_event(0, 0x04001000, this, &action)});
    }
    void observe_image(frontend::MenuControlDefinition &image, bool selectable) {
        image.initialization.push_back(frontend::observer_control_event(0, 0x04001000, this, &select));
        if (selectable) {
            image.initialization.push_back(
                frontend::observer_control_event(0, 0x04013000, this, &double_click));
            image.initialization.push_back(frontend::observer_control_event(0, 0x0400a000, this, &drag));
            image.initialization.push_back(frontend::observer_control_event(0, 0x04008000, this, &drag));
            image.initialization.push_back(frontend::observer_control_event(0, 0x04009000, this, &drag));
        }
    }
    static unsigned candidate_id(unsigned i) { return 0x04101000 + (i + 1) * 16; }
    static unsigned slot_image_id(unsigned slot) { return 0x04800000 + (slot * 3 + 3) * 0x1000; }
    void raw(unsigned id, unsigned style, const std::string &value, bool immediate=false) {
        frontend::ControlEvent e{id, 0x01000014, style, 0, 0, {}};
        e.text = std::make_shared<const std::string>(value);
        if (immediate) queue_.dispatch_immediate(std::move(e));
        else queue_.enqueue(e);
    }
    void refresh(bool immediate=false) {
        const auto send=[this,immediate](frontend::ControlEvent event) {
            if (immediate) queue_.dispatch_immediate(std::move(event));
            else queue_.enqueue(event);
        };
        auto &team = startup_.pre_mission().team();
        //00693C10/00694C40 edit the active list incrementally. Clearing it
        // sends02000085 and resets the scroll offset on every transfer.
        for (unsigned i = 0; i < names_.size(); ++i) {
            const bool included = !team.contains(i);
            if (included != candidate_in_list_[i]) {
                send({0x04101000, included ? 0x02000083u : 0x02000084u,
                                candidate_id(i), 0, 0, {}});
                candidate_in_list_[i] = included;
            }
        }
        for (unsigned i = 0; i < 4; ++i) {
            const auto &slot = team.slots()[i];
            const auto *human = startup_.pre_mission_humans().record(slot.human);
            const unsigned base = 0x04800000 + (i * 3 + 1) * 0x1000;
            //00690DE0: style12 name and style2 rank from HumanStore+24.
            raw(base, 12, human ? display_name(*human) : "",immediate);
            if (human) {
                if (human->field24 >= 8)
                    throw std::runtime_error("Invalid HumanStore rank index");
                // Use the full rank from the original pair, as in the user reference.
                send({base + 0x1000, 0x01000015, 2, 0x2792 + human->field24, 0, {}});
            } else
                raw(base + 0x1000, 11, "",immediate);
            send({base + 0x2000, 0x01000009, static_cast<unsigned>(slot.human), 0, 0, {}});
            send(PreMissionImage::texture_event(
                base + 0x2000, human ? static_cast<EngineFrame *>(human->portrait) : nullptr));
        }
        send({0x04b00000, team.size() ? 0x01000002u : 0x01000001u, 0, 0, 0, {}});
        refresh_selected();
    }
    static const std::string &display_name(const frontend::HumanRecord &human) {
        return human.user_name.empty() ? human.full_name : human.user_name;
    }
    void refresh_selected() {
        const auto *human = startup_.pre_mission_humans().record(selected_);
        constexpr std::array<unsigned, 7> fields{{5, 2, 1, 3, 7, 8, 9}};
        for (unsigned i = 0; i < 7; ++i) {
            const float value =
                human ? frontend::pre_mission_detail::Bytes::real(human->stats[fields[i]]) : 0.f;
            if (!std::isfinite(value))
                throw std::runtime_error("Invalid selected soldier statistic");
            const float ratio = i == 2 && human ? 1.f : value * .01f;
            unsigned word = 0;
            std::memcpy(&word, &ratio, 4);
            queue_.enqueue({0x04200000 + (i * 3 + 2) * 0x1000, 0x01000092, 0, word, 0, {}});
            char number[40];
            std::snprintf(number, sizeof(number),
                          i == 0 || i >= 4 ? "%3.0f %%"
                          : i == 1         ? "%3.0f Kg"
                                           : "%3.0f",
                          value);
            raw(0x04200000 + (i * 3 + 3) * 0x1000, 23, human ? number : "");
        }
        queue_.enqueue({0x04301000, 0x01000015, 1, human ? human->stats[0] : 0x7d2, 0, {}});
        queue_.enqueue({0x04500000, human ? 0x01000002u : 0x01000001u, 0, 0, 0, {}});
        raw(0x04500000, 12, human ? display_name(*human) : "");
        queue_.enqueue({0x04900000,
                        0x01000015,
                        12,
                        startup_.pre_mission().team().contains(selected_) ? 0xa62u : 0xa61u,
                        0,
                        {}});
        // 00696740 hides all medals, then places only awarded IDs. When
        // there are more than six, earlier awards use the compact b variant.
        for (auto *frame : medal_frames_)
            scene_.set_screen_visible(module_, frame, false);
        if (human)
            for (std::size_t i = 0; i < human->items.size(); ++i) {
                const unsigned medal = human->items[i];
                if (medal >= 12)
                    throw std::runtime_error("Invalid HumanStore medal index");
                const bool compact = human->items.size() - i > 6;
                char anchor_name[32];
                std::snprintf(
                    anchor_name, sizeof(anchor_name), "medal_%02u",
                    static_cast<unsigned>(compact ? i + 1 : i + (human->items.size() <= 6 ? 7 : 1)));
                auto *anchor = scene_.control_frame(anchor_name, module_, false, model_);
                auto *frame = medal_frames_[medal + (compact ? 12 : 0)];
                clone_.link(frame, anchor, 1);
                scene_.set_frame_local_position(frame, {0, 0, 0});
                scene_.set_screen_visible(module_, frame, true);
                scene_.update_screen_frame(module_, frame);
            }
    }
    void position_selector(EngineFrame *frame) {
        clone_.link(selector_, frame, 0);
        scene_.set_frame_local_position(selector_, {0, 0, -.1f});
        scene_.set_screen_visible(module_, selector_, true);
        scene_.update_screen_frame(module_, selector_);
    }
    void select_candidate(unsigned start) {
        const auto &team = startup_.pre_mission().team();
        for (unsigned i = start; i < frames_.size(); ++i)
            if (!team.contains(i)) {
                selected_ = static_cast<int>(i);
                refresh_selected();
                position_selector(frames_[i].get());
                return;
            }
    }
    static bool select(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        auto &image = *static_cast<PreMissionImage *>(sender);
        self.selected_ = frontend::widget_signed_word(image.user_data());
        self.refresh_selected();
        self.position_selector(image.identifier() < 0x04800000
                                   ? self.frames_.at((image.identifier() - 0x04101010) / 16).get()
                                   : self.slot_frames_.at((image.identifier() - 0x04803000) / 0x3000));
        return true;
    }
    static bool double_click(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        const auto human = frontend::widget_signed_word(static_cast<PreMissionImage *>(sender)->user_data());
        if (human != self.selected_)
            return true;
        self.startup_.pre_mission().team().toggle(human, self.startup_.pre_mission_humans());
        self.scene_.set_screen_visible(self.module_, self.selector_, false);
        self.refresh();
        return true;
    }
    static bool drag(void *owner, void *, void *sender, unsigned tag, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        auto &image = *static_cast<PreMissionImage *>(sender);
        if (tag == 0x0400a000) {
            const int human = frontend::widget_signed_word(image.user_data());
            if (human < 0)
                return false;
            //006937F0 selects the source before creating the moving portrait.
            select(owner, nullptr, sender, 0, 0);
            self.drag_human_ = human;
            self.drag_from_squad_ = image.identifier() >= 0x04800000;
            auto *source = self.drag_from_squad_
                               ? self.slot_frames_.at((image.identifier() - 0x04803000) / 0x3000)
                               : self.frames_.at((image.identifier() - 0x04101010) / 16).get();
            const char* name=engine_frame_name(source);
            self.drag_rectangle_=menu_rectangle(self.scene_.label_bounds(name,name,self.module_,false,source),800,600);
            //006937F0 makes a shallow visual copy, leaving selector children
            //and control IDs behind. The original source is hidden until drop.
            self.drag_frame_ = self.clone_.create_like(source);
            self.clone_.copy(self.drag_frame_.get(), source);
            self.clone_.link(self.drag_frame_.get(), self.model_, 0);
            self.drag_position_ = self.scene_.frame_world_position(self.module_, source);
            self.drag_pointer_ = {self.context_->input.pointer.cursor().x(),
                                  self.context_->input.pointer.cursor().y()};
            self.drag_source_ = source;
            self.scene_.set_screen_visible(self.module_, source, false);
            unsigned no_target = 0;
            std::memcpy(reinterpret_cast<char *>(self.drag_frame_.get()) + 0xec, &no_target, 4);
            self.scene_.set_screen_visible(self.module_, self.drag_frame_.get(), true);
            self.move_drag();
            return true;
        }
        if (tag == 0x04008000) {
            self.move_drag();
            return true;
        }
        self.drag_frame_.reset();
        if (self.drag_source_) {
            self.scene_.set_screen_visible(self.module_, self.drag_source_, true);
            self.drag_source_ = nullptr;
        }
        const int human = std::exchange(self.drag_human_, -1);
        if (human < 0)
            return false;
        const auto &cursor = self.context_->input.pointer.cursor();
        const unsigned target =
            self.scene_.pick_model(self.module_, cursor.x(), cursor.y(), 800, 600, self.model_);
        auto &team = self.startup_.pre_mission().team();
        bool reordered=false;
        //00693C10 uses normalized X=.5 for transfers between the two halves;
        // a squad control pick is needed only for swapping existing members.
        if (self.drag_from_squad_ && (target & 0x03f00000u) == 0x00800000u) {
            std::size_t source = 0;
            while (source < team.size() && team.slots()[source].human != human)
                ++source;
            const auto destination = (target - 0x04801000u) / 0x3000u;
            team.reorder(source, destination);
            reordered=true;
        } else if ((!self.drag_from_squad_ && cursor.x() >= 400.f) ||
                   (self.drag_from_squad_ && cursor.x() <= 400.f)) {
            team.toggle(human, self.startup_.pre_mission_humans());
        }
        //The outline returns at the source position, including after a roster
        //soldier enters the squad. Detach before refresh hides/removes that
        //roster frame, preserving its authored world position.
        const bool assigned=!self.drag_from_squad_ && team.contains(human);
        if (assigned) self.clone_.link(self.selector_,self.model_,1);
        const bool retained=assigned || (!reordered && team.contains(human)==self.drag_from_squad_);
        self.scene_.set_screen_visible(self.module_, self.selector_, retained);
        //Drop runs after0066BEA0's only queue drain. Commit the roster and
        //squad visuals together before rendering this frame.
        self.refresh(true);
        return true;
    }
    void move_drag() {
        if (!drag_frame_)
            return;
        const auto &cursor = context_->input.pointer.cursor();
        //00693B60 adds cursor displacement to X/Y, but REPLACES Z with
        //0066A150's -1. Keeping the source depth puts the drag behind borders.
        const auto displacement = frontend::cursor_frame_position(
            cursor.x() - drag_pointer_[0], cursor.y() - drag_pointer_[1], 800, 600);
        scene_.position_menu_model(module_, drag_frame_.get(),
                                   drag_position_[0] + displacement[0],
                                   drag_position_[1] + displacement[1], displacement[2]);
        scene_.update_screen_frame(module_, drag_frame_.get());
    }
    static bool medal_hover(void *owner, void *, void *sender, unsigned tag, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        if (self.description_) {
            if (tag == 0x04004000)
                self.description_->hide();
            else
                self.description_->request_medal(static_cast<PreMissionImage *>(sender)->user_data());
        }
        return true;
    }
    static bool hidden(void *owner, void *, void *, unsigned, unsigned) {
        static_cast<PreMissionTeamMenu *>(owner)->startup_.save_pre_mission_names();
        return true;
    }
    static bool rename(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        if (auto *human = self.startup_.pre_mission_humans().record(self.selected_)) {
            human->user_name = self.edit_->value();
            frontend::finalize_human_text(*human, self.startup_.wide_characters());
            self.refresh();
        }
        return true;
    }
    static bool action(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionTeamMenu *>(owner);
        const auto id = static_cast<ButtonVisualPair *>(sender)->identifier();
        if (id == 0x04900000) {
            self.startup_.pre_mission().team().toggle(self.selected_, self.startup_.pre_mission_humans());
            self.scene_.set_screen_visible(self.module_, self.selector_, false);
            self.refresh();
        } else if (id == 0x04a00000) {
            self.startup_.pre_mission().auto_assign([&self] { return self.random_(); });
            self.refresh();
        } else if (id == 0x04b00000) {
            const auto &route = self.startup_.pre_mission().route();
            if (route.root_index() == 0 && route.campaign_index() == std::optional<std::uint32_t>(0) &&
                route.game_mode() != 2) {
                if (!self.startup_.pre_mission().team().size())
                    return true;
                std::vector<const frontend::HumanRecord *> humans;
                for (const auto &slot : self.startup_.pre_mission().team().slots())
                    if (slot.human >= 0)
                        humans.push_back(self.startup_.pre_mission_humans().record(slot.human));
                self.selection_.open(humans);
            } else {
                self.startup_.pre_mission().accept_team();
                self.advance_();
            }
        } else if (id == 0x04c00000)
            self.back_();
        else if (id == 0x04600000) {
            self.biography_ = !self.biography_;
            self.queue_.enqueue({0x04200000, self.biography_ ? 0x01000004u : 0x01000003u, 0, 0, 0, {}});
            self.queue_.enqueue({0x04300000, self.biography_ ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
            self.queue_.enqueue({0x04600000, 0x01000015, 12, self.biography_ ? 0xa6au : 0xa69u, 0, {}});
            self.queue_.enqueue({0x04700000, 0x01000015, 11, self.biography_ ? 0xa6cu : 0xa6bu, 0, {}});
        }
        return true;
    }
    inline static constexpr const char *stat_names[7][3] = {
        {"text_shooting", "hlcpct00", "text_shooting01"}, {"text_strenght", "hlcpct01", "text_strenght01"},
        {"text_health", "hlcpct02", "text_health01"},     {"text_endurance", "hlcpct03", "text_endurance01"},
        {"text_stealth", "hlcpct04", "text_stealth01"},   {"text_first aid", "hlcpct05", "text_first aid01"},
        {"text_lockpick", "hlcpct06", "text_lockpick01"}};
    inline static constexpr const char *slot_names[4][3] = {{"text_name00", "text_team member00", "member00"},
                                                            {"text_name01", "text_team member01", "photo00"},
                                                            {"text_name02", "text_team member02", "photo01"},
                                                            {"text_name03", "text_team member03", "photo02"}};
    inline static constexpr std::array<const char *, 24> medal_names{{"DSO_a",
                                                                      "DCM_a",
                                                                      "MILIT_MEDAL_a",
                                                                      "GSM_AFRICA_a",
                                                                      "GSM_a",
                                                                      "WAR_MEDAL_a",
                                                                      "DEFENCE_MEDAL_a",
                                                                      "MILITARY_CROSS_a",
                                                                      "VICTORIA_CROSS_a",
                                                                      "GERM_FRANCE_STAR_a",
                                                                      "AFRICA_STAR_a",
                                                                      "BURMA_STAR_a",
                                                                      "DSO_b",
                                                                      "DCM_b",
                                                                      "MILIT_MEDAL_b",
                                                                      "GSM_AFRICA_b",
                                                                      "GSM_b",
                                                                      "WAR_MEDAL_b",
                                                                      "DEFENCE_MEDAL_b",
                                                                      "MILITARY_CROSS_b",
                                                                      "VICTORIA_CROSS_b",
                                                                      "GERM_FRANCE_STAR_b",
                                                                      "AFRICA_STAR_b",
                                                                      "BURMA_STAR_b"}};
    StartupSession &startup_;
    MenuScene &scene_;
    HMODULE module_;
    EngineFrame *model_;
    frontend::MenuEventQueue &queue_;
    const frontend::GameDataNode *descriptor_;
    CinematicsCloneBackend clone_;
    PreMissionSoldierSelection selection_;
    std::deque<std::string> names_;
    std::vector<bool> candidate_in_list_;
    MenuDriver::Owner drag_frame_{nullptr, nullptr};
    EngineFrame *drag_source_ = nullptr;
    std::array<float, 3> drag_position_{};
    std::array<float, 2> drag_pointer_{};
    MenuRectangle drag_rectangle_{};
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> frames_;
    std::array<EngineFrame *, 4> slot_frames_{};
    std::array<EngineFrame *, 24> medal_frames_{};
    std::shared_ptr<PreMissionDescription> description_;
    EngineFrame *selector_ = nullptr;
    ProfileEdit *edit_ = nullptr;
    MenuWidgetContext *context_ = nullptr;
    int drag_human_ = -1;
    bool drag_from_squad_ = false;
    std::mt19937 random_;
    std::function<void()> advance_, back_;
    int selected_ = -1;
    bool biography_ = false;
};
} // namespace hd2::host
#endif
