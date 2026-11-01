#ifndef HD2_HOST_PRE_MISSION_EQUIPMENT_MENU_H
#define HD2_HOST_PRE_MISSION_EQUIPMENT_MENU_H
#include "host/pre_mission_choice.h"
#include "host/pre_mission_description.h"
#include "host/pre_mission_item_row.h"
#include "host/pre_mission_soldier_selection.h"
#include <deque>

namespace hd2::host {

// 00674560/006763B0/00676D10/006761A0/006755F0's equipment content.
// All transfers mutate StartupSession's live stock/soldiers. The model frames,
// icons and callbacks are owned until the derived application state shuts down.
class PreMissionEquipmentMenu {
  public:
    PreMissionEquipmentMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                            EngineFrame *model, frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), module_(module), model_(model), queue_(queue),
          clone_{driver, module}, icons_(module), selection_(scene, driver, module, queue) {}
    void transitions(std::function<void()> advance, std::function<void()> back,
                     std::function<void(unsigned, frontend::SharedText, unsigned, unsigned)> prompt) {
        advance_ = std::move(advance);
        back_ = std::move(back);
        prompt_ = std::move(prompt);
    }
    frontend::MenuScreenDefinition definition() {
        using namespace frontend;
        MenuScreenDefinition out{"ITEMS", 0x04000000, "items selection", {}};
        auto *equipment = scene_.control_frame("equippanel", module_, false, model_);
        name_frame(equipment, "P_Equipment");
        auto panel = group(0x04100000, "P_Equipment");
        for (unsigned i = 0; i < 5; ++i) {
            auto *frame = scene_.control_frame(slot_names[i], module_, false, model_);
            clone_.link(frame, equipment, 1);
            slot_frames_[i] = frame;
            panel.children.push_back(image(0x04101000 + i * 0x1000, slot_names[i]));
        }
        auto *drop = scene_.control_frame("drop", module_, false, model_);
        clone_.link(drop, equipment, 1);
        panel.children.push_back(image(0x04106000, "drop"));
        for (unsigned carried = 0; carried < 2; ++carried) {
            const auto scroll_id = 0x04107000 + carried * 0x2000;
            auto scroll = group(scroll_id, carried ? "scroll14" : "scroll13");
            auto *scroll_frame = scene_.control_frame(scroll.frame, module_, false, model_);
            clone_.link(scroll_frame, equipment, 1);
            scroll.children.push_back({scroll_id | 0x10,
                                       carried ? "hlcpct01" : "hlcpct00",
                                       MenuControlKind::meter,
                                       nullptr,
                                       nullptr,
                                       MenuControlRole::none,
                                       {{0, 0x01000093, 2, 0, 0, {}}}});
            scroll.children.push_back(button(scroll_id | 0x20, carried ? "barrow up24" : "barrow up23", 0));
            scroll.children.push_back(
                button(scroll_id | 0x30, carried ? "barrow down24" : "barrow down23", 0));
            scroll.children.push_back(
                text_definition(scroll_id | 0x40, carried ? "text_bkpckcap" : "text_hlstrscap"));
            panel.children.push_back(std::move(scroll));
            const char *name = carried ? "GR_BackPack" : "GB_Holsters";
            auto *group_frame = scene_.create_group_frame(module_, equipment, name, {0, 0, 0});
            auto list = group(scroll_id + 0x1000, name, MenuControlKind::binding_list);
            list.initialization.push_back({0, 0x01000028, 4, 0, 0, {}});
            for (unsigned i = 0; i < 4; ++i) {
                const auto id = scroll_id + 0x1000 + (i + 1) * 16;
                auto *frame =
                    scene_.control_frame(carried ? pack_names[i] : holster_names[i], module_, false, model_);
                clone_.link(frame, group_frame, 1);
                list.children.push_back(image(id, carried ? pack_names[i] : holster_names[i]));
            }
            panel.children.push_back(std::move(list));
        }
        out.controls.push_back(std::move(panel));
        out.controls.push_back(text_definition(0x04200000, "name- capacity"));
        auto *soldiers = scene_.create_group_frame(module_, model_, "GB_Soldiers", {0, 0, 0});
        auto squad = group(0x04300000, "GB_Soldiers", MenuControlKind::binding_list);
        squad.initialization = {{0, 0x01000028, 4, 0, 0, {}}, {0, 0x01000029, 0, 0x3dd91687, 0, {}}};
        auto *portrait = scene_.control_frame("photo03", module_, false, model_);
        for (unsigned i = 0; i < startup_.pre_mission().squad().size(); ++i) {
            char name[32];
            std::snprintf(name, sizeof(name), "Soldier_%02u", i);
            names_.emplace_back(name);
            auto frame = clone_menu_frame(clone_, portrait, soldiers);
            name_frame(frame.get(), names_.back().c_str());
            const auto &soldier = startup_.pre_mission().squad()[i];
            const auto *human = startup_.pre_mission_humans().record(soldier.human);
            auto d = image(0x04300000 + (i + 1) * 0x1000, names_.back().c_str());
            d.initialization = {{0, 0x01000009, i, 0, 0, {}},
                                PreMissionImage::texture_event(
                                    0, human ? static_cast<EngineFrame *>(human->portrait) : nullptr),
                                observer_control_event(0, 0x04001000, this, &select_soldier),
                                observer_control_event(0, 0x04003000, this, &soldier_hover),
                                observer_control_event(0, 0x04004000, this, &soldier_hover)};
            squad.children.push_back(std::move(d));
            frames_.push_back(std::move(frame));
        }
        out.controls.push_back(std::move(squad));
        for (const char *name : {"photo03", "photo04", "photo05", "photo06", "photo07", "table11", "table13",
                                 "table14", "table15"})
            if (auto *frame = scene_.find_control_frame(model_, name, module_))
                scene_.set_screen_visible(module_, frame, false);
        out.controls.push_back(store_definition());
        auto stats = group(0x04500000, "Panel02");
        stats.initialization.push_back({0, 0x01000005, 0, 0, 0, {}});
        stats.initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
        for (unsigned i = 0; i < 7; ++i) {
            const auto id = 0x04500000 + (i * 3 + 1) * 0x1000;
            stats.children.push_back(
                text_definition(id, stat_names[i][0], {caption_definition(11, 0xa6e + i)}));
            stats.children.push_back({id + 0x1000,
                                      stat_names[i][1],
                                      MenuControlKind::meter,
                                      nullptr,
                                      nullptr,
                                      MenuControlRole::none,
                                      {{0, 0x01000093, 2, 0, 0, {}}}});
            stats.children.push_back(text_definition(id + 0x2000, stat_names[i][2]));
        }
        stats.children.push_back(
            text_definition(0x04516000, "text_soldier stats", {caption_definition(11, 0xab3)}));
        out.controls.push_back(std::move(stats));
        out.controls.push_back(button(0x04600000, "bassign to all", 0xab0));
        out.controls.push_back(
            text_definition(0x04700000, "text_inventory", {caption_definition(11, 0xaa8)}));
        auto presets = group(0x04800000, "soldier_type", MenuControlKind::combo);
        presets.initialization = {{0, 0x01000081, 12, 500, 0, {}},
                                  observer_control_event(0, 0x0400e000, this, &preset_changed)};
        presets.initialization.push_back({0, 0x01000017, 19, 500, 0, {}});
        preset_indices_.clear();
        const auto &values = startup_.pre_mission().resources().presets;
        for (std::size_t i = 0; i < values.size(); ++i)
            if (values[i].field1ac & 5u) {
                ControlEvent e{0, 0x01000016, 19, 0, 0, {}};
                e.text = std::make_shared<const std::string>(values[i].name);
                presets.initialization.push_back(e);
                preset_indices_.push_back({i, false});
            }
        const auto &user = startup_.pre_mission().resources().user_presets;
        for (std::size_t i = 0; i < user.size(); ++i)
            if (user[i].field1ac & 5u) {
                ControlEvent e{0, 0x01000016, 19, 0, 0, {}};
                e.text = std::make_shared<const std::string>(user[i].name);
                presets.initialization.push_back(e);
                preset_indices_.push_back({i, true});
            }
        PreMissionChoice::children(presets);
        out.controls.push_back(std::move(presets));
        out.controls.push_back(text_definition(0x04900000, "text_mission essentials"));
        const bool original = startup_.pre_mission().descriptor()->word10 == 3;
        out.controls.push_back(
            text_definition(0x04a00000, "text_screen", {caption_definition(10, original ? 0xa8d : 0xa8e)}));
        out.controls.push_back(
            text_definition(0x04b00000, "text_military storages", {caption_definition(11, 0xa93)}));
        out.controls.push_back(text_definition(0x04c00000, "text_team", {caption_definition(11, 0xa92)}));
        out.controls.push_back(button(0x04d00000, "baccept and continue", original ? 0xa90 : 0xab1));
        out.controls.push_back(button(0x04e00000, "bauto equip", 0xa8f));
        out.controls.push_back(button(0x04f00000, "bback", 0xa91));
        out.controls.push_back(
            text_definition(0x05000000, "text_h&dversion", {caption_definition(40, 0x7d6)}));
        out.controls.push_back(
            text_definition(0x05100000, "text_assortment", {caption_definition(11, 0xaa6)}));
        out.controls.push_back(text_definition(0x05200000, "description", {caption_definition(11, 0x7d2)}));
        selector_ = scene_.control_frame("selector", module_, false, model_);
        scene_.set_screen_visible(module_, selector_, false);
        return out;
    }
    void description(std::shared_ptr<PreMissionDescription> value) { description_ = std::move(value); }
    PreMissionSoldierSelection &selection() { return selection_; }
    void shown() {
        if (!initialized_) {
            startup_.pre_mission().auto_equipment(true);
            initialized_ = true;
        }
        refresh();
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
            for (std::size_t i = 0; i < startup_.pre_mission().squad().size(); ++i)
                startup_.pre_mission().squad()[i].active = i == selection_.selected();
            queue_.enqueue({0, 0x020000c1, 0, 0, 0, {}});
            return true;
        case 0x020000c1:
            accept(false, true);
            return true;
        case 0x0200000b:
            accept(false);
            return true;
        case 0x020000c6:
            accept(true);
            return true;
        default:
            return false;
        }
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &d, EngineFrame *frame,
                                              MenuTextObject &text, MenuWidgetContext &context,
                                              std::function<MenuTextObject *(unsigned)> lookup) {
        using K = frontend::MenuControlKind;
        context_ = &context;
        lookup_ = lookup;
        auto extra = [this, &context, lookup](const frontend::MenuControlDefinition &child, EngineFrame *f) {
            return make_control(child, f, *lookup(child.identifier), context, lookup);
        };
        if (d.kind == K::image)
            return std::make_unique<PreMissionImage>(scene_, module_, d, frame, text, context);
        if (d.kind == K::meter)
            return std::make_unique<PreMissionMeter>(scene_, module_, d, frame, text, context.queue);
        if (d.kind == K::combo)
            return std::make_unique<PreMissionChoice>(scene_, module_, d, frame, text, context, lookup);
        if (d.kind == K::item_row)
            return std::make_unique<PreMissionItemRow>(scene_, module_, d, frame, text, context, lookup);
        if (d.kind == K::binding_list || d.kind == K::movie_list)
            return std::make_unique<PreMissionList>(scene_, module_, d, frame, text, context, lookup, extra);
        if (d.kind == K::container)
            return std::make_unique<OptionsGroup>(scene_, module_, d, frame, text, context, lookup, extra);
        return {};
    }

  private:
    struct StoreRow {
        unsigned list, id;
        int item, ammo;
    };
    struct PresetRef {
        std::size_t index;
        bool user;
    };
    static frontend::MenuControlDefinition
    group(unsigned id, const char *name,
          frontend::MenuControlKind kind = frontend::MenuControlKind::container) {
        return {id, name, kind, nullptr, nullptr, frontend::MenuControlRole::none, {}};
    }
    void name_frame(EngineFrame *frame, const char *name) {
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        cinematics_engine_method<Name>(frame, module_, 0x34)(frame, name);
    }
    frontend::MenuControlDefinition image(unsigned id, const char *name) {
        auto d = group(id, name, frontend::MenuControlKind::image);
        frontend::ControlEvent bitmap{0, 0x0100009a, 0, 0, 0, {}};
        bitmap.text = std::make_shared<const std::string>("X_SQUARE.BMP");
        d.initialization.push_back(bitmap);
        for (const auto tag : {0x04001000u, 0x04013000u, 0x0400a000u, 0x04009000u, 0x04003000u, 0x04004000u})
            d.initialization.push_back(frontend::observer_control_event(0, tag, this, &item_action));
        return d;
    }
    frontend::MenuControlDefinition button(unsigned id, const char *name, unsigned label) {
        auto d = frontend::button_definition(
            id, name, "normal*", "actived*", frontend::MenuControlRole::single_player,
            {frontend::observer_control_event(0, 0x04001000, this, &action)});
        if (label)
            d.initialization.insert(d.initialization.begin(), frontend::caption_definition(13, label));
        return d;
    }
    static unsigned item_group(const frontend::PreMissionItem &item) {
        const auto kind = item.kind();
        if (kind == 8)
            return 0;
        if (!item.item_class)
            return 4;
        if (kind == 16 || kind == 15)
            return 6;
        if (kind <= 2)
            return 1;
        if (kind == 3 || kind == 4)
            return 2;
        if (kind == 12 || (kind >= 5 && kind <= 7))
            return 3;
        if (kind == 9 || kind == 10 || kind == 11 || kind == 21)
            return 5;
        return 7; // 007DFA10.
    }
    frontend::MenuControlDefinition store_definition() {
        using namespace frontend;
        auto *parent = scene_.create_group_frame(module_, model_, "P_visualstore", {0, 0, 0});
        auto store = group(0x04400000, "P_visualstore");
        for (const char *name : {"filter", "scroll01"})
            clone_.link(scene_.control_frame(name, module_, false, model_), parent, 1);
        auto filter = group(0x04401000, "filter", MenuControlKind::combo);
        filter.initialization = {{0, 0x01000081, 12, 0xa9f, 0, {}},
                                 observer_control_event(0, 0x0400e000, this, &filter_changed)};
        for (unsigned label : {0xa9fu, 0xaa0u, 0xaa1u, 0xaa2u, 0xaa3u, 0xaa4u})
            filter.initialization.push_back({0, 0x01000017, 19, label, 0, {}});
        PreMissionChoice::children(filter);
        store.children.push_back(std::move(filter));
        store.children.push_back({0x04402000,
                                  "scroll01",
                                  MenuControlKind::slider,
                                  nullptr,
                                  nullptr,
                                  MenuControlRole::none,
                                  {{0, 0x0100000b, 0x04403000, 0, 0, {}}}});
        auto *prototype = scene_.create_group_frame(module_, parent, "vi_template", {0, 0, 0});
        auto *icon = scene_.control_frame("item08", module_, false, model_);
        auto *ammo = scene_.control_frame("item09", module_, false, model_);
        auto *caption = scene_.control_frame("text_info00", module_, false, model_);
        clone_.link(icon, prototype, 1);
        clone_.link(ammo, prototype, 1);
        clone_.link(caption, prototype, 1);
        name_frame(icon, "icon");
        name_frame(ammo, "ammo");
        name_frame(caption, "text_info");
        for (unsigned group_index = 0; group_index < 6; ++group_index) {
            auto *frame = scene_.create_group_frame(module_, parent, group_names[group_index], {0, 0, 0});
            const unsigned id = 0x04403000 + group_index * 0x1000;
            auto list = group(id, group_names[group_index], MenuControlKind::movie_list);
            list.initialization = {{0, 0x0100000b, 0x04402000, 0, 0, {}},
                                   {0, 0x01000028, 4, 0, 0, {}},
                                   {0, 0x0100002a, 0, 0x3dc28f5c, 0, {}}};
            unsigned row = 0;
            for (int item_id = 0; item_id < 500; ++item_id) {
                const auto *item = startup_.pre_mission().resources().catalogue.item(item_id);
                if (!item || item->fields38[3] == 3 || item_group(*item) != group_values[group_index] ||
                    item->kind() == 16)
                    continue;
                if (++row > 255)
                    throw std::runtime_error("Original store category exceeds row identifier space");
                char name[32];
                std::snprintf(name, sizeof(name), "frame_%u", row);
                names_.emplace_back(name);
                auto clone = clone_menu_frame(clone_, prototype, frame);
                name_frame(clone.get(), names_.back().c_str());
                auto entry = group(id + row * 16, names_.back().c_str(), MenuControlKind::item_row);
                entry.children.push_back(image(entry.identifier | 1, "icon"));
                entry.children.push_back(image(entry.identifier | 2, "ammo"));
                const auto associated = item->item_class == 1 ? item->ammunition() : -1;
                entry.children[0].initialization.push_back(
                    {0, 0x01000009, static_cast<unsigned>(item_id), 0, 0, {}});
                entry.children[0].initialization.push_back(
                    PreMissionImage::texture_event(0, icons_.get(*item)));
                entry.children[1].initialization.push_back(
                    {0, 0x01000009, static_cast<unsigned>(associated), 0, 0, {}});
                const auto *bullet = startup_.pre_mission().resources().catalogue.item(associated);
                entry.children[1].initialization.push_back(
                    PreMissionImage::texture_event(0, bullet ? icons_.get(*bullet) : nullptr));
                entry.children.push_back(text_definition(entry.identifier | 3, "text_info"));
                rows_.push_back({id, entry.identifier, item_id, associated});
                list.children.push_back(std::move(entry));
                frames_.push_back(std::move(clone));
            }
            store.children.push_back(std::move(list));
        }
        scene_.set_screen_visible(module_, prototype, false);
        scene_.set_screen_visible(module_, scene_.control_frame("itemselector01", module_, false, model_),
                                  false);
        return store;
    }
    void raw(unsigned id, unsigned style, const std::string &text) {
        frontend::ControlEvent event{id, 0x01000014, style, 0, 0, {}};
        event.text = std::make_shared<const std::string>(text);
        queue_.enqueue(event);
    }
    void item_visual(unsigned id, int item) {
        const auto *value = startup_.pre_mission().resources().catalogue.item(item);
        queue_.enqueue({id, 0x01000009, static_cast<unsigned>(item), 0, 0, {}});
        if (value)
            queue_.enqueue(PreMissionImage::texture_event(id, icons_.get(*value)));
        else {
            queue_.enqueue({id, 0x01000039, 0, 0, 0, {}});
            queue_.enqueue({id, 0x01000003, 0, 0, 0, {}});
        }
    }
    void refresh() {
        auto &flow = startup_.pre_mission();
        if (soldier_ >= flow.squad().size())
            throw std::logic_error("Missing selected equipment soldier");
        const auto &soldier = flow.squad()[soldier_];
        for (unsigned i = 0; i < 5; ++i)
            item_visual(0x04101000 + i * 0x1000, soldier.equipment[slot_indices[i]].item);
        char text[256];
        std::snprintf(text, sizeof(text), "%s  %.0f / %d Kg", soldier.name.c_str(), soldier.load,
                      soldier.capacity);
        raw(0x04200000, 11, text);
        for (unsigned carried = 0; carried < 2; ++carried) {
            const auto &entries = carried ? soldier.backpack : soldier.holsters;
            auto &offset = carried ? pack_offset_ : holster_offset_;
            offset = std::min<std::size_t>(offset, entries.size() > 4 ? entries.size() - 4 : 0);
            for (unsigned i = 0; i < 4; ++i)
                item_visual(0x04108000 + carried * 0x2000 + (i + 1) * 16,
                            offset + i < entries.size() ? entries[offset + i].item : -1);
            raw((0x04107000 + carried * 0x2000) | 0x40, 11,
                std::to_string(carried ? soldier.pack_volume : soldier.holster_volume));
            const float ratio =
                carried ? (soldier.pack_capacity > 0 ? soldier.pack_load / soldier.pack_capacity : 0)
                        : soldier.holster_volume / 12.f;
            queue_.enqueue({(0x04107000 + carried * 0x2000) | 0x10,
                            0x01000092,
                            0,
                            frontend::widget_float_bits(ratio),
                            0,
                            {}});
        }
        for (unsigned i = 0; i < 6; ++i) {
            const auto id = 0x04403000 + i * 0x1000;
            queue_.enqueue({id, 0x02000085, 0, 0, 0, {}});
            queue_.enqueue({id, filter_ == i ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
        }
        for (const auto &row : rows_)
            if (row.list == 0x04403000 + filter_ * 0x1000 &&
                (flow.store().count(row.item) > 0 || flow.store().count(row.ammo) > 0)) {
                queue_.enqueue({row.list, 0x02000083, row.id, 0, 0, {}});
                raw(row.id | 3, 6,
                    std::to_string(flow.store().count(row.item)) +
                        (row.ammo >= 0 ? " / " + std::to_string(flow.store().count(row.ammo)) : ""));
            }
        queue_.enqueue({0x04402000, 0x0100000b, 0x04403000 + filter_ * 0x1000, 0, 0, {}});
        const auto *policy = flow.equipment_policy();
        queue_.enqueue({0x04900000, 0x02000018, 0, 0, 0, {}});
        if (policy && !policy->required.empty()) {
            queue_.enqueue({0x04900000, 0x01000017, 2, 0xaa9, 0, {}});
            for (std::size_t i = 0; i < policy->required.size(); ++i) {
                const auto &required = policy->required[i];
                const auto *item = flow.resources().catalogue.item(required[0]);
                if (!item)
                    throw std::runtime_error("Required equipment metadata is missing");
                frontend::ControlEvent count{0x04900000, 0x01000016, 15, 0, 0, {}};
                count.text = std::make_shared<const std::string>(std::to_string(required[1]) + "x ");
                queue_.enqueue(count);
                queue_.enqueue({0x04900000, 0x01000017, 15, item->caption, 0, {}});
                frontend::ControlEvent punctuation{0x04900000, 0x01000016, 15, 0, 0, {}};
                punctuation.text =
                    std::make_shared<const std::string>(i + 1 < policy->required.size() ? ", " : ". ");
                queue_.enqueue(punctuation);
            }
        }
        const auto *human = startup_.pre_mission_humans().record(soldier.human);
        constexpr std::array<unsigned, 7> fields{{5, 2, 1, 3, 7, 8, 9}};
        for (unsigned i = 0; i < 7; ++i) {
            const float value =
                human ? frontend::pre_mission_detail::Bytes::real(human->stats[fields[i]]) : 0.f;
            if (!std::isfinite(value))
                throw std::runtime_error("Invalid equipment soldier statistic");
            queue_.enqueue({0x04500000 + (i * 3 + 2) * 0x1000,
                            0x01000092,
                            0,
                            frontend::widget_float_bits(i == 2 ? 1.f : value * .01f),
                            0,
                            {}});
            char number[40];
            std::snprintf(number, sizeof(number),
                          i == 0 || i >= 4 ? "%3.0f %%"
                          : i == 1         ? "%3.0f Kg"
                                           : "%3.0f",
                          value);
            raw(0x04500000 + (i * 3 + 3) * 0x1000, 23, number);
        }
    }
    void accept(bool confirmed, bool selected = false) {
        auto &flow = startup_.pre_mission();
        const auto location = flow.route().location();
        if (!confirmed && !selected && flow.route().root_index() == 0 && location &&
            (location->flat_ordinal == 2 || location->flat_ordinal == 15) && flow.squad().size() > 1) {
            std::vector<const frontend::HumanRecord *> humans;
            for (const auto &soldier : flow.squad())
                humans.push_back(startup_.pre_mission_humans().record(soldier.human));
            selection_.open(humans);
            return;
        }
        if (!selected && !confirmed)
            for (auto &soldier : flow.squad())
                soldier.active = true;
        using A = frontend::PreMissionAcceptance;
        const auto result = startup_.pre_mission().accept_equipment(confirmed);
        if (result == A::accepted) {
            advance_();
            return;
        }
        if (result == A::confirm_unarmed) {
            prompt_(1, lookup_(0x04200000)->localize(0x2531), 0x020000c6, 0);
            return;
        }
        if (result == A::overloaded) {
            prompt_(2, lookup_(0x04200000)->localize(0x7e7), 0, 0);
            return;
        }
        const auto missing = startup_.pre_mission().missing_item();
        const auto *item = missing ? startup_.pre_mission().resources().catalogue.item(*missing) : nullptr;
        if (!item)
            throw std::logic_error("Equipment rejection has no missing item");
        const auto format = lookup_(0x04200000)->localize(0x804);
        const auto name = lookup_(0x04200000)->localize(item->caption);
        std::string message = format ? *format : std::string{};
        const auto position = message.find("%s");
        if (position != std::string::npos)
            message.replace(position, 2, name ? *name : std::string{});
        prompt_(2, std::make_shared<const std::string>(std::move(message)), 0, 0);
    }
    void equip_item(int item, unsigned mask = 0) {
        auto &flow = startup_.pre_mission();
        const auto *value = flow.resources().catalogue.item(item);
        if (!value)
            return;
        if (mask)
            flow.equip(soldier_, mask, item, 1, true);
        else
            for (unsigned slot : {1u, 2u, 4u, 8u, 16u, 32u, 64u})
                if (value->permitted_slots() & slot)
                    if (flow.equip(soldier_, slot, item, 1))
                        break;
        refresh();
    }
    void remove_item(unsigned id) {
        auto &flow = startup_.pre_mission();
        auto &soldier = flow.squad()[soldier_];
        const auto child = id & 0xff000u;
        if (child >= 0x1000 && child <= 0x5000) {
            const auto i = child / 0x1000 - 1;
            flow.unequip(soldier_, slot_masks[i], soldier.equipment[slot_indices[i]]);
        } else if (child == 0x8000 || child == 0xa000) {
            const bool pack = child == 0xa000;
            const auto offset = (pack ? pack_offset_ : holster_offset_) + (id & 0xff0u) / 16 - 1;
            const auto &entries = pack ? soldier.backpack : soldier.holsters;
            if (offset < entries.size())
                flow.unequip(soldier_, pack ? 64 : 32, entries[offset]);
        }
        refresh();
    }
    unsigned inventory_mask(unsigned id) const {
        if ((id & 0x03f00000) != 0x00100000)
            return 0;
        const unsigned child = (id & 0xff000u) / 0x1000;
        return child >= 1 && child <= 5 ? slot_masks[child - 1] : child == 8 ? 32 : child == 10 ? 64 : 0;
    }
    frontend::PreMissionEquipmentEntry inventory_entry(unsigned id) const {
        const auto &soldier = startup_.pre_mission().squad()[soldier_];
        const unsigned child = (id & 0xff000u) / 0x1000;
        if (child >= 1 && child <= 5)
            return soldier.equipment[slot_indices[child - 1]];
        if (child == 8 || child == 10) {
            const bool pack = child == 10;
            const auto &entries = pack ? soldier.backpack : soldier.holsters;
            const auto offset = (pack ? pack_offset_ : holster_offset_) + (id & 0xff0u) / 16;
            if (offset && offset - 1 < entries.size())
                return entries[offset - 1];
        }
        return {};
    }
    static bool item_action(void *owner, void *, void *sender, unsigned tag, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        auto &image = *static_cast<PreMissionImage *>(sender);
        const unsigned id = image.identifier();
        const int item = frontend::widget_signed_word(image.user_data());
        if (tag == 0x04004000) {
            if (self.description_)
                self.description_->hide();
            return true;
        }
        if (tag == 0x04003000 || tag == 0x04001000) {
            self.selected_item_ = item;
            const auto *value = self.startup_.pre_mission().resources().catalogue.item(item);
            self.queue_.enqueue({0x05200000, 0x01000015, 11, value ? value->caption : 0x7d2, 0, {}});
            if (self.description_ && value) {
                const auto name = self.lookup_(0x05200000)->localize(value->caption);
                char weight[40];
                std::snprintf(weight, sizeof(weight), "%.2f Kg", value->weight);
                self.description_->request(
                    std::make_shared<const std::string>((name ? *name : std::string{}) + "\n" + weight));
            }
            return true;
        }
        if (tag == 0x0400a000) {
            self.drag_item_ = item;
            self.drag_source_ = id;
            return true;
        }
        if (tag == 0x04013000) {
            if ((id & 0x03f00000) == 0x00400000)
                self.equip_item(item);
            else
                self.remove_item(id);
            return true;
        }
        const int dragged = std::exchange(self.drag_item_, -1);
        if (dragged < 0)
            return true;
        const auto &cursor = self.context_->input.pointer.cursor();
        const unsigned target =
            self.scene_.pick_model(self.module_, cursor.x(), cursor.y(), 800, 600, self.model_);
        if ((self.drag_source_ & 0x03f00000) == 0x00400000 && (target & 0x03f00000) == 0x00100000) {
            const unsigned mask = self.inventory_mask(target);
            if (mask)
                self.equip_item(dragged, mask);
        } else if ((self.drag_source_ & 0x03f00000) == 0x00100000 &&
                   ((target & 0x03f00000) == 0x00400000 || (target & 0x03fff000) == 0x00106000))
            self.remove_item(self.drag_source_);
        else if (const unsigned source = self.inventory_mask(self.drag_source_)) {
            const auto entry = self.inventory_entry(self.drag_source_);
            auto &flow = self.startup_.pre_mission();
            if (const auto mask = self.inventory_mask(target))
                flow.move_equipment(self.soldier_, source, entry, self.soldier_, mask);
            else if ((target & 0x03f00000) == 0x00300000) {
                const auto selected = (target & 0xff000u) / 0x1000;
                if (selected && selected <= flow.squad().size()) {
                    const auto *item = flow.resources().catalogue.item(entry.item);
                    if (item)
                        for (unsigned mask : {1u, 2u, 4u, 8u, 16u, 32u, 64u})
                            if ((item->permitted_slots() & mask) &&
                                flow.move_equipment(self.soldier_, source, entry, selected - 1, mask))
                                break;
                }
            }
            self.refresh();
        }
        return true;
    }
    static bool soldier_hover(void *owner, void *, void *, unsigned tag, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        self.queue_.enqueue({0x04500000, tag == 0x04004000 ? 0x01000004u : 0x01000003u, 0, 0, 0, {}});
        return true;
    }
    static bool select_soldier(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        self.soldier_ = static_cast<PreMissionImage *>(sender)->user_data();
        self.pack_offset_ = self.holster_offset_ = 0;
        self.refresh();
        auto *frame = self.frames_.at(self.soldier_).get();
        self.clone_.link(self.selector_, frame, 0);
        self.scene_.set_frame_local_position(self.selector_, {0, 0, -.1f});
        self.scene_.set_screen_visible(self.module_, self.selector_, true);
        self.scene_.update_screen_frame(self.module_, self.selector_);
        return true;
    }
    static bool filter_changed(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        frontend::ControlEvent query{0x04401000, 0x0200002f, 0, 0, 0, {}};
        self.queue_.query_control(query);
        if (query.argument < 6)
            self.filter_ = query.argument;
        self.refresh();
        return true;
    }
    static bool preset_changed(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        frontend::ControlEvent query{0x04800000, 0x0200002f, 0, 0, 0, {}};
        self.queue_.query_control(query);
        if (!query.argument) {
            self.startup_.pre_mission().clear_optional_equipment(self.soldier_);
            self.preset_ = self.preset_indices_.size();
            self.refresh();
        } else if (query.argument - 1 < self.preset_indices_.size()) {
            self.preset_ = query.argument - 1;
            const auto ref = self.preset_indices_[self.preset_];
            self.startup_.pre_mission().use_preset(self.soldier_, ref.index, ref.user);
            self.refresh();
        }
        return true;
    }
    static bool action(void *owner, void *, void *sender, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionEquipmentMenu *>(owner);
        const auto id = static_cast<ButtonVisualPair *>(sender)->identifier();
        auto &flow = self.startup_.pre_mission();
        if (id == 0x04f00000)
            self.back_();
        else if (id == 0x04e00000)
            self.accept(false);
        else if (id == 0x04d00000) {
            flow.auto_equipment();
            self.refresh();
        } else if (id == 0x04600000) {
            for (std::size_t i = 0; i < flow.squad().size(); ++i) {
                if (self.preset_ < self.preset_indices_.size()) {
                    const auto ref = self.preset_indices_[self.preset_];
                    flow.use_preset(i, ref.index, ref.user);
                } else
                    flow.clear_optional_equipment(i);
            }
            self.refresh();
        } else if ((id & 0xffff0000u) == 0x04100000) {
            auto &offset = (id & 0xf000) == 0x9000 ? self.pack_offset_ : self.holster_offset_;
            if ((id & 0xff0) == 0x20) {
                if (offset)
                    --offset;
            } else
                ++offset;
            self.refresh();
        }
        return true;
    }
    inline static constexpr const char *slot_names[5] = {"head", "uniform", "back", "shoulder", "hands"};
    inline static constexpr unsigned slot_indices[5] = {1, 0, 3, 2, 4}, slot_masks[5] = {2, 1, 8, 4, 16};
    inline static constexpr const char *holster_names[4] = {"item00", "item01", "item02", "item03"};
    inline static constexpr const char *pack_names[4] = {"item04", "item05", "item06", "item07"};
    inline static constexpr const char *group_names[6] = {
        "GR_SmallWeapons", "GR_MiddleWeapons", "GR_HeavyWeapons", "GR_Explosives", "GR_Dresses", "GR_Other"};
    inline static constexpr unsigned group_values[6] = {1, 2, 3, 5, 6, 7};
    inline static constexpr const char *stat_names[7][3] = {
        {"text_shooting", "hlcpct07", "text_shooting01"}, {"text_strenght", "hlcpct08", "text_strenght01"},
        {"text_health", "hlcpct09", "text_health01"},     {"text_endurance", "hlcpct03", "text_endurance01"},
        {"text_stealth", "hlcpct04", "text_stealth01"},   {"text_first aid", "hlcpct05", "text_first aid01"},
        {"text_lockpick", "hlcpct06", "text_lockpick01"}};
    StartupSession &startup_;
    MenuScene &scene_;
    HMODULE module_;
    EngineFrame *model_;
    frontend::MenuEventQueue &queue_;
    CinematicsCloneBackend clone_;
    PreMissionIcons icons_;
    PreMissionSoldierSelection selection_;
    std::deque<std::string> names_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> frames_;
    std::vector<StoreRow> rows_;
    std::vector<PresetRef> preset_indices_;
    std::array<EngineFrame *, 5> slot_frames_{};
    EngineFrame *selector_ = nullptr;
    MenuWidgetContext *context_ = nullptr;
    std::function<MenuTextObject *(unsigned)> lookup_;
    std::function<void()> advance_, back_;
    std::function<void(unsigned, frontend::SharedText, unsigned, unsigned)> prompt_;
    std::size_t soldier_ = 0, preset_ = std::numeric_limits<std::size_t>::max(), pack_offset_ = 0,
                holster_offset_ = 0;
    unsigned filter_ = 0, drag_source_ = 0;
    std::shared_ptr<PreMissionDescription> description_;
    int selected_item_ = -1, drag_item_ = -1;
    bool initialized_ = false;
};
} // namespace hd2::host
#endif
