#ifndef HD2_HOST_SINGLE_PLAYER_MENU_H
#define HD2_HOST_SINGLE_PLAYER_MENU_H
#include "frontend/single_player_definition.h"
#include "frontend/single_player_mode.h"
#include "host/pre_mission_controls.h"
#include <deque>
#include <map>

namespace hd2::host {

// 0063B590/0063B740 and006391E0/00639A70/0063A010's retained entry owner.
// Metadata comes from the same actual archive resolver as the live preparation
// flow. Progress gates use the selected profile's original per-mode words.
class SinglePlayerMenu {
  public:
    SinglePlayerMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                     frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), module_(module), queue_(queue), clone_{driver, module},
          modes_(scene.load_screen_model(module, "singleplayer")),
          missions_(scene.load_screen_model(module, "single mission 2")) {
        if (!data_.load(startup_.pre_mission_files()))
            throw std::runtime_error("Missing single-player GameData");
    }
    void transitions(std::function<void()> begin, std::function<void(frontend::SharedText)> unavailable) {
        begin_ = std::move(begin);
        unavailable_ = std::move(unavailable);
    }
    EngineFrame *mode_model() const { return modes_.get(); }
    EngineFrame *mission_model() const { return missions_.get(); }
    frontend::MenuScreenDefinition modes_definition() {
        return frontend::single_player_definition(this, &shown, &mode_action);
    }
    frontend::MenuScreenDefinition missions_definition() {
        using namespace frontend;
        MenuScreenDefinition out{"SINGLE MISSION", 0x14000000, "single mission 2", {}};
        for (const char *name : {"video", "blong", "bshort"})
            scene_.set_screen_visible(module_, scene_.control_frame(name, module_, false, missions_.get()),
                                      false);
        out.controls.push_back(text_definition(0x14100000, "text_screen", {caption_definition(10, 0x8cb)}));
        out.controls.push_back(
            {0x14200000, "screen_shot", MenuControlKind::image, nullptr, nullptr, MenuControlRole::none, {}});
        out.controls.push_back(text_definition(0x14300000, "text00", {{0, 0x02000073, 0, 0, 0, {}}}));
        out.controls.push_back(button_definition(0x14400000, "bload mission", "normal*", "actived*",
                                                 MenuControlRole::single_player,
                                                 {caption_definition(13, 0x935),
                                                  {0, 0x01000001, 0, 0, 0, {}},
                                                  observer_control_event(0, 0x04001000, this, &start)}));
        out.controls.push_back(
            button_definition(0x14500000, "bexit", "normal*", "actived*", MenuControlRole::single_player,
                              {caption_definition(13, 0x8d0), {0, 0x06001003, 0, 0, 0, {}}}));
        out.controls.push_back({0x14600000,
                                "scroll00",
                                MenuControlKind::slider,
                                nullptr,
                                nullptr,
                                MenuControlRole::none,
                                {{0, 0x0100000b, 0x14700000, 0, 0, {}}}});
        auto *long_row = scene_.control_frame("blong", module_, false, missions_.get());
        auto *short_row = scene_.control_frame("bshort", module_, false, missions_.get());
        std::array<float, 3> origin{};
        std::memcpy(origin.data(), reinterpret_cast<const char *>(long_row) + 0xb0, 12);
        for (unsigned standalone = 0; standalone < 2; ++standalone) {
            const auto list_id = standalone ? 0x14800000u : 0x14700000u;
            const char *name = standalone ? "GB_SingleMission" : "GB_Campaign";
            auto *group = scene_.create_group_frame(module_, missions_.get(), name, {0, 0, 0});
            MenuControlDefinition list{
                list_id,
                name,
                MenuControlKind::movie_list,
                nullptr,
                nullptr,
                MenuControlRole::none,
                {{0, 0x0100000b, 0x14600000, 0, 0, {}},
                 {0, 0x01000028, 17, 0, 0, {}},
                 {0, 0x0100002a, 0, 0x3d03126f, 0, {}},
                 {0, 0x0100006b, 0, 0, 0, std::make_shared<std::array<float, 3>>(origin)}}};
            unsigned row = 0, flat = 0;
            if (!data_.roots().empty())
                for (const auto &campaign : data_.roots()[0]->children)
                    if (campaign->kind == 2) {
                        if (!standalone)
                            append_row(list, group, long_row, "CAMP", ++row, -1, campaign->word8);
                        for (const auto &mission : campaign->children)
                            if (mission->kind == 3) {
                                if (!standalone || flat > 0)
                                    append_row(list, group, short_row, standalone ? "SMMission" : "Mission",
                                               ++row, flat, mission->word8);
                                ++flat;
                            }
                    }
            out.controls.push_back(std::move(list));
        }
        out.controls.push_back(
            text_definition(0x14900000, "text_list of profiles", {caption_definition(11, 0x8d2)}));
        out.controls.push_back(
            text_definition(0x14a00000, "text_h&dversion", {caption_definition(40, 0x7d6)}));
        out.controls.push_back(button_definition(0x14b00000, "bload lastsave", "normal*", "actived*",
                                                 MenuControlRole::single_player,
                                                 {caption_definition(13, 0x839),
                                                  {0, 0x01000001, 0, 0, 0, {}},
                                                  observer_control_event(0, 0x04001000, this, &saved_game)}));
        out.controls.push_back(
            button_definition(0x14c00000, "bexit01", "normal*", "actived*", MenuControlRole::single_player,
                              {caption_definition(13, 0x8d0), {0, 0x01000004, 0, 0, 0, {}}}));
        return out;
    }
    bool receive(frontend::ControlEvent &event) {
        switch (event.code & 0x03000fffu) {
        case 0x02000033:
            prepare();
            if (startup_.pre_mission_progress(mode_.root, mode_.mode) < 0)
                begin_campaign();
            else
                show_missions();
            return true;
        case 0x02000074:
            prepare();
            show_missions();
            return true;
        case 0x0200001e:
            begin_selected();
            return true;
        case 0x02000075:
            unavailable_saved_game();
            return true;
        default:
            return false;
        }
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &d, EngineFrame *frame,
                                              MenuTextObject &text, MenuWidgetContext &context,
                                              std::function<MenuTextObject *(unsigned)> lookup) {
        using K = frontend::MenuControlKind;
        if (d.kind == K::image_selector)
            return std::make_unique<PreMissionSideImages>(scene_, module_, d, frame, text, context.queue);
        if (d.kind == K::image)
            return std::make_unique<PreMissionImage>(scene_, module_, d, frame, text, context);
        if (d.kind == K::movie_list)
            return std::make_unique<PreMissionList>(
                scene_, module_, d, frame, text, context, lookup,
                [this, &context, lookup](const frontend::MenuControlDefinition &child, EngineFrame *f) {
                    return make_control(child, f, *lookup(child.identifier), context, lookup);
                });
        if (d.kind == K::movie_row) {
            auto result =
                std::make_unique<MenuControl>(scene_, module_, false, d, frame, text, true, &context);
            const auto identifier = d.identifier;
            result->on_row_activation([this, identifier] { select(identifier); });
            row_controls_.emplace(identifier, result.get());
            return result;
        }
        return {};
    }

  private:
    struct Row {
        unsigned identifier;
        int mission;
        bool standalone;
    };
    void append_row(frontend::MenuControlDefinition &list, EngineFrame *group, EngineFrame *prototype,
                    const char *prefix, unsigned row, int mission, unsigned label) {
        char name[40];
        std::snprintf(name, sizeof(name), "%s_%02u", prefix, row);
        names_.emplace_back(name);
        auto frame = frontend::clone_menu_frame(clone_, prototype, group);
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        cinematics_engine_method<Name>(frame.get(), module_, 0x34)(frame.get(), names_.back().c_str());
        scene_.set_screen_visible(module_, frame.get(), true);
        const auto id = list.identifier + row * 0x1000;
        auto d = frontend::button_definition(id, names_.back().c_str(), "normal*", "actived*",
                                             frontend::MenuControlRole::single_player,
                                             {frontend::caption_definition(mission < 0 ? 13 : 12, label)});
        d.kind = frontend::MenuControlKind::movie_row;
        d.initialization.push_back({0, 0x01000009, static_cast<unsigned>(mission), 0, 0, {}});
        if (mission < 0)
            d.initialization.push_back({0, 0x01000005, 0, 0, 0, {}});
        list.children.push_back(std::move(d));
        rows_.push_back({id, mission, list.identifier == 0x14800000});
        frames_.push_back(std::move(frame));
    }
    void prepare() {
        if (!startup_.prepare_pre_mission())
            throw std::runtime_error("Single-player preparation resource load failed");
    }
    void begin_campaign() {
        if (!startup_.pre_mission().begin_campaign(mode_.root, mode_.mode))
            throw std::runtime_error("Empty campaign preparation route");
        begin_();
    }
    void show_missions() {
        selected_ = -1;
        const auto list = mode_.standalone ? 0x14800000u : 0x14700000u;
        const auto progress = startup_.pre_mission_progress(mode_.root, mode_.standalone ? 0 : mode_.mode);
        for (unsigned id : {0x14700000u, 0x14800000u}) {
            queue_.enqueue({id, 0x02000085, 0, 0, 0, {}});
            queue_.enqueue({id, id == list ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
        }
        for (const auto &row : rows_)
            if (row.standalone == mode_.standalone && (row.mission < 0 || row.mission <= progress))
                queue_.enqueue({list, 0x02000083, row.identifier, 0, 0, {}});
        queue_.enqueue({0x14600000, 0x0100000b, list, 0, 0, {}});
        queue_.enqueue({0x14100000,
                        0x01000015,
                        10,
                        mode_.standalone ? (mode_.mode == 3 ? 0x8d6u : 0x8cbu) : 0x8d2u + mode_.mode,
                        0,
                        {}});
        queue_.enqueue({0x14300000, 0x02000018, 0, 0, 0, {}});
        queue_.enqueue({0x14400000, 0x01000001, 0, 0, 0, {}});
        queue_.enqueue({0, 0x02000001, 5, 0, 0, {}});
    }
    void select(unsigned identifier) {
        const auto found = std::find_if(rows_.begin(), rows_.end(),
                                        [&](const auto &row) { return row.identifier == identifier; });
        if (found == rows_.end() || found->mission < 0)
            return;
        for (const auto &row : row_controls_)
            if (row.first != identifier)
                row.second->deselect_row();
        selected_ = found->mission;
        const auto location = frontend::find_pre_mission_location(data_, mode_.root, selected_);
        if (!location)
            throw std::logic_error("Selected mission metadata is absent");
        queue_.enqueue({0x14400000, 0x01000002, 0, 0, 0, {}});
        queue_.enqueue({0x14300000, 0x02000018, 0, 0, 0, {}});
        queue_.enqueue({0x14300000, 0x01000017, 2, location->campaign->word8, 0, {}});
        queue_.enqueue({0x14300000, 0x01000017, 3, location->mission->word8, 0, {}});
        frontend::ControlEvent picture{0x14200000, 0x0100003a, 0, 0, 0, {}};
        picture.text = std::make_shared<const std::string>(location->mission->text28);
        queue_.enqueue(picture);
    }
    void begin_selected() {
        if (selected_ < 0)
            return;
        if (mode_.standalone) {
            if (!startup_.pre_mission().begin_standalone(mode_.root, selected_, mode_.mode))
                throw std::runtime_error("Empty standalone preparation route");
            begin_();
        } else if (selected_ == 0)
            begin_campaign();
        else
            unavailable_saved_game();
    }
    void unavailable_saved_game() {
        unavailable_(
            std::make_shared<const std::string>("Loading a saved game is unavailable in this build."));
    }
    static bool shown(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<SinglePlayerMenu *>(owner);
        frontend::show_single_player_modes(self.queue_);
        return true;
    }
    static bool mode_action(void *owner, void *, void *sender, unsigned tag, unsigned) {
        auto &self = *static_cast<SinglePlayerMenu *>(owner);
        return frontend::dispatch_single_player_mode(
            self.queue_, self.mode_, static_cast<ButtonVisualPair *>(sender)->identifier(), tag);
    }
    static bool start(void *owner, void *, void *, unsigned, unsigned) {
        static_cast<SinglePlayerMenu *>(owner)->begin_selected();
        return true;
    }
    static bool saved_game(void *owner, void *, void *, unsigned, unsigned) {
        static_cast<SinglePlayerMenu *>(owner)->unavailable_saved_game();
        return true;
    }
    StartupSession &startup_;
    MenuScene &scene_;
    HMODULE module_;
    frontend::MenuEventQueue &queue_;
    CinematicsCloneBackend clone_;
    MenuDriver::Owner modes_, missions_;
    frontend::GameData data_;
    frontend::SinglePlayerMode mode_;
    std::deque<std::string> names_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> frames_;
    std::vector<Row> rows_;
    std::map<unsigned, MenuControl *> row_controls_;
    std::function<void()> begin_;
    std::function<void(frontend::SharedText)> unavailable_;
    int selected_ = -1;
};
} // namespace hd2::host
#endif
