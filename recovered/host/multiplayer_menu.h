#ifndef HD2_HOST_MULTIPLAYER_MENU_H
#define HD2_HOST_MULTIPLAYER_MENU_H

#include "frontend/multiplayer_maps.h"
#include "host/multiplayer_controls.h"
#include "host/multiplayer_discovery.h"
#include "host/pre_mission_choice.h"
#include <deque>
#include <map>

namespace hd2::host {

// The ordinary type2 owner,006333C0, slots8..14. Connection-owner slots1/2
// remain a separate owner: their IDs must never be inserted in this registry.
class MultiplayerMenu {
  public:
    using Definition=frontend::MenuScreenDefinition;
    using Control=frontend::MenuControlDefinition;
    using Event=frontend::ControlEvent;
    using K=frontend::MenuControlKind;
    using Text=frontend::SharedText;
    MultiplayerMenu(StartupSession& startup,MenuScene& scene,MenuDriver& driver,HMODULE module,
                    frontend::MenuEventQueue& queue)
        : startup_(startup),scene_(scene),module_(module),queue_(queue),clone_{driver,module} {
        for (const char* model:{"multiplayer menu","create server","password","server filters","gameinfo","confirm","password"})
            models_.push_back(scene_.load_screen_model(module_,model));
        for (unsigned slot:{10u,13u,14u})
            scene_.position_menu_model(module_,model(slot),.5f,-.375f,-.5f);
        auto catalogue=frontend::pre_mission_detail::read_file(startup_.pre_mission_files(),"mpmaplist.txt");
        if (!catalogue) catalogue=frontend::pre_mission_detail::read_file(startup_.pre_mission_files(),"GameData\\mpmaplist.txt");
        if (catalogue) maps_=frontend::decode_multiplayer_maps(std::string_view(
            reinterpret_cast<const char*>(catalogue->data()),catalogue->size()));
    }
    ~MultiplayerMenu() { cancel(); }
    EngineFrame* model(unsigned slot) const { return models_.at(slot-8).get(); }
    void errors(std::function<void(Text)> callback) { error_=std::move(callback); }
    std::optional<unsigned> backdrop(unsigned id) const {
        const auto found=backdrops_.find(id);
        return found==backdrops_.end()?std::optional<unsigned>{}:found->second;
    }
    void cancel() { discovery_.stop(); active_=false; cancel_drag();
        if (browser_busy_) browser_busy(false,false); }
    MenuTextObject* drag_text() const { return drag_text_.get(); }
    PreMissionChoice* dropdown() const { return preset_choice_ && preset_choice_->expanded()?preset_choice_:nullptr; }

    Definition definition(unsigned slot) {
        switch (slot) {
        case 8: return browser_definition();
        case 9: return create_definition();
        case 10: return password_definition();
        case 11: return filters_definition();
        case 12: return information_definition();
        case 13: return console_definition();
        case 14: return address_definition();
        default: throw std::logic_error("Invalid multiplayer screen slot");
        }
    }
    std::unique_ptr<MenuControl> make_control(const Control& d,EngineFrame* frame,MenuTextObject& text,
                                             MenuWidgetContext& context,
                                             std::function<MenuTextObject*(unsigned)> lookup) {
        context_=&context;
        if (d.identifier>=0x30100000 && d.identifier<=0x30b00000)
            information_text_[d.identifier]=&text;
        if (d.identifier==0x30100000)
            information_text_[0x30100008]=lookup(0x30100008);
        if (d.identifier==0x20200000) {
            auto arrow=std::make_unique<MultiplayerSortArrow>(scene_,module_,d,frame,text,context);
            sort_arrow_=arrow.get(); return arrow;
        }
        if (d.kind==K::numeric) {
            auto field=std::make_unique<MultiplayerNumeric>(scene_,module_,d,frame,text,context,lookup);
            numeric_[d.identifier]=field.get(); return field;
        }
        if (d.kind==K::edit) {
            auto edit=std::make_unique<ProfileEdit>(scene_,module_,d,frame,text,context);
            edits_[d.identifier]=edit.get(); return edit;
        }
        if (d.kind==K::labeled_toggle) {
            internals_.push_back(d);
            auto& inner=internals_.back(); inner.kind=K::toggle; inner.frame="text*";
            auto* plane=scene_.control_frame("text*",module_,false,frame);
            text.bind_frame(plane,"text*"); text.object().inset=0;
            return std::make_unique<MultiplayerLabeledToggle>(scene_,module_,inner,plane,frame,text,context,clone_);
        }
        if (d.kind==K::combo) {
            auto choice=std::make_unique<PreMissionChoice>(scene_,module_,d,frame,text,context,lookup);
            if (d.identifier==0x25200000) preset_choice_=choice.get();
            return choice;
        }
        if (d.kind==K::image)
            return std::make_unique<MenuControl>(scene_,module_,false,d,frame,text,false,&context);
        if (d.kind==K::container) {
            auto extra=[this,&context,lookup](const Control& child,EngineFrame* f) {
                return make_control(child,f,*lookup(child.identifier),context,lookup);
            };
            return std::make_unique<OptionsGroup>(scene_,module_,d,frame,text,context,lookup,extra);
        }
        if (d.kind==K::movie_list) {
            auto extra=[this,&context,lookup](const Control& child,EngineFrame* f) {
                return make_control(child,f,*lookup(child.identifier),context,lookup);
            };
            if (d.identifier==0x20900000) {
                auto list=std::make_unique<MultiplayerServerList>(scene_,module_,d,frame,text,context,lookup,extra,
                    [this](unsigned id){return rows_.at(id);},[this](const auto& value){select(value);});
                browser_=list.get(); return list;
            }
            return std::make_unique<PreMissionList>(scene_,module_,d,frame,text,context,lookup,extra);
        }
        if (d.kind==K::movie_row) {
            if ((d.identifier&0xfff00000u)==0x20900000) {
                auto row=std::make_unique<MultiplayerServerRow>(scene_,module_,d,frame,text,context,clone_);
                const auto id=d.identifier;
                row->on_row_activation([this,id]{browser_->choose(id);});
                rows_[id]=row.get(); return row;
            }
            auto row=std::make_unique<MultiplayerMapRow>(scene_,module_,false,d,frame,text,true,&context);
            const auto id=d.identifier;
            row->on_row_activation([this,id]{map_selected(id);});
            map_rows_[id]=row.get(); map_frames_[id]=frame; return row;
        }
        if (d.kind==K::profile_list)
            return std::make_unique<ProfileList>(scene_,module_,d,frame,text,context,lookup,frame);
        return {};
    }

    bool receive(Event& event) {
      try {
        const auto code=event.code&0x03000fffu;
        if (code==0x02000099 || code==0x0200009a) { enter(code==0x0200009a); return true; }
        if (code==0x02000028) { cancel(); return true; }
        if (code==0x0200009b) { save(); return true; }
        if (code==0x0200002a) { join(); return true; }
        if (code==0x02000029) { start_host(); return true; }
        const auto* screen=queue_.screens().current();
        if (code==0x0200000a && screen && screen->identifier>=0x20000000 && screen->identifier<=0x38000000) {
            back(screen->identifier>>26); return true;
        }
        return false;
      } catch (const std::exception& error) { fail(error.what()); return true; }
    }
    void shown(unsigned identifier) {
        switch (identifier>>26) {
        case 9: fill_create(); break;
        case 11: fill_filters(); break;
        case 12: refresh_information(); break;
        default: break;
        }
    }
    void tick(unsigned ms) {
        clock_+=ms;
        if (auto* choice=dropdown()) {
            if (context_ && context_->input.input.pressed_mask3) {
                const auto& cursor=context_->input.pointer.cursor();
                choice->dismiss_on_press(cursor.x(),cursor.y());
            }
        }
        if (!pending_error_.empty() && queue_.screens().current() && queue_.screens().current()->identifier!=0 && error_) {
            auto message=std::move(pending_error_); pending_error_.clear(); error_(shared(std::move(message)));
        }
        if (console_) {
            console_elapsed_+=ms;
            if (console_elapsed_>=console_delay_) {
                console_=false;
                set_text(0x34100000,"Multiplayer game loading is unavailable in this build.",25);
            }
        }
        if (!active_) return;
        auto replies=discovery_.drain();
        for (const auto& value:replies.servers) { merge(value); last_response_=clock_; dirty_=true; }
        for (const auto& value:replies.restored) {
            const auto old=std::find_if(servers_.begin(),servers_.end(),[&](const auto& server) {
                return server.ipv4==value.requested.ipv4 && server.port==value.requested.port;
            });
            if (old!=servers_.end()) { *old=value.server; old->manual=value.requested.manual; }
            last_response_=clock_; dirty_=true;
        }
        for (auto& value:replies.information) {
            if (selected_ && selected_->ipv4==value.requested.ipv4 && selected_->port==value.requested.port) {
                selected_=value.server; selected_->manual=value.requested.manual;
                information_=std::move(value.details); information_pending_=false; fill_information();
            }
        }
        if (FAILED(replies.callback_error)) { cancel(); fail("The multiplayer discovery callback failed."); }
        if (dirty_ && clock_-last_paint_>1000) { paint_browser(); last_paint_=clock_; }
        if (active_ && clock_-last_response_>15000) {
            const bool pending=information_pending_; information_pending_=false;
            cancel(); paint_browser();
            if (pending) set_text(0x30200000,"No response from server.",2);
        }
    }

  private:
    static Text shared(std::string text) { return std::make_shared<const std::string>(std::move(text)); }
    static std::string peer_text(std::string_view value) {
        std::string result(value);
        for (char& c:result) if (static_cast<unsigned char>(c)<32 || c==127) c=' ';
        return result;
    }
    void fail(std::string text) { if (pending_error_.empty()) pending_error_=std::move(text); }
    void open(unsigned slot,bool modal=false) {
        if (modal && queue_.screens().current()) {
            auto base=queue_.screens().current()->identifier;
            if (const auto retained=backdrop(base)) base=*retained;
            backdrops_[slot<<26]=base;
        }
        queue_.enqueue({0,modal?0x02000002u:0x02000001u,slot,0,0,{}});
    }
    void pop(bool modal=false) { queue_.enqueue({0,modal?0x02000005u:0x02000003u,0,0,0,{}}); }
    void set_text(unsigned id,std::string value,unsigned style=13) {
        Event event{id,0x01000014,style,0,0,{}}; event.text=shared(std::move(value)); queue_.enqueue(event);
    }
    void caption(unsigned id,unsigned label,unsigned style=13) { queue_.enqueue({id,0x01000015,style,label,0,{}}); }
    void enabled(unsigned id,bool value) { queue_.enqueue({id,value?0x01000002u:0x01000001u,0,0,0,{}}); }
    unsigned selected_value(unsigned id) {
        Event query{id,0x0100006c,0,0,0,{}}; queue_.query_control(query); return query.argument;
    }
    void selected_value(unsigned id,unsigned value) { queue_.dispatch_immediate({id,0x0100006d,value,0,0,{}}); }
    static int integer(const std::string& value) {
        int result=0; const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
        if (parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size())
            throw std::runtime_error("Enter a valid number.");
        return result;
    }
    static int numeric_value(const std::string& value) {
        return value.empty() || value=="-"?0:integer(value); //Original sscanf destination starts0.
    }
    Control button(unsigned id,const char* frame,unsigned label) {
        return frontend::button_definition(id,frame,"normal*","actived*",frontend::MenuControlRole::multiplayer,
            {frontend::caption_definition(13,label),frontend::observer_control_event(0,0x04001000,this,&action)});
    }
    static bool action(void* owner,void*,void* sender,unsigned,unsigned) {
        auto& self=*static_cast<MultiplayerMenu*>(owner);
        const auto id=static_cast<ButtonVisualPair*>(sender)->identifier();
        try { self.clicked(id); } catch (const std::exception& error) { self.fail(error.what()); }
        return true;
    }
    static bool join_row(void* owner,void*,void*,unsigned,unsigned) {
        static_cast<MultiplayerMenu*>(owner)->join(); return true;
    }
    static bool changed_style(void* owner,void*,void*,unsigned,unsigned) {
        static_cast<MultiplayerMenu*>(owner)->style_changed(); return true;
    }
    static bool changed_preset(void* owner,void*,void*,unsigned,unsigned) {
        static_cast<MultiplayerMenu*>(owner)->preset_changed(); return true;
    }
    static bool changed_rule(void* owner,void*,void*,unsigned,unsigned) {
        auto& self=*static_cast<MultiplayerMenu*>(owner);
        if (!self.filling_) self.refresh_rule_eligibility();
        return true;
    }
    static bool map_action(void* owner,void*,void* sender,unsigned,unsigned) {
        static_cast<MultiplayerMenu*>(owner)->toggle_map(static_cast<ButtonVisualPair*>(sender)->identifier());
        return true;
    }
    static bool map_drag(void* owner,void*,void* sender,unsigned tag,unsigned) {
        auto& self=*static_cast<MultiplayerMenu*>(owner);
        self.drag_map(static_cast<ButtonVisualPair*>(sender)->identifier(),tag); return true;
    }
    static bool password_reply(void* owner,void*,void*,unsigned tag,unsigned) {
        auto& self=*static_cast<MultiplayerMenu*>(owner);
        if (tag==0x04016000) self.clicked(0x28300000);
        else if (tag==0x04017000) {
            self.join_password_.clear(); self.joining_.reset(); self.pop(true);
            //The character owner still restores/releases and consumes Escape.
        }
        return true;
    }
    static bool hostname_changed(void* owner,void*,void*,unsigned,unsigned) {
        auto& self=*static_cast<MultiplayerMenu*>(owner);
        self.settings_.hostname=self.edits_.at(0x24100000)->value();
        const auto& selected=self.settings_.rotations[std::min(self.settings_.preset,4u)][self.settings_.style%3];
        self.enabled(0x24900000,!self.settings_.hostname.empty() && !selected.empty() && !self.maps_.empty());
        return true;
    }
    Control selector(unsigned id,const char* frame,std::initializer_list<unsigned> labels) {
        Control result{id,frame,K::selector,nullptr,nullptr,frontend::MenuControlRole::none,{}};
        for (const auto label:labels) result.initialization.push_back({0,0x01000067,label,0,0,{}});
        return result;
    }
    Control edit(unsigned id,const char* frame,unsigned style=13) {
        return {id,frame,K::edit,"normal*","actived*",frontend::MenuControlRole::none,
            {frontend::caption_definition(style,0x7d2)}};
    }
    Control clone_row(EngineFrame* parent,EngineFrame* prototype,unsigned id,const std::string& name) {
        names_.push_back(name);
        auto frame=frontend::clone_menu_frame(clone_,prototype,parent);
        using Name=void(WINAPI*)(EngineFrame*,const char*);
        cinematics_engine_method<Name>(frame.get(),module_,0x34)(frame.get(),names_.back().c_str());
        scene_.set_screen_visible(module_,frame.get(),true);
        clones_.push_back(std::move(frame));
        auto result=frontend::button_definition(id,names_.back().c_str(),"normal*","actived*",
            frontend::MenuControlRole::multiplayer,{}); result.kind=K::movie_row;
        return result;
    }
    Definition browser_definition() {
        using namespace frontend;
        Definition out{"MULTIPLAYER",0x20000000,"multiplayer menu",{}};
        out.controls.push_back(text_definition(0x20100000,"text_screen",{caption_definition(10,0x8fd)}));
        auto arrows=button(0x20200000,"barrows",0x7d2);
        arrows.normal="normalup*"; arrows.active="activedup*"; arrows.initialization.erase(arrows.initialization.begin());
        out.controls.push_back(std::move(arrows));
        const char* headers[]{"bhost name","bmap","bstyle","bping","bplayers/max"};
        for (unsigned i=0;i<5;++i) out.controls.push_back(button(0x20300000+i*0x100000,headers[i],0x8fe + i));
        out.controls.push_back({0x20800000,"scroll00",K::slider,nullptr,nullptr,MenuControlRole::none,
            {{0,0x0100000b,0x20900000,0,0,{}}}});
        auto* group=scene_.create_group_frame(module_,model(8),"GB_ServerList",{0,0,0});
        auto* prototype=scene_.control_frame("sever00",module_,false,model(8));
        clone_.link(prototype,group,1); scene_.set_screen_visible(module_,prototype,false);
        Control list{0x20900000,"GB_ServerList",K::movie_list,nullptr,nullptr,MenuControlRole::none,
            {{0,0x0100000b,0x20800000,0,0,{}}}};
        for (unsigned i=1;i<=11;++i) {
            auto row=clone_row(group,prototype,0x20900000|(i<<12),"Server"+std::to_string(i));
            row.initialization.push_back(observer_control_event(0,0x04013000,this,&join_row));
            list.children.push_back(std::move(row));
        }
        out.controls.push_back(std::move(list));
        const char* frames[]{"brestore list","brefresh list","benter server ip","bserver filters","bjoin game",
            "bcreate server","bview game info","bback"};
        constexpr unsigned labels[]{0x915,0x917,0x904,0x905,0x906,0x907,0x908,0x909};
        for (unsigned i=0;i<8;++i) out.controls.push_back(button(0x20a00000+i*0x100000,frames[i],labels[i]));
        out.controls.push_back(text_definition(0x21200000,"text_h&dversion",{caption_definition(40,0x7d6)}));
        for (const auto pair:{std::pair<unsigned,const char*>{0x21300000,"gamespylogo"},{0x21400000,"esrb-US"}})
            out.controls.push_back({pair.first,pair.second,K::image,nullptr,nullptr,MenuControlRole::none,{}});
        out.controls.push_back(text_definition(0x21500000,"text_online"));
        out.controls.push_back(button(0x21600000,"badd/remove",0x4e86));
        out.controls.push_back(button(0x21700000,"bfavorites",0x4e84));
        return out;
    }
    Definition filters_definition() {
        using namespace frontend;
        Definition out{"SERVER FILTERS",0x2c000000,"server filters",{}};
        out.controls.push_back(selector(0x2c100000,"scan style",{2890,2891,2892,2893,20001}));
        out.controls.push_back(multiplayer_numeric_definition(0x2c200000,"bshort00",0x9fb,0,9999));
        const char* frames[]{"responding only","dedicated only","that are not empty","that are not full","without password","current version"};
        constexpr unsigned labels[]{0xa00,0xa01,0xa02,0xa03,0xa04,0xa13};
        for (unsigned i=0;i<6;++i) out.controls.push_back({0x2c300000+i*0x100000,frames[i],K::toggle,nullptr,nullptr,
            MenuControlRole::none,{caption_definition(13,labels[i])}});
        out.controls.push_back(button(0x2c900000,"baccept",0x9fc));
        out.controls.push_back(button(0x2ca00000,"bback",0x9fd));
        out.controls.push_back(text_definition(0x2cb00000,"text_screen",{caption_definition(10,0x9f7)}));
        out.controls.push_back(text_definition(0x2cc00000,"text_h&dversion",{caption_definition(40,0x7d6)}));
        out.controls.push_back(text_definition(0x2cd00000,"text_scan style",{caption_definition(11,0x9fa)}));
        out.controls.push_back(text_definition(0x2ce00000,"text_show servers",{caption_definition(11,0x9f8)}));
        return out;
    }
    Definition password_definition() {
        using namespace frontend;
        Definition out{"PASSWORD",0x28000000,"password",{}};
        out.controls.push_back(text_definition(0x28100000,"text",{caption_definition(12,0x7d3)}));
        auto input=edit(0x28200000,"binput",14);
        Event exclude{0,0x0100009b,0,0,0,{}}; exclude.text=shared(">_"); input.initialization.push_back(exclude);
        input.initialization.push_back(observer_control_event(0,0x04016000,this,&password_reply));
        input.initialization.push_back(observer_control_event(0,0x04017000,this,&password_reply));
        out.controls.push_back(std::move(input));
        out.controls.push_back(button(0x28300000,"byes*",0x7d1));
        out.controls.push_back(button(0x28400000,"bno*",2000));
        for (const char* name:{"binput01","binput02"}) scene_.set_screen_visible(module_,scene_.control_frame(name,module_,false,model(10)),false);
        return out;
    }
    Definition address_definition() {
        using namespace frontend;
        Definition out{"ADD SERVER",0x38000000,"password",{}};
        out.controls.push_back(text_definition(0x38100000,"text",{caption_definition(12,2022)}));
        out.controls.push_back(edit(0x38200000,"binput01"));
        auto port=edit(0x38300000,"binput02"); port.initialization.push_back({0,0x0100006f,1,0,0,{}});
        out.controls.push_back(std::move(port));
        out.controls.push_back(button(0x38400000,"byes*",0x7d1));
        out.controls.push_back(button(0x38500000,"bno*",2000));
        scene_.set_screen_visible(module_,scene_.control_frame("binput",module_,false,model(14)),false);
        return out;
    }
    Definition console_definition() {
        using namespace frontend;
        Definition out{"CONNECTION",0x34000000,"confirm",{}};
        out.controls.push_back(text_definition(0x34100000,"text"));
        for (const char* name:{"byes*","bno*"}) scene_.set_screen_visible(module_,scene_.control_frame(name,module_,false,model(13)),false);
        out.controls.push_back(button(0x34400000,"bok*",0x7d8));
        return out;
    }

    Definition create_definition();
    Definition information_definition();
    void clicked(unsigned id);
    void enter(bool internet);
    void back(unsigned slot);
    void load();
    void save();
    void paint_browser();
    void merge(const frontend::MultiplayerServer& value);
    void select(const frontend::MultiplayerServer& value);
    void refresh(bool restore=false);
    void browser_busy(bool busy,bool restore) {
        browser_busy_=busy;
        caption(0x20a00000,busy && restore?0x916:0x915);
        caption(0x20b00000,busy && !restore?0x918:0x917);
        enabled(0x20a00000,!busy || restore);
        enabled(0x20b00000,!busy || !restore);
    }
    void join();
    void start_host();
    void fill_create();
    void refresh_rule_eligibility();
    void commit_create();
    void fill_filters();
    void commit_filters();
    void refresh_information();
    void fill_information();
    void add_address();
    void style_changed();
    void preset_changed();
    void map_selected(unsigned id);
    void toggle_map(unsigned id);
    void drag_map(unsigned id,unsigned tag);
    void move_drag();
    void cancel_drag() { drag_text_.reset(); drag_frame_.reset(); drag_map_name_.clear(); }
    void reorder_drag(unsigned target);
    unsigned pick_maps();
    void fill_maps();
    void connection(bool host);

    StartupSession& startup_;
    MenuScene& scene_;
    HMODULE module_;
    frontend::MenuEventQueue& queue_;
    CinematicsCloneBackend clone_;
    std::vector<MenuDriver::Owner> models_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> clones_;
    std::deque<std::string> names_;
    std::deque<Control> internals_;
    std::map<unsigned,ProfileEdit*> edits_;
    std::map<unsigned,MultiplayerNumeric*> numeric_;
    std::map<unsigned,MultiplayerServerRow*> rows_;
    std::map<unsigned,MenuControl*> map_rows_;
    std::map<unsigned,EngineFrame*> map_frames_;
    MenuWidgetContext* context_=nullptr;
    MultiplayerServerList* browser_=nullptr;
    MultiplayerSortArrow* sort_arrow_=nullptr;
    PreMissionChoice* preset_choice_=nullptr;
    MultiplayerLanDiscovery discovery_;
    frontend::MultiplayerSettings settings_;
    frontend::MultiplayerServerLists saved_;
    std::vector<frontend::MultiplayerServer> servers_;
    std::vector<frontend::MultiplayerMap> maps_;
    std::optional<frontend::MultiplayerServer> selected_,joining_;
    std::optional<frontend::MultiplayerInformation> information_;
    std::map<unsigned,MenuTextObject*> information_text_;
    std::function<void(Text)> error_;
    bool internet_=false,favorites_=false,active_=false,dirty_=false,information_pending_=false;
    bool console_=false;
    bool browser_busy_=false;
    bool filling_=false;
    unsigned displayed_style_=0,focused_map_=0;
    std::map<unsigned,std::size_t> available_maps_;
    std::map<unsigned,unsigned> backdrops_;
    unsigned clock_=0,last_response_=0,last_paint_=0,console_elapsed_=0,console_delay_=0;
    std::string join_password_;
    std::string pending_error_;
    std::optional<frontend::MenuFrameClone<CinematicsCloneBackend>> drag_frame_;
    std::unique_ptr<MenuTextObject> drag_text_;
    std::string drag_map_name_;
    std::array<float,3> drag_position_{};
    std::array<float,2> drag_pointer_{};
    std::array<float,8> drag_bounds_{};
    std::size_t drag_index_=0;
    bool drag_available_=false;
};

inline void MultiplayerMenu::load() {
    settings_={};
    settings_.hostname=frontend::lookup_localized_text(startup_.text(),0x7e9);
    const auto settings=frontend::pre_mission_detail::read_file(startup_.pre_mission_files(),
        (startup_.pre_mission_profile_directory()+"MPSetingsOrig.sav").c_str());
    if (settings) {
        try { settings_=frontend::MultiplayerSettings::decode(settings->data(),settings->size(),settings_); }
        catch (const std::exception&) { fail("The multiplayer settings file is damaged. Defaults are in use."); }
    }
    saved_={};
    const auto saved=frontend::pre_mission_detail::read_file(startup_.pre_mission_files(),"PlayersProfiles\\ServerList.bin");
    if (saved) {
        try { saved_=frontend::MultiplayerServerLists::decode(saved->data(),saved->size()); }
        catch (const std::exception&) { fail("The saved server list is damaged."); }
    }
}

inline void MultiplayerMenu::save() {
    const auto write=[&](const std::string& path,const std::vector<unsigned char>& bytes) {
        auto& files=startup_.pre_mission_files();
        const auto handle=files.open_write(path.c_str(),0x10000);
        if (handle==0xffffffffu) return false;
        struct Close { ChunkFiles& files;unsigned handle;~Close(){files.close(handle);} } close{files,handle};
        return files.write(handle,bytes.data(),static_cast<unsigned>(bytes.size()))==bytes.size();
    };
    if (!write(startup_.pre_mission_profile_directory()+"MPSetingsOrig.sav",settings_.encode()))
        fail("Could not save multiplayer settings.");
    if (internet_) {
        if (!favorites_) saved_.ordinary=servers_;
        if (!write("PlayersProfiles\\ServerList.bin",saved_.encode())) fail("Could not save the server list.");
    }
}

inline void MultiplayerMenu::enter(bool internet) {
    cancel(); internet_=internet; favorites_=false; selected_.reset(); joining_.reset(); information_.reset();
    load();
    caption(0x20100000,internet?0x911:0x910,10);
    enabled(0x20a00000,internet); enabled(0x20c00000,internet);
    enabled(0x21600000,false); enabled(0x21700000,internet);
    enabled(0x20e00000,false); enabled(0x21000000,false);
    queue_.enqueue({0x21300000,internet?0x01000003u:0x01000004u,0,0,0,{}});
    servers_=internet?saved_.ordinary:std::vector<frontend::MultiplayerServer>{};
    open(8); paint_browser();
    if (internet) fail("Internet server browsing through GameSpy is unavailable. Saved servers and Add Server remain available.");
    else refresh();
}

inline void MultiplayerMenu::merge(const frontend::MultiplayerServer& value) {
    const auto found=std::find_if(servers_.begin(),servers_.end(),[&](const auto& old) {
        return old.ipv4==value.ipv4 && (!internet_ || old.port==value.port);
    });
    if (found==servers_.end()) servers_.push_back(value);
    else { const auto manual=found->manual; *found=value; found->manual=manual; }
    if (selected_ && selected_->ipv4==value.ipv4 && (!internet_ || selected_->port==value.port)) selected_=value;
}

inline void MultiplayerMenu::paint_browser() {
    std::vector<frontend::MultiplayerServer> visible;
    for (const auto& server:servers_)
        if (settings_.filters.accepts(server,internet_&&!favorites_?1:0,1,12)) visible.push_back(server);
    const auto lower=[](std::string_view value) {
        std::string result(value);
        for (auto& c:result) if (c>='A' && c<='Z') c=static_cast<char>(c+'a'-'A');
        return result;
    };
    std::stable_sort(visible.begin(),visible.end(),[&](const auto& a,const auto& b) {
        int comparison=0;
        const auto number=[&](auto x,auto y){return x<y?-1:x>y?1:0;};
        switch (settings_.sort&7u) {
        case 1: comparison=lower(frontend::multiplayer_text(a.host)).compare(lower(frontend::multiplayer_text(b.host))); break;
        case 2: comparison=lower(frontend::multiplayer_text(a.map)).compare(lower(frontend::multiplayer_text(b.map))); break;
        case 3: comparison=number(a.style,b.style); break;
        case 4: comparison=number(a.ping,b.ping); break;
        case 5: comparison=number(a.players,b.players); break;
        default: break;
        }
        return settings_.descending?comparison>0:comparison<0;
    });
    if (selected_) {
        const auto found=std::find_if(visible.begin(),visible.end(),[&](const auto& value) {
            return value.ipv4==selected_->ipv4 && value.port==selected_->port;
        });
        if (found==visible.end()) selected_.reset();
    }
    const auto filtered_count=visible.size();
    browser_->records(std::move(visible));
    enabled(0x20e00000,selected_.has_value()); enabled(0x21000000,selected_.has_value());
    enabled(0x21600000,internet_ && selected_.has_value());
    unsigned players=0; for (const auto& server:servers_) players+=server.players;
    set_text(0x21500000,std::to_string(filtered_count)+"/"+std::to_string(servers_.size())+
        " ----- "+std::to_string(players)+" "+frontend::lookup_localized_text(startup_.text(),0x22ae),9);
    sort_arrow_->direction(settings_.descending!=0); dirty_=false;
}

inline void MultiplayerMenu::select(const frontend::MultiplayerServer& value) {
    selected_=value;
    enabled(0x20e00000,true); enabled(0x21000000,true); enabled(0x21600000,internet_);
}

inline void MultiplayerMenu::refresh(bool restore) {
    if (active_) { cancel(); return; }
    if (internet_ && !restore) { fail("Internet server browsing through GameSpy is unavailable."); return; }
    cancel(); selected_.reset(); information_.reset();
    if (!restore) servers_.clear();
    paint_browser();
    if (restore && servers_.empty()) {
        browser_busy(true,true); active_=true; last_response_=last_paint_=clock_; return;
    }
    auto submission=restore?discovery_.start(&servers_.front(),2,startup_.wide_characters()):
        discovery_.start(nullptr,0,startup_.wide_characters());
    if (submission.accepted && restore)
        for (std::size_t i=1;i<servers_.size();++i) {
            const auto appended=discovery_.append(servers_[i],2);
            if (!appended.accepted) submission=appended;
        }
    if (!submission.accepted) {
        cancel(); char message[128];
        std::snprintf(message,sizeof(message),"Multiplayer discovery failed: 0x%08x.",static_cast<unsigned>(submission.result));
        fail(message); return;
    }
    browser_busy(true,restore); active_=true; last_response_=last_paint_=clock_;
}

inline void MultiplayerMenu::fill_filters() {
    filling_=true;
    selected_value(0x2c100000,static_cast<unsigned>(settings_.filters.style+1));
    numeric_.at(0x2c200000)->value(shared(std::to_string(settings_.filters.maximum_ping<1?9999:settings_.filters.maximum_ping)));
    const bool flags[]{settings_.filters.responding,settings_.filters.dedicated,settings_.filters.not_empty,
        settings_.filters.not_full,settings_.filters.no_password,settings_.filters.current_version};
    for (unsigned i=0;i<6;++i) selected_value(0x2c300000+i*0x100000,flags[i]);
    filling_=false;
}

inline void MultiplayerMenu::commit_filters() {
    numeric_.at(0x2c200000)->deactivate(queue_);
    settings_.filters.maximum_ping=numeric_value(numeric_.at(0x2c200000)->value());
    settings_.filters.style=static_cast<int>(selected_value(0x2c100000))-1;
    settings_.filters.responding=selected_value(0x2c300000)!=0;
    settings_.filters.dedicated=selected_value(0x2c400000)!=0;
    settings_.filters.not_empty=selected_value(0x2c500000)!=0;
    settings_.filters.not_full=selected_value(0x2c600000)!=0;
    settings_.filters.no_password=selected_value(0x2c700000)!=0;
    settings_.filters.current_version=selected_value(0x2c800000)!=0;
    save(); paint_browser();
}

inline void MultiplayerMenu::add_address() {
    const auto address=edits_.at(0x38200000)->value();
    const int port=integer(edits_.at(0x38300000)->value());
    if (port<1 || port>65535) throw std::runtime_error("Enter a port from1 to65535.");
    std::uint32_t ip=0; std::size_t at=0;
    for (unsigned part=0;part<4;++part) {
        const auto end=address.find('.',at);
        if ((part==3)!=(end==std::string::npos)) throw std::runtime_error("Enter a numeric IPv4 address.");
        const auto word=address.substr(at,end==std::string::npos?end:end-at);
        if (word.empty() || word.size()>3 || word.find_first_not_of("0123456789")!=std::string::npos)
            throw std::runtime_error("Enter a numeric IPv4 address.");
        const auto octet=integer(word);
        if (octet>255) throw std::runtime_error("Enter a numeric IPv4 address.");
        ip|=static_cast<unsigned>(octet)<<(part*8); at=end+1;
    }
    frontend::MultiplayerServer server;
    std::copy_n(" ??? ",5,server.host.data()); //00637440 initializes both display names to unknown.
    server.ipv4=ip; server.port=static_cast<unsigned>(port); server.manual=1; server.major=1; server.minor=12;
    merge(server); paint_browser(); pop(true); save();
}

inline void MultiplayerMenu::join() {
    if (!selected_) return;
    cancel(); joining_=*selected_; join_password_.clear();
    if (joining_->password) {
        caption(0x28200000,0x7d2,14); open(10,true); frontend::request_control_focus(queue_,0x28200000);
    } else connection(false);
}

inline void MultiplayerMenu::connection(bool host) {
    frontend::MultiplayerLaunchRequest request;
    request.mode=host?frontend::MultiplayerLaunchRequest::Mode::host:
                      frontend::MultiplayerLaunchRequest::Mode::client;
    request.settings=settings_; request.server=joining_; request.password=join_password_;
    request.validate(); startup_.multiplayer_request()=std::move(request);
    console_=true; console_elapsed_=0; console_delay_=host?1500:300;
    caption(0x34100000,host?0x91b:0x91c,25); open(13,true);
}

inline void MultiplayerMenu::back(unsigned slot) {
    for (const auto& edit:edits_) if ((edit.first>>26)==slot) edit.second->deactivate(queue_);
    if (slot==8) { cancel(); save(); servers_.clear(); selected_.reset(); pop(); }
    else if (slot==9) { commit_create(); save(); pop(); }
    else if (slot==10 || slot==14) { join_password_.clear(); joining_.reset(); pop(true); }
    else if (slot==13) { cancel(); console_=false; joining_.reset(); join_password_.clear();
        startup_.multiplayer_request().reset(); pop(true); }
    else { cancel(); pop(); }
}

inline void MultiplayerMenu::clicked(unsigned id) {
    if (id>=0x20300000 && id<=0x20700000) {
        const auto sort=(id-0x20200000)>>20;
        if (settings_.sort==sort) settings_.descending=!settings_.descending;
        else settings_.sort=static_cast<std::uint8_t>(sort);
        paint_browser(); return;
    }
    switch (id) {
    case 0x20200000: settings_.descending=!settings_.descending; paint_browser(); break;
    case 0x20a00000: refresh(true); break;
    case 0x20b00000: refresh(); break;
    case 0x20c00000:
        set_text(0x38200000,""); set_text(0x38300000,"11001"); open(14,true);
        frontend::request_control_focus(queue_,0x38200000); break;
    case 0x20d00000: cancel(); open(11); break;
    case 0x20e00000: case 0x30400000: join(); break;
    case 0x20f00000: cancel(); open(9); break;
    case 0x21000000: cancel(); if (selected_) open(12); break;
    case 0x21100000: back(8); break;
    case 0x21600000:
        if (selected_) {
            auto& list=saved_.favorites;
            const auto found=std::find_if(list.begin(),list.end(),[&](const auto& value){return value.ipv4==selected_->ipv4 && value.port==selected_->port;});
            if (found==list.end()) list.push_back(*selected_); else list.erase(found);
            if (favorites_) servers_=list;
            save(); paint_browser();
        }
        break;
    case 0x21700000:
        cancel(); if (!favorites_) saved_.ordinary=servers_;
        favorites_=!favorites_; selected_.reset(); servers_=favorites_?saved_.favorites:saved_.ordinary; paint_browser(); break;
    case 0x24900000: start_host(); break;
    case 0x24a00000: back(9); break;
    case 0x28300000:
        edits_.at(0x28200000)->deactivate(queue_);
        join_password_=edits_.at(0x28200000)->value();
        if (!join_password_.empty()) { pop(true); connection(false); }
        break;
    case 0x28400000: back(10); break;
    case 0x2c900000: commit_filters(); pop(); break;
    case 0x2ca00000: back(11); break;
    case 0x30300000: refresh_information(); break;
    case 0x30500000: back(12); break;
    case 0x34400000: back(13); break;
    case 0x38400000: add_address(); break;
    case 0x38500000: back(14); break;
    default: break;
    }
}

inline MultiplayerMenu::Definition MultiplayerMenu::create_definition() {
    using namespace frontend;
    Definition out{"CREATE SERVER",0x24000000,"create server",{}};
    auto hostname=edit(0x24100000,"bhostname");
    hostname.initialization.push_back(observer_control_event(0,0x0400b000,this,&hostname_changed));
    out.controls.push_back(std::move(hostname));
    auto style=selector(0x24200000,"style",{2891,2892,2893});
    style.initialization.push_back(observer_control_event(0,0x04010000,this,&changed_style));
    out.controls.push_back(std::move(style));
    auto* panel=scene_.create_group_frame(module_,model(9),"MP_MAPS_PANEL",{0,0,0});
    Control map_panel{0x24300000,"MP_MAPS_PANEL",K::container,nullptr,nullptr,MenuControlRole::none,{}};
    auto* available=scene_.control_frame("bmap_A",module_,false,model(9));
    auto* selected=scene_.control_frame("bmap_B",module_,false,model(9));
    auto* scroll=scene_.control_frame("scroll00",module_,false,model(9));
    auto* scroll_selected=scene_.control_frame("scroll13",module_,false,model(9));
    clone_.link(scroll,panel,1); clone_.link(scroll_selected,panel,1);
    scene_.set_screen_visible(module_,available,false); scene_.set_screen_visible(module_,selected,false);
    map_panel.children.push_back({0x24301000,"scroll00",K::slider,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x24302000,0,0,{}}}});
    const auto list=[&](unsigned id,const char* name,EngineFrame* prototype,unsigned slider) {
        scene_.create_group_frame(module_,panel,name,{0,0,0});
        std::array<float,3> anchor{};
        std::memcpy(anchor.data(),reinterpret_cast<const char*>(prototype)+0xb0,12);
        return Control{id,name,K::movie_list,nullptr,nullptr,MenuControlRole::none,
            {{0,0x0100000b,slider,0,0,{}},{0,0x01000028,11,0,0,{}},
             {0,0x0100002a,0,0x3d09a027,0,{}},
             {0,0x0100006b,0,0,0,std::make_shared<std::array<float,3>>(anchor)}}};
    };
    const char* groups[]{"gr_mp_deathmatch","gr_mp_zonegame","gr_mp_hiddenmaps","gr_mp_coopmaps"};
    for (unsigned group=0;group<4;++group) {
        const unsigned id=0x24302000+group*0x1000;
        auto definition=list(id,groups[group],available,0x24301000);
        auto* root=scene_.control_frame(groups[group],module_,false,panel);
        unsigned row=0;
        for (std::size_t i=0;i<maps_.size();++i) if (maps_[i].style==group) {
            const auto child=id|(++row<<4);
            auto control=clone_row(root,available,child,"MPAvailable"+std::to_string(group)+"_"+std::to_string(row));
            control.initialization.push_back(observer_control_event(0,0x04013000,this,&map_action));
            for (unsigned tag:{0x0400a000u,0x04008000u,0x04009000u})
                control.initialization.push_back(observer_control_event(0,tag,this,&map_drag));
            definition.children.push_back(std::move(control)); available_maps_[child]=i;
        }
        if (group) definition.initialization.push_back({0,0x01000004,0,0,0,{}});
        map_panel.children.push_back(std::move(definition));
    }
    map_panel.children.push_back({0x24306000,"scroll13",K::slider,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x24307000,0,0,{}}}});
    auto selected_list=list(0x24307000,"gr_mp_sel_list",selected,0x24306000);
    auto* selected_root=scene_.control_frame("gr_mp_sel_list",module_,false,panel);
    for (unsigned row=1;row<=50;++row) {
        auto control=clone_row(selected_root,selected,0x24307000|(row<<4),"MPMap"+std::to_string(row-1));
        control.initialization.push_back(observer_control_event(0,0x04013000,this,&map_action));
        for (unsigned tag:{0x0400a000u,0x04008000u,0x04009000u})
            control.initialization.push_back(observer_control_event(0,tag,this,&map_drag));
        selected_list.children.push_back(std::move(control));
    }
    selected_list.initialization.push_back({0,0x02000085,0,0,0,{}});
    map_panel.children.push_back(std::move(selected_list));
    out.controls.push_back(std::move(map_panel));
    for (const auto item:{std::pair<unsigned,const char*>{0x24400000,"internet server"},{0x24500000,"dedicated server"}})
        out.controls.push_back({item.first,item.second,K::toggle,nullptr,nullptr,MenuControlRole::none,
            {caption_definition(13,item.first==0x24400000?0x94d:0x94e)}});
    out.controls.push_back({0x24600000,"scroll01",K::slider,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x24700000,0,0,{}}}});
    auto* settings=scene_.create_group_frame(module_,model(9),"gr_setings",{0,0,0});
    auto* number=scene_.control_frame("bshort00",module_,false,model(9));
    auto* toggle=scene_.control_frame("yes_no_tmpl",module_,false,model(9));
    auto* password=scene_.control_frame("binput",module_,false,model(9));
    scene_.set_screen_visible(module_,number,false); scene_.set_screen_visible(module_,toggle,false);
    scene_.set_screen_visible(module_,password,false);
    scene_.set_screen_visible(module_,scene_.control_frame("combo_tmpl",module_,false,model(9)),false);
    std::array<float,3> origin{}; std::memcpy(origin.data(),reinterpret_cast<const char*>(number)+0xb0,12);
    Control fields{0x24700000,"gr_setings",K::movie_list,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x24600000,0,0,{}},{0,0x01000028,7,0,0,{}},{0,0x0100002a,0,0x3d23d70a,0,{}},
         {0,0x0100006b,0,0,0,std::make_shared<std::array<float,3>>(origin)}}};
    struct Field {unsigned row;const char* name;unsigned label;int minimum,maximum;unsigned capacity;};
    const Field numbers[]{
        {2,"player_num",0x93c,2,32,3},{5,"Frag_Limit",0x93d,0,999,4},
        {6,"Round_Limit",0x93e,1,99,3},{7,"Round_Count",0x93f,1,999,4},
        {8,"WarmUp_Time",0x940,0,99,3},{10,"Respawn_Time",0x941,5,99,3},
        {11,"Spawn_Protection",0x948,0,15,3},{12,"Self_Damage",0x949,0,999,4},
        {18,"bport",0x94a,1024,65530,5}};
    const char* toggle_names[]{"Allow_respawn","Friendly_Fire","Auto_Team_Balence","3person_view","yn_crosshair"};
    const unsigned toggle_rows[]{9,13,14,15,16},toggle_labels[]{0x942,0x943,0x944,0x945,0x4ef4};
    for (unsigned row=2;row<=18;++row) {
        const auto n=std::find_if(std::begin(numbers),std::end(numbers),[&](const Field& field){return field.row==row;});
        const auto t=std::find(std::begin(toggle_rows),std::end(toggle_rows),row);
        if (n==std::end(numbers) && t==std::end(toggle_rows) && row!=17) continue;
        const auto id=0x24700000|(row<<12);
        auto clone=clone_row(settings,row==17 || row==18?password:t!=std::end(toggle_rows)?toggle:number,id,
            n!=std::end(numbers)?n->name:row==17?"bpassword":toggle_names[t-std::begin(toggle_rows)]);
        if (n!=std::end(numbers)) clone=multiplayer_numeric_definition(id,clone.frame,n->label,n->minimum,n->maximum,n->capacity);
        else if (row==17) {
            clone=multiplayer_numeric_definition(id,clone.frame,0x946,0,0);
            clone.initialization={caption_definition(13,0x946)};
            Event exclude{0,0x0100009b,0,0,0,{}}; exclude.text=shared(">_"); clone.initialization.push_back(exclude);
        } else {
            clone.kind=K::labeled_toggle;
            clone.normal=nullptr; clone.active=nullptr;
            clone.initialization={caption_definition(13,toggle_labels[t-std::begin(toggle_rows)])};
            clone.initialization.push_back(observer_control_event(0,0x04010000,this,&changed_rule));
        }
        fields.children.push_back(std::move(clone));
    }
    out.controls.push_back(std::move(fields));
    out.controls.push_back(selector(0x24800000,"voice chat",{2880,2881,2882,2883,2884,2885,2886,2887}));
    out.controls.push_back(button(0x24900000,"bstart",0x935));
    out.controls.push_back(button(0x24a00000,"bback",0x936));
    const char* text_names[]{"text_screen","text_hostname and style","text_available maps checklist","text_server options",
        "text_voice chat","text_h&dversion","text_server type"};
    const unsigned labels[]{0x92f,0x930,0x932,0x933,0x947,0x7d6,0x94c};
    for (unsigned i=0;i<7;++i) out.controls.push_back(text_definition(0x24b00000+i*0x100000,text_names[i],
        {caption_definition(i==0?10:i==5?40:11,labels[i])}));
    Control preset{0x25200000,"maplist preset",K::combo,nullptr,nullptr,MenuControlRole::none,
        {{0,0x01000081,12,500,0,{}}}};
    for (unsigned i=0;i<5;++i) preset.initialization.push_back({0,0x01000017,19,0x4ee8+i,0,{}});
    preset.initialization.push_back(observer_control_event(0,0x0400e000,this,&changed_preset));
    PreMissionChoice::children(preset); out.controls.push_back(std::move(preset));
    out.controls.push_back(text_definition(0x25300000,"text_maplist preset",{caption_definition(11,0x4eed)}));
    out.controls.push_back(text_definition(0x25400000,"text_maplist",{caption_definition(11,0x4eee)}));
    return out;
}

inline void MultiplayerMenu::fill_create() {
    filling_=true; displayed_style_=settings_.style%3;
    set_text(0x24100000,settings_.hostname);
    selected_value(0x24200000,displayed_style_);
    selected_value(0x24400000,internet_); selected_value(0x24500000,settings_.dedicated);
    selected_value(0x24800000,std::min(settings_.voice,7u));
    const auto& values=settings_.rules[displayed_style_].values;
    constexpr unsigned numeric_rows[]{2,5,6,7,8,10,11,12};
    constexpr unsigned numeric_indices[]{0,1,2,3,4,5,10,11};
    for (unsigned i=0;i<8;++i) numeric_.at(0x24700000|(numeric_rows[i]<<12))->value(shared(std::to_string(values[numeric_indices[i]])));
    constexpr unsigned toggle_rows[]{9,13,14,15,16},toggle_indices[]{6,7,8,9,19};
    for (unsigned i=0;i<5;++i) selected_value(0x24700000|(toggle_rows[i]<<12),values[toggle_indices[i]]!=0);
    numeric_.at(0x24711000)->value(shared(settings_.password));
    numeric_.at(0x24712000)->value(shared(std::to_string(settings_.port)));
    queue_.dispatch_immediate({0x25200000,0x01000076,std::min(settings_.preset,4u),0,0,{}});
    for (const auto& binding:available_maps_) set_text(binding.first,maps_[binding.second].name);
    refresh_rule_eligibility();
    fill_maps(); filling_=false;
}

inline void MultiplayerMenu::refresh_rule_eligibility() {
    const bool respawn=selected_value(0x24709000)!=0;
    const bool friendly=selected_value(0x2470d000)!=0;
    for (unsigned row:{5u,6u,9u,10u,12u,13u,14u})
        enabled(0x24700000|(row<<12),
            frontend::multiplayer_rule_enabled(displayed_style_,row,respawn,friendly));
}

inline void MultiplayerMenu::commit_create() {
    edits_.at(0x24100000)->deactivate(queue_);
    for (const auto& field:numeric_) if ((field.first&0xfff00000u)==0x24700000) field.second->deactivate(queue_);
    settings_.hostname=edits_.at(0x24100000)->value();
    settings_.internet=selected_value(0x24400000)!=0; settings_.dedicated=selected_value(0x24500000)!=0;
    settings_.voice=selected_value(0x24800000);
    auto values=settings_.rules[displayed_style_].values;
    constexpr unsigned rows[]{2,5,6,7,8,10,11,12},indices[]{0,1,2,3,4,5,10,11};
    for (unsigned i=0;i<8;++i) values[indices[i]]=numeric_value(numeric_.at(0x24700000|(rows[i]<<12))->value());
    constexpr unsigned toggle_rows[]{9,13,14,15,16},toggle_indices[]{6,7,8,9,19};
    for (unsigned i=0;i<5;++i) values[toggle_indices[i]]=selected_value(0x24700000|(toggle_rows[i]<<12))!=0;
    settings_.password=numeric_.at(0x24711000)->value();
    const auto port=integer(numeric_.at(0x24712000)->value());
    if (port<1024 || port>65530) throw std::runtime_error("Enter a port from1024 to65530.");
    settings_.port=static_cast<unsigned>(port); settings_.rules[displayed_style_].values=values;
    settings_.style=std::min(selected_value(0x24200000),2u);
}

inline void MultiplayerMenu::style_changed() {
    if (filling_) return;
    commit_create(); fill_create();
}

inline void MultiplayerMenu::preset_changed() {
    if (filling_) return;
    Event query{0x25200000,0x0200002f,0,0,0,{}}; queue_.query_control(query);
    if (query.argument<5) { settings_.preset=query.argument; fill_maps(); }
}

inline void MultiplayerMenu::fill_maps() {
    const auto style=settings_.style%3;
    const auto& selected=settings_.rotations[std::min(settings_.preset,4u)][style];
    for (unsigned i=0;i<4;++i) queue_.enqueue({0x24302000+i*0x1000,i==style?0x01000003u:0x01000004u,0,0,0,{}});
    queue_.enqueue({0x24301000,0x0100000b,0x24302000+style*0x1000,0,0,{}});
    queue_.enqueue({0x24302000+style*0x1000,0x0200002c,0,0,0,{}});
    for (unsigned i=0;i<50;++i) {
        const auto id=0x24307000|((i+1)<<4);
        if (i<selected.size()) set_text(id,selected[i]);
        queue_.enqueue({0x24307000,i<selected.size()?0x02000083u:0x02000084u,id,0,0,{}});
    }
    enabled(0x24900000,!settings_.hostname.empty() && !selected.empty() && !maps_.empty());
}

inline void MultiplayerMenu::map_selected(unsigned id) {
    focused_map_=id;
    for (const auto& row:map_rows_) if (row.first!=id) row.second->deselect_row();
}

inline void MultiplayerMenu::toggle_map(unsigned id) {
    auto& selected=settings_.rotations[std::min(settings_.preset,4u)][settings_.style%3];
    if ((id&0xfffff000u)==0x24307000) {
        const auto row=((id&0xff0u)>>4);
        if (row && row<=selected.size()) selected.erase(selected.begin()+row-1);
    } else {
        const auto found=available_maps_.find(id);
        if (found==available_maps_.end() || maps_[found->second].style!=settings_.style%3) return;
        if (selected.size()==50) { fail("A map list can contain at most50 maps."); return; }
        selected.push_back(maps_[found->second].name);
    }
    fill_maps();
}

inline void MultiplayerMenu::move_drag() {
    if (!drag_frame_ || !context_) return;
    const auto& cursor=context_->input.pointer.cursor();
    const auto delta=frontend::cursor_frame_position(cursor.x()-drag_pointer_[0],cursor.y()-drag_pointer_[1],800,600);
    scene_.position_menu_model(module_,drag_frame_->get(),drag_position_[0]+delta[0],drag_position_[1]+delta[1],delta[2]);
    scene_.update_screen_frame(module_,drag_frame_->get());
    auto bounds=drag_bounds_;
    bounds[0]+=delta[0]; bounds[4]+=delta[0];
    bounds[1]+=delta[1]; bounds[5]+=delta[1];
    drag_text_->projected_bounds(bounds);
    if (!drag_available_ && frontend::multiplayer_map_side(frontend::cursor_frame_position(cursor.x(),cursor.y(),800,600))==
            frontend::MultiplayerMapSide::selected)
        reorder_drag(pick_maps());
}

inline unsigned MultiplayerMenu::pick_maps() {
    const auto& cursor=context_->input.pointer.cursor();
    if (!drag_frame_) return scene_.pick_model(module_,cursor.x(),cursor.y(),800,600,model(9));
    // The original transient is linked outside the Create model's pick tree.
    // Exclude the native clone explicitly while querying that same tree.
    scene_.set_screen_visible(module_,drag_frame_->get(),false);
    try {
        const auto target=scene_.pick_model(module_,cursor.x(),cursor.y(),800,600,model(9));
        scene_.set_screen_visible(module_,drag_frame_->get(),true); return target;
    } catch (...) { scene_.set_screen_visible(module_,drag_frame_->get(),true); throw; }
}

inline void MultiplayerMenu::reorder_drag(unsigned target) {
    if ((target&0xfffff000u)!=0x24307000) return;
    const auto row=(target&0xff0u)>>4;
    if (!row) return;
    auto& selected=settings_.rotations[std::min(settings_.preset,4u)][settings_.style%3];
    if (frontend::move_multiplayer_map(selected,drag_index_,row-1)) {
        drag_index_=row-1;
        for (const auto& control:map_rows_) control.second->display_row_selection(false);
        focused_map_=target; fill_maps();
    }
}

inline void MultiplayerMenu::drag_map(unsigned id,unsigned tag) {
    auto& selected=settings_.rotations[std::min(settings_.preset,4u)][settings_.style%3];
    if (tag==0x0400a000) {
        cancel_drag();
        const auto found=available_maps_.find(id);
        drag_available_=found!=available_maps_.end();
        if (drag_available_) drag_map_name_=maps_[found->second].name;
        else {
            const auto row=(id&0xff0u)>>4;
            if (!row || row>selected.size()) return;
            drag_index_=row-1; drag_map_name_=selected[drag_index_];
        }
        auto* source=scene_.control_frame("normal*",module_,false,map_frames_.at(id));
        drag_bounds_=scene_.label_bounds("normal*","normal*",module_,false,source);
        auto* row=map_frames_.at(id);
        drag_frame_=frontend::clone_menu_frame(clone_,row,model(9));
        auto* normal=scene_.control_frame("normal*",module_,false,drag_frame_->get());
        auto* active=scene_.control_frame("actived*",module_,false,drag_frame_->get());
        scene_.set_screen_visible(module_,normal,true); scene_.set_screen_visible(module_,active,false);
        drag_position_=scene_.frame_world_position(module_,row);
        const auto& cursor=context_->input.pointer.cursor(); drag_pointer_={cursor.x(),cursor.y()};
        const unsigned no_target=0; std::memcpy(reinterpret_cast<char*>(drag_frame_->get())+0xec,&no_target,4);
        scene_.set_screen_visible(module_,drag_frame_->get(),true);
        Control text_definition{0,"normal*",K::text,nullptr,nullptr,frontend::MenuControlRole::none,{}};
        drag_text_=std::make_unique<MenuTextObject>(scene_,module_,text_definition,false,[this](unsigned label){
            return shared(frontend::lookup_localized_text(startup_.text(),static_cast<int>(label)));});
        drag_text_->bind_frame(normal); drag_text_->object().inset=10;
        drag_text_->object().set(shared(drag_map_name_),13,0); move_drag(); return;
    }
    if (tag==0x04008000) { move_drag(); return; }
    if (!drag_frame_) return;
    const auto& cursor=context_->input.pointer.cursor();
    const auto side=frontend::multiplayer_map_side(frontend::cursor_frame_position(cursor.x(),cursor.y(),800,600));
    drag_text_.reset(); drag_frame_.reset();
    if (!drag_available_ && side==frontend::MultiplayerMapSide::available && drag_index_<selected.size())
        selected.erase(selected.begin()+drag_index_);
    else if (drag_available_ && side==frontend::MultiplayerMapSide::selected) {
        if (selected.size()<50) {
            selected.push_back(drag_map_name_); drag_index_=selected.size()-1;
            reorder_drag(pick_maps());
        } else fail("A map list can contain at most50 maps.");
    }
    drag_map_name_.clear(); fill_maps();
}

inline void MultiplayerMenu::start_host() {
    commit_create();
    const auto& selected=settings_.rotations[std::min(settings_.preset,4u)][settings_.style%3];
    if (settings_.hostname.empty() || selected.empty()) { fail("Enter a server name and choose at least one map."); return; }
    for (const auto& name:selected)
        if (std::none_of(maps_.begin(),maps_.end(),[&](const auto& map){return map.style==settings_.style && map.name==name;})) {
            fail("A selected map is unavailable."); return;
        }
    save();
    if (settings_.dedicated) { fail("The reconstructed dedicated server is unavailable in this build."); return; }
    connection(true);
}

inline MultiplayerMenu::Definition MultiplayerMenu::information_definition() {
    using namespace frontend;
    Definition out{"GAME INFORMATION",0x30000000,"gameinfo",{}};
    Control players{0x30100000,"LB_Players",K::profile_list,nullptr,nullptr,MenuControlRole::none,
        {{0,0x02000073,0,0,0,{}},{0,0x01000005,0,0,0,{}}}};
    players.children.push_back({0x30100008,"text*",K::records,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x30100004,0,0,{}}}});
    players.children.push_back({0x30100004,"scroll*",K::slider,nullptr,nullptr,MenuControlRole::none,
        {{0,0x0100000b,0x30100008,0,0,{}}}});
    out.controls.push_back(std::move(players));
    out.controls.push_back(text_definition(0x30200000,"text_01"));
    out.controls.push_back(button(0x30300000,"brefresh status",0xa05));
    out.controls.push_back(button(0x30400000,"bjoin game",0xa06));
    out.controls.push_back(button(0x30500000,"breturn",0xa07));
    out.controls.push_back(text_definition(0x30600000,"text_screen",{caption_definition(10,0xa08)}));
    out.controls.push_back(text_definition(0x30700000,"text_h&dversion",{caption_definition(40,0x7d6)}));
    out.controls.push_back(text_definition(0x30800000,"text_team status",{caption_definition(11,0xa09)}));
    out.controls.push_back(text_definition(0x30900000,"text_"));
    out.controls.push_back(text_definition(0x30a00000,"allied"));
    out.controls.push_back(text_definition(0x30b00000,"axis"));
    return out;
}

inline void MultiplayerMenu::refresh_information() {
    cancel(); information_.reset(); information_pending_=false;
    for (auto& entry:information_text_) entry.second->column_rows.reset();
    for (unsigned id:{0x30900000u,0x30a00000u,0x30b00000u})
        information_text_.at(id)->object().clear();
    queue_.enqueue({0x30100000,0x02000018,0,0,0,{}});
    if (!selected_) { set_text(0x30200000,"The selected server is no longer available.",2); return; }
    set_text(0x30200000,"Reading Data from server...",2);
    const auto submission=discovery_.start(&*selected_,1,startup_.wide_characters());
    if (!submission.accepted) { set_text(0x30200000,"Could not query the selected server.",2); return; }
    information_pending_=active_=true; last_response_=clock_;
}

inline void MultiplayerMenu::fill_information() {
    if (!information_ || !selected_) return;
    const auto localized=[this](unsigned id) {
        return frontend::lookup_localized_text(startup_.text(),static_cast<std::int32_t>(id));
    };
    auto& players=*information_text_.at(0x30100008);
    players.column_rows.emplace();
    queue_.enqueue({0x30100000,0x02000018,0,0,0,{}});
    //0062F960 traverses the original vector backwards, skipping empty names.
    for (auto it=information_->players.rbegin();it!=information_->players.rend();++it) {
        const auto& player=*it;
        if (!player.name[0]) continue;
        Event append{0x30100000,0x01000016,19,0,0,{}};
        const auto name=peer_text(frontend::multiplayer_text(player.name));
        append.text=shared(name);
        queue_.enqueue(append);
        MenuTextRow row;
        if (player.word24<=2)
            row.push_back({player.word24==0?"!":"=",4,.02f,0,.06f,false,
                player.word24==2?0x00000000u:0xffffffffu});
        row.push_back({name,2,.04f,.06f,.18f});
        row.push_back({std::to_string(player.word04),2,.04f,.65f,.05f});
        row.push_back({std::to_string(player.score),2,.04f,.98f,.05f,true});
        players.column_rows->push_back(std::move(row));
    }
    auto& header=*information_text_.at(0x30900000);
    header.column_line_height=12;
    header.column_rows=std::vector<MenuTextRow>{{
        {localized(0xa0a),3,.02f,0,.18f},
        {localized(0xa21),3,.02f,.65f,.05f},
        {localized(0xa22),3,.02f,.98f,.05f,true}}};
    auto& body=*information_text_.at(0x30200000);
    body.column_line_height=11.4f;
    body.column_rows.emplace();
    auto& rows=*body.column_rows;
    const auto heading=[&](unsigned label) {rows.push_back({{localized(label),3,.019f}});};
    const auto field=[&](unsigned label,std::string value) {
        rows.push_back({{localized(label),3,.019f,0,.65f},{std::move(value),3,.019f,.65f,.35f}});
    };
    const auto rule=[&](unsigned label,unsigned index,bool boolean=false) {
        const auto value=information_->rules.at(index);
        field(label,!value?std::string{}:boolean?localized(*value?0x7fd:0x7fe):std::to_string(*value));
    };
    heading(0xa0b); heading(0x7d2);
    field(0xa0c,peer_text(frontend::multiplayer_text(selected_->host)));
    std::string address;
    for (unsigned i=0;i<4;++i) {if (i) address+='.'; address+=std::to_string((selected_->ipv4>>(8*i))&255);}
    field(0xa0d,address);
    field(0xa0e,std::to_string(selected_->ping));
    field(0xa0f,std::to_string(selected_->capacity)+" / "+std::to_string(selected_->players));
    field(0xa10,localized(selected_->dedicated?0x7fd:0x7fe));
    field(0xa12,localized(selected_->manual?0x7fd:0x7fe));
    field(0xa11,localized(selected_->password?0x7fd:0x7fe));
    heading(0x7d2); heading(0xa14); heading(0x7d2);
    constexpr unsigned modes[]{0xb4b,0xb4c,0xb4d,0x4e21}; //006AC8D0 table0084297C.
    field(0xa15,localized(selected_->style<4?modes[selected_->style]:0xb4e));
    if (selected_->style==3) {
        const auto difficulty=information_->rules[14];
        field(0x501e,difficulty?localized(0x5014+*difficulty):std::string{});
        rule(0x501f,18,true); rule(0x5020,15);
    }
    field(0xa16,peer_text(frontend::multiplayer_text(selected_->map)));
    for (unsigned i=0;i<5;++i) rule(0xa17+i,1+i);
    rule(0xa1c,6,true); rule(0xa1d,7,true); rule(0x5023,19);
    rule(0xa1e,8,true); rule(0xa1f,9,true); rule(0xa20,10);
    rule(0x5021,16,true); rule(0x5022,13,true);
    for (unsigned id:{0x30a00000u,0x30b00000u}) {
        auto& team=*information_text_.at(id);
        team.column_rows.emplace(); team.column_line_height=24;
        if (id==0x30b00000 && selected_->style==3) continue;
        team.column_rows->push_back({
            {localized(id==0x30a00000?0xa3c:0xa3d),2,.04f,.02f,.18f},
            {std::to_string(information_->team_scores[id==0x30a00000?1:0]),2,.04f,.98f,.05f,true}});
    }
}

} // namespace hd2::host
#endif
