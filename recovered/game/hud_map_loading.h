#ifndef HD2_GAME_HUD_MAP_LOADING_H
#define HD2_GAME_HUD_MAP_LOADING_H
#include "game/hud_text_loading.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
namespace hd2::game {
//004A4AE0's depth differs from006449D0: null map gives1; required camera,
//map WORLD update BEFORE camera WORLD update; extended comparison before store.
template<class Frames>float hud_map_depth(Frames& frames,typename Frames::Frame scene,typename Frames::Frame visual) {
    if(!visual)return 1.f;
    const auto camera=frames.hud_active_camera(scene);
    if(!camera)throw std::runtime_error("HUD map depth requires actual HUD camera");
    const auto clips=frames.hud_camera_clips(camera);
    const auto visual_matrix=frames.actor_world_matrix(visual);
    const auto camera_matrix=frames.actor_world_matrix(camera);
    const auto depth=(std::abs(static_cast<long double>(visual_matrix[14])-camera_matrix[14])-.1f-clips[0])/
        (static_cast<long double>(clips[1])-clips[0]);
    return depth<0||depth>1?-1.f:static_cast<float>(depth);
}
//004A4AE0/004A4F60 and loading record helpers. HUD scene and actual world
//registry/objective producers must outlive borrowers. MapWorld supplies the
//SAME0089B3B0 hud_actor_identities(), type37 groupA4, and missionA8 objective vector;
//no empty successful world service is supplied. Runtime map interactions absent.
template<class Frames,class Factory,class Actor,class Objective>
class HudMapLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using Owner=decltype(std::declval<Frames&>().create_hud_info_model());
    struct Allied {std::optional<Owner> visual;std::unique_ptr<Text> text;std::uint8_t flag=0;};
    struct Zone {Frame root{},allied{},axis{};Actor* actor=nullptr;std::uint32_t tag=0,state=2;};
    struct Goal {Frame root{},allied{},axis{};Objective* objective=nullptr;std::unique_ptr<Text> text;
        std::uint32_t state0c=0,state10=0;}; //Defined guards on native ignored-false/unwritten fields.
    struct EditorGoal {Frame root{},allied{},axis{};Objective* objective=nullptr;std::int32_t state=-1;};
    HudMapLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory){}
    ~HudMapLoading(){close();}
    HudMapLoading(const HudMapLoading&)=delete;HudMapLoading& operator=(const HudMapLoading&)=delete;
    template<class Configuration,class World,class Pointer>
    bool bind(Frame scene,const std::string& selected_mission,void* context,std::uint32_t mode,
              Configuration& configuration,World& world,Pointer& pointer) {
        if(bound_)throw std::logic_error("HUD map rebound before borrower retirement");
        require(scene);bound_=true;scene_=scene;context14_=context;
        if(selected_mission.size()>240)throw std::runtime_error("Mission map name exceeds original path buffer");
        model_.emplace(frames_.create_hud_info_model());if(!model_->get())throw std::runtime_error("HUD map factory returned null");
        std::array<std::uint32_t,3> saved{};constexpr std::array<unsigned,3> states{{0x15,0x19,0x11}};
        for(unsigned n=0;n!=3;++n)saved[n]=frames_.hud_driver_state(states[n])!=0;
        frames_.hud_driver_state(0x15,0);frames_.hud_driver_state(0x19,1);frames_.hud_driver_state(0x11,0);
        std::int32_t status;
        try{status=frames_.load_hud_map_model(model_->get(),("missions\\"+selected_mission+"\\map").c_str());}
        catch(...){for(unsigned n=0;n!=3;++n)frames_.hud_driver_state(states[n],saved[n]);throw;}
        for(unsigned n=0;n!=3;++n)frames_.hud_driver_state(states[n],saved[n]);
        if(status!=0){center_=map_=Frame{};model_.reset();return false;} //Native tolerated absent map, not successful load.
        center_=frames_.actor_find(model_->get(),"center",0xffff);
        map_=frames_.actor_find(model_->get(),"map*",0x10001);
        if(center_){frames_.hud_inventory_link_root(center_,scene);frames_.hud_position(center_,{{.5f,-.375f,-1.f}});frames_.update_scene_object(center_);}
        scale_=0;
        if(map_){const auto properties=frames_.car_user_data(map_);const auto start=properties.find("scale");
            if(start!=std::string::npos)scale_=static_cast<float>(std::strtod(properties.c_str()+start+5,nullptr));}
        constexpr std::array<const char*,5> names{{"allied_active","vip","mapcamera","enemy","objective"}};
        for(unsigned n=0;n!=5;++n){auto& frame=markers_[n];frame=frames_.hud_find(scene,names[n],0xffff);require(frame);
            frames_.link_scene_object(frame,map_);frames_.object_visible(frame,false);frames_.update_scene_object(frame);}
        if((mode&7u)==7u){kind_=0;allied_count_=6;}
        else {const auto style=configuration.hud_style224();switch(style){case 0:kind_=1;allied_count_=0;break;case 1:kind_=2;allied_count_=32;break;case 2:kind_=3;allied_count_=32;break;case 3:kind_=4;allied_count_=6;break;default:break;}}
        bind_allied();
        if(kind_==0)bind_editor_goals(world);
        else if(kind_==2)bind_zones(world);
        else if(kind_==3||kind_==4){bind_zones(world);bind_goals(world);}
        depth_=hud_map_depth(frames_,scene_,map_);
        hide(pointer);
        return true;
    }
    template<class Pointer>void hide(Pointer& pointer){ //004A5630(false,false); missing center is a genuine no-op.
        if(!center_)return;
        flag24_=0;frames_.object_visible(center_,false);require(map_);frames_.object_visible(map_,false);
        if(kind_!=1&&kind_!=0)pointer.hide_selected_pointer();
    }
    void close()noexcept { //004A5190 then shrink dynamic lists, kind-specific records, model LAST.
        for(unsigned n=0;n!=allied_count_;++n){allied_[n].visual.reset();allied_[n].text.reset();allied_[n].flag=0;}
        while(!enemy_models_.empty())enemy_models_.pop_back();
        while(!objective_models_.empty())objective_models_.pop_back();
        if(kind_==0){for(auto& goal:editor_goals_)goal.objective=nullptr;editor_goals_.clear();}
        else if(kind_==2){zones_.clear();select_zone_=Frame{};}
        else if(kind_==3||kind_==4){zones_.clear();select_zone_=Frame{};for(auto& goal:goals_){goal.objective=nullptr;goal.text.reset();}goals_.clear();}
        model_.reset();
    }
    void draw(){ //004A6FE0→004A7910/004A7EB0: goal text FIRST, then reverse allied.
        //004A7EB0 draws text; it does not update objective state or visibility.
        for(auto& goal:goals_)if(goal.text&&goal.state0c!=4)factory_.hud_draw_text(*goal.text);
        for(auto n=allied_count_;n;){auto& entry=allied_[--n];if(entry.flag){
            if(!entry.text)throw std::logic_error("Active HUD allied label has no actual text owner");
            factory_.hud_draw_text(*entry.text);
        }}
    }
    void discard_borrowers()noexcept{center_=map_=select_zone_=scene_=Frame{};markers_.fill(Frame{});context14_=nullptr;bound_=false;}
    Frame center()const{return center_;}Frame map()const{return map_;}float scale()const{return scale_;}float depth()const{return depth_;}
    std::int32_t kind()const{return kind_;}unsigned allied_count()const{return allied_count_;}
    const auto& allied()const{return allied_;}const auto& zones()const{return zones_;}const auto& goals()const{return goals_;}const auto& editor_goals()const{return editor_goals_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD map artwork is missing");}
    std::unique_ptr<Text> make_text(){auto owner=factory_.make_hud_text();if(!owner)throw std::runtime_error("HUD map text factory returned null");return owner;}
    void bind_allied(){ //004A4FD0 always looks up template even when count0; no retain/template mutation.
        const auto source=frames_.hud_find(scene_,"allied",0xffff);
        for(unsigned n=0;n!=32;++n){auto& entry=allied_[n];entry.flag=0;if(n>=allied_count_)continue;
            require(source);entry.visual.emplace(frames_.create_hud_inventory_visual());if(!entry.visual->get())throw std::runtime_error("HUD map allied factory returned null");
            const auto frame=entry.visual->get();frames_.hud_inventory_duplicate(frame,source);frames_.link_scene_object(frame,map_);
            frames_.object_visible(frame,false);frames_.update_scene_object(frame);entry.text=make_text();}
    }
    template<class World>void bind_zones(World& world){ //004A72C0.
        select_zone_=frames_.actor_find(model_->get(),"selectzone",1);if(!select_zone_)return;
        frames_.actor_link_preserving_world(select_zone_,center_);
        std::vector<Actor*> candidates;candidates.reserve(20);
        for(std::size_t n=32;n<world.hud_actor_identities().size();++n){auto* actor=world.hud_actor_identities()[n];
            if(!actor)throw std::runtime_error("HUD map registry has a null actor");
            if(actor->type()==37&&world.map_zone_group(*actor)==-1)candidates.push_back(actor);}
        if(candidates.size()>=16)return; //Native rejects ALL zones at16, not a truncated list.
        for(std::size_t n=0;n!=candidates.size();++n){auto* actor=candidates[n];const auto name=frames_.car_short_name(actor->frame());
            const auto root=frames_.actor_find(map_,name.c_str(),0x20);if(!root)continue;
            zones_.emplace_back();auto& zone=zones_.back();zone.root=root;zone.actor=actor;zone.tag=static_cast<unsigned>(n)+1000;
            const auto disabled=frames_.actor_find(root,"zone_disabled*",0x10001);
            zone.allied=frames_.actor_find(root,"allied*",0x10001);zone.axis=frames_.actor_find(root,"axis*",0x10001);
            require(disabled);frames_.hud_frame_tag(disabled,zone.tag);
            for(auto frame:{zone.allied,zone.axis})if(frame){frames_.hud_frame_tag(frame,zone.tag);frames_.object_visible(frame,false);frames_.update_scene_object(frame);}}
    }
    template<class World>void bind_goals(World& world){ //004A7520/004A7A90, ignored false when faction visuals absent.
        decltype(auto) objectives=world.map_objectives(); //Same missionA8 captured once, live vector count reread.
        for(std::size_t n=0;n<objectives.size();++n){const auto root=objective_root(n);if(!root)continue;
            goals_.emplace_back();auto& goal=goals_.back();goal.root=root;goal.objective=objectives[n];
            const auto caption=frames_.actor_find(root,"text_obj*",0x10001);
            goal.allied=frames_.actor_find(root,"allied_obj*",0x10001);goal.axis=frames_.actor_find(root,"axis_obj*",0x10001);
            if(!goal.allied||!goal.axis)continue;
            if(caption){goal.text=make_text();bind_hud_text_geometry(frames_,*goal.text,scene_,caption);frames_.object_visible(caption,false);}
            frames_.object_visible(goal.allied,false);frames_.object_visible(goal.axis,false);frames_.object_visible(root,false);
            goal.state0c=4;goal.state10=2;}
    }
    template<class World>void bind_editor_goals(World& world){ //004A7680/004A7ED0: only successful record is appended.
        decltype(auto) objectives=world.map_objectives();
        for(std::size_t n=0;n<objectives.size();++n){const auto root=objective_root(n);if(!root)continue;
            EditorGoal goal;goal.root=root;goal.objective=objectives[n];
            goal.allied=frames_.actor_find(root,"allied_obj*",0x10001);goal.axis=frames_.actor_find(root,"axis_obj*",0x10001);
            if(!goal.allied||!goal.axis)continue;
            frames_.object_visible(goal.allied,false);frames_.object_visible(goal.axis,false);frames_.object_visible(root,false);editor_goals_.push_back(goal);}
    }
    Frame objective_root(std::size_t n){std::array<char,40> name{};std::snprintf(name.data(),name.size(),"objective%u",static_cast<unsigned>(n)+1);return frames_.actor_find(map_,name.data(),0x20);}
    Frames& frames_;Factory& factory_;Frame scene_{},center_{},map_{},select_zone_{};void* context14_=nullptr;
    std::optional<Owner> model_;std::array<Allied,32> allied_;std::array<Frame,5> markers_{};
    std::vector<Owner> enemy_models_,objective_models_;std::vector<Zone> zones_;std::vector<Goal> goals_;std::vector<EditorGoal> editor_goals_;
    std::int32_t kind_=0;unsigned allied_count_=0;float scale_=0,depth_=1;std::uint8_t flag24_=0;bool bound_=false;
};
} //namespace hd2::game
#endif
