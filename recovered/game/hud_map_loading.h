#ifndef HD2_GAME_HUD_MAP_LOADING_H
#define HD2_GAME_HUD_MAP_LOADING_H
#include "game/hud_text_loading.h"
#include "game/hud_map_presentation.h"
#include "game/human_death_objective.h"
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
//no empty successful world service is supplied. Runtime sources borrow the SAME
//Players, Human, camera, item registry and live squad, only during each call.
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
    template<class World,class Pointer>void show(World& world,Pointer& pointer){ //004A5630(true,false).
        if(!center_)return;
        flag24_=0;frames_.object_visible(center_,true);require(map_);frames_.object_visible(map_,true);
        if(kind_!=1&&kind_!=0){pointer.select_pointer(0);refresh_zones(world);refresh_goals(world);}
    }
    template<class World>void refresh_zones(World& world){ //004A7180/004A7A70/004A7A20, fresh actual side and marker borrowers.
        for(std::size_t n=0;n<zones_.size();++n){
            auto* actor=zones_[n].actor;if(!actor)throw std::logic_error("HUD map zone lost its actual actor");
            const auto side=world.map_zone_side(*actor);if(zones_[n].state==side)continue;
            zones_[n].state=side;
            if(const auto allied=zones_[n].allied){frames_.object_visible(allied,side==1);require(zones_.at(n).allied);frames_.update_scene_object(zones_[n].allied);}
            if(const auto axis=zones_.at(n).axis){frames_.object_visible(axis,zones_[n].state==0);require(zones_.at(n).axis);frames_.update_scene_object(zones_[n].axis);}
        }
        bool selected=false;
        if(!world.live_squad().empty()&&world.live_squad().at(0).network_identity>0){
            for(auto n=zones_.size();n;){--n;auto* actor=zones_[n].actor;if(!actor)throw std::logic_error("HUD zone selection requires actual actor");
                if(actor->id10()!=world.live_squad().at(0).network_identity)continue;
                if(world.live_squad().at(0).specialization!=2&&static_cast<std::uint32_t>(world.live_squad().at(0).specialization)==zones_[n].state){
                    require(zones_[n].root);const auto matrix=frames_.actor_world_matrix(zones_[n].root);require(select_zone_);
                    frames_.hud_position(select_zone_,{{matrix[12],matrix[13],static_cast<float>(static_cast<long double>(matrix[14])-.5f)}});
                    frames_.hud_frame_tag(select_zone_,zones_.at(n).tag);selected=true;
                }else world.live_squad().at(0).network_identity=-1;
                break;
            }
        }
        require(select_zone_);frames_.object_visible(select_zone_,selected);require(select_zone_);frames_.update_scene_object(select_zone_);
    }
    template<class World>void refresh_goals(World& world){ //004A78E0, live forward count.
        for(std::size_t n=0;n<goals_.size();++n)refresh_goal(n,world);
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
    template<class Pointer>void hide(Pointer& pointer){ //004A5630(false,false), only actual nullable-center early return.
        if(!center_)return;flag24_=0;frames_.object_visible(center_,false);require(map_);frames_.object_visible(map_,false);
        if(kind_!=1&&kind_!=0)pointer.select_pointer(-1);
    }
    template<class Queries,class Pointer>void show(bool enabled,bool camera,Queries& queries,Pointer& pointer){ //004A5630.
        if(!center_)return;flag24_=camera;frames_.object_visible(center_,enabled);require(map_);frames_.object_visible(map_,enabled);
        if(kind_!=0&&kind_!=1){pointer.select_pointer(enabled?0:-1);if(enabled){update_zones(queries);update_goals(queries);}}
    }
    template<class Queries>void update(std::uint32_t,Queries& queries){ //004A6FA0, elapsed and visibility are unused.
        if(!map_)return;
        switch(kind_){case 0:update_offline(queries);break;case 1:update_neutral(queries);break;case 2:case 3:case 4:update_network(queries);break;default:break;}
    }
    template<class Queries,class Pointer,class Input>void process_controls(Queries& queries,Pointer& pointer,Input& input){ //004A7010.
        if(kind_==0||kind_==1||input.second(55)==0)return;
        select_zone(queries,pointer);input.consume_action(55); //Consume even for a genuine no-hit; not on failure.
    }
    void retire_actor(const Actor* actor)noexcept{for(auto& zone:zones_)if(zone.actor==actor)zone.actor=nullptr;} //Defined lifetime guard before actor destruction.
    auto& enemy_models()const{return enemy_models_;}auto& objective_models()const{return objective_models_;}
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
    void position(Frame frame,const HudMapVector& value,float y=0){require(frame);frames_.hud_message_position(frame,
        static_cast<float>(static_cast<long double>(value[0])*scale_),static_cast<float>(static_cast<long double>(value[2])*scale_+y),-.5f);}
    void direction(Frame frame,const HudMapVector& value){require(frame);frames_.car_camera_direction(frame,hud_map_direction(value),1.5707963705062866f);}
    void visible(Frame frame,bool enabled){require(frame);frames_.object_visible(frame,enabled);}
    void updated(Frame frame){require(frame);frames_.update_scene_object(frame);}
    void resize_models(std::vector<Owner>& models,std::size_t count,Frame source){ //004A51D0/004A5400.
        while(models.size()>count)models.pop_back();
        while(models.size()<count){require(source);auto visual=frames_.create_hud_inventory_visual();if(!visual.get())throw std::runtime_error("HUD map marker factory returned null");
            frames_.hud_inventory_duplicate(visual.get(),source);frames_.link_scene_object(visual.get(),map_);visible(visual.get(),false);updated(visual.get());models.push_back(std::move(visual));}
    }
    template<class Queries>void update_offline(Queries& q){ //004A5B50.
        auto* current=q.current_human();if(!current)throw std::logic_error("HUD map requires actual current Human");
        position(markers_[0],q.position(*current));direction(markers_[0],q.direction(*current));frames_.hud_info_scale(markers_[0],1.5f,1.5f,1.5f);visible(markers_[0],true);updated(markers_[0]);
        for(std::int32_t n=0;n<q.roster_count();++n){auto* human=q.roster_human(n);if(!human)throw std::logic_error("HUD map roster lost actual Human");auto& entry=allied_.at(static_cast<std::size_t>(n));
            if(!entry.visual)throw std::logic_error("HUD map roster exceeds initialized allied markers");const auto frame=entry.visual->get();
            if(human==current){visible(frame,false);continue;}
            frames_.hud_visual_a4(frame,q.health(*human)>0?1.f:0.f);position(frame,q.position(*human));direction(frame,q.direction(*human));visible(frame,true);updated(frame);
        }
        auto enemies=q.known_enemies();if(q.game_mode()==3&&enemies.empty())if(auto* human=q.last_seen_human())enemies.push_back(human);
        resize_models(enemy_models_,enemies.size(),markers_[3]);
        for(auto n=enemies.size();n;){--n;auto* human=enemies[n];if(!human)throw std::logic_error("HUD enemy list contains null Human");const auto frame=enemy_models_[n].get();
            if(q.health(*human)<1||q.unavailable(*human)){visible(frame,false);continue;}
            position(frame,q.position(*human));direction(frame,q.direction(*human));visible(frame,true);updated(frame);
        }
        if(flag24_){position(markers_[2],q.camera_position());frames_.car_camera_direction(markers_[2],hud_map_camera_direction(q.camera_direction()),1.5707963705062866f);visible(markers_[2],true);updated(markers_[2]);}
        else visible(markers_[2],false);
        update_editor_goals(q);
    }
    template<class Queries>void update_neutral(Queries& q){ //004A6DE0.
        auto* human=q.current_human();if(!human)throw std::logic_error("HUD map requires actual current Human");
        position(markers_[0],q.position(*human));direction(markers_[0],q.direction(*human));visible(markers_[0],true);updated(markers_[0]);
    }
    template<class Queries>void update_network(Queries& q){ //004A65B0.
        const float width=static_cast<float>(q.display_width());const auto height=q.display_height();
        auto* current=q.current_human();if(!current)throw std::logic_error("HUD map requires actual current Human");const auto current_id=q.identity(*current);
        if(q.vip_identity()==current_id){visible(markers_[0],false);updated(markers_[0]);}
        else{position(markers_[0],q.position(*current));direction(markers_[0],q.direction(*current));frames_.hud_info_scale(markers_[0],1.5f,1.5f,1.5f);visible(markers_[0],true);updated(markers_[0]);frames_.hud_inventory_slot_opacity(markers_[0],1.f);}
        for(unsigned n=0;n<allied_count_;++n){auto* human=q.reserved_human(n);auto& entry=allied_[n];if(!entry.visual)continue;const auto frame=entry.visual->get();
            if(!human||!q.player_controlled(*human)||q.health(*human)<1||q.suppressed(*human)){visible(frame,false);continue;}
            const auto team=q.team(*human);if(q.team(*current)!=team){visible(frame,false);continue;}
            position(frame,q.position(*human));
            if(kind_==1||!q.has_speech(*human)){frames_.hud_inventory_slot_opacity(frame,1.f);entry.flag=0;}
            else{
                if(!entry.text)throw std::logic_error("HUD map speech label lost actual text");
                const auto first=world_position(frame),second=world_position(frame);hud_map_label_geometry(*entry.text,first,second,width,height,depth_);
                const auto style=q.team(*human)==1?0x4cu:0x4du;entry.text->text.set(std::make_shared<const std::string>(q.speech(*human)),style,0);entry.flag=1;
                const auto timer=q.speech_timer(*human)%500;const auto alpha=timer<251?static_cast<float>(1.f-static_cast<long double>(timer)*.004f):static_cast<float>(static_cast<long double>(timer-250)*.004f);
                frames_.hud_inventory_slot_opacity(human==current?markers_[0]:frame,alpha);
            }
            if(human!=current&&static_cast<std::int32_t>(n)!=q.vip_identity()){direction(frame,q.direction(*human));visible(frame,true);updated(frame);}else visible(frame,false);
        }
        update_vip(q);update_item_markers(q);update_goals(q);update_zones(q);
    }
    HudMapVector world_position(Frame frame){require(frame);const auto matrix=frames_.actor_world_matrix(frame);return {{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    template<class Queries>void update_vip(Queries& q){ //004A5690, A4 alpha and A8 opacity are distinct.
        const auto identity=q.vip_identity();if(identity<0){if(frames_.hud_frame_visible(markers_[1]))visible(markers_[1],false);updated(markers_[1]);return;}
        auto* human=q.reserved_human(static_cast<std::uint32_t>(identity));if(!human)throw std::logic_error("HUD VIP identity lost actual Human");
        frames_.hud_visual_a4(markers_[1],q.health(*human)<1?.3f:1.f);position(markers_[1],q.position(*human));const auto forward=hud_map_direction(q.direction(*human));
        auto* current=q.current_human();if(!current)throw std::logic_error("HUD VIP requires current Human");const auto current_id=q.identity(*current);const float scale=q.vip_identity()==current_id?1.5f:1.f;
        frames_.hud_info_scale(markers_[1],scale,scale,scale);frames_.car_camera_direction(markers_[1],forward,1.5707963705062866f);visible(markers_[1],true);updated(markers_[1]);
    }
    template<class Queries>void update_item_markers(Queries& q){ //004A5970 snapshots actual kind20 registry.
        auto items=q.marked_items();resize_models(objective_models_,items.size(),markers_[4]);
        for(auto n=items.size();n;){--n;const auto frame=objective_models_[n].get();position(frame,q.marked_item_position(items[n]),.01f);visible(frame,true);updated(frame);}
    }
    void zone_state(Zone& zone,std::uint32_t state){ //004A7A20.
        if(zone.state==state)return;zone.state=state;
        if(zone.allied){visible(zone.allied,state==1);updated(zone.allied);}if(zone.axis){visible(zone.axis,state==0);updated(zone.axis);}
    }
    template<class Queries>void update_zones(Queries& q){ //004A7180/004A7A70.
        for(auto& zone:zones_)zone_state(zone,zone.actor?static_cast<std::uint32_t>(q.zone_state(*zone.actor)):2u);
        auto* selected=q.spawn_record();if(selected&&q.spawn_identity(*selected)>0){
            for(auto n=zones_.size();n;){auto& zone=zones_[--n];if(!zone.actor)continue;
                if(q.actor_identity(*zone.actor)!=q.spawn_identity(*selected))continue;
                if(q.spawn_team(*selected)!=2&&static_cast<std::uint32_t>(q.spawn_team(*selected))==zone.state){
                    auto p=world_position(zone.root);p[2]=static_cast<float>(static_cast<long double>(p[2])-.5f);require(select_zone_);frames_.hud_scroll_position(select_zone_,p);frames_.hud_frame_tag(select_zone_,zone.tag);visible(select_zone_,true);updated(select_zone_);return;
                }
                q.spawn_identity(*selected)=-1;break;
            }
        }
        visible(select_zone_,false);updated(select_zone_);
    }
    template<class Queries,class Pointer>void select_zone(Queries& q,Pointer& pointer){ //004A7050.
        auto* selected=q.spawn_record();if(!selected||q.spawn_team(*selected)==2)return;const auto tag=pointer.pointer_hit_tag();if(tag<=999||tag>=1015)return;
        for(auto n=zones_.size();n;){auto& zone=zones_[--n];if(zone.tag!=tag)continue;
            if(zone.root&&zone.actor&&zone.state==static_cast<std::uint32_t>(q.spawn_team(*selected))){q.spawn_identity(*selected)=q.actor_identity(*zone.actor);
                auto p=world_position(zone.root);p[2]=static_cast<float>(static_cast<long double>(p[2])-.5f);require(select_zone_);frames_.hud_scroll_position(select_zone_,p);visible(select_zone_,true);frames_.hud_frame_tag(select_zone_,tag);updated(select_zone_);}return;
        }
    }
    template<class Queries>void update_editor_goals(Queries& q){ //004A7940/004A7F50.
        for(auto& goal:editor_goals_){if(!goal.objective)throw std::logic_error("HUD editor goal has no actual objective");const auto state=q.objective_state(*goal.objective);if(static_cast<std::uint32_t>(goal.state)==state)continue;goal.state=static_cast<std::int32_t>(state);
            switch(state){case 0:visible(goal.root,true);visible(goal.allied,false);visible(goal.axis,true);break;
            case 1:visible(goal.root,true);if(goal.allied)visible(goal.allied,true);if(goal.axis)visible(goal.axis,false);break;
            case 2:case 3:case 4:visible(goal.root,false);visible(goal.allied,false);visible(goal.axis,false);break;default:break;}
            updated(goal.allied);updated(goal.axis);updated(goal.root);
        }
    }
    template<class Queries>void update_goals(Queries& q){ //004A78E0/004A7C60.
        for(auto& goal:goals_){if(q.local_team()==2)continue;if(!goal.objective)throw std::logic_error("HUD goal has no actual objective");
            if(goal.state0c==q.objective_state(*goal.objective)&&goal.state10==static_cast<std::uint32_t>(q.local_team()))continue;
            goal.state0c=q.objective_state(*goal.objective);goal.state10=static_cast<std::uint32_t>(q.local_team());
            const auto title=[&]{if(goal.text&&(goal.state10==0||goal.state10==1))goal.text->text.set(factory_.hud_localized(q.objective_caption(*goal.objective,goal.state10)),0x2d,0);};
            switch(goal.state0c){case 0:visible(goal.root,true);visible(goal.allied,false);visible(goal.axis,true);title();break;
            case 1:visible(goal.root,true);visible(goal.allied,true);visible(goal.axis,false);title();if(goal.text)goal.text->text.set(factory_.hud_localized(goal.state10==static_cast<std::uint32_t>(q.objective_team(*goal.objective))?0xb63:0xb64),0x2d,1);break;
            case 2:visible(goal.root,true);title();if(goal.text)goal.text->text.set(factory_.hud_localized(goal.state10==static_cast<std::uint32_t>(q.objective_team(*goal.objective))?0xb64:0xb63),0x2d,1);break;
            case 4:visible(goal.root,false);visible(goal.allied,false);visible(goal.axis,false);if(goal.text)goal.text->text.clear();break;default:break;}
            updated(goal.allied);updated(goal.axis);
        }
    }
    template<class World>void refresh_goal(std::size_t n,World& world){ //004A7C60: local side reads stay separate across callbacks.
        if(world.map_local_side()==2)return;
        if(!goals_.at(n).objective)throw std::logic_error("HUD map goal lost its actual descriptor");
        if(goals_[n].state0c==goals_[n].objective->word10&&goals_[n].state10==world.map_local_side())return;
        goals_[n].state0c=goals_[n].objective->word10;goals_[n].state10=world.map_local_side();
        const auto show=[&](unsigned part,bool visible){auto& goal=goals_.at(n);const auto frame=part==0?goal.root:part==1?goal.allied:goal.axis;require(frame);frames_.object_visible(frame,visible);};
        const auto caption=[&](std::uint32_t id,bool append){const auto label=factory_.hud_localized(id);auto* text=goals_.at(n).text.get();if(!text)throw std::logic_error("HUD goal text owner disappeared");text->text.set(label,0x2d,append?1:0);};
        switch(goals_[n].state0c){
        case 0:case 1:case 2:{
            const auto status=goals_[n].state0c;show(0,true);
            if(status!=2){show(1,status==1);show(2,status==0);}
            if(goals_[n].text){
                if(goals_[n].state10==1)caption(human_death_objective_fields(goals_[n].objective).field1c,false);
                else if(goals_[n].state10==0)caption(human_death_objective_fields(goals_[n].objective).field20,false);
                if(status!=0){
                    const bool same=goals_[n].state10==human_death_objective_fields(goals_[n].objective).side18;
                    caption((status==1?same:!same)?0xb63u:0xb64u,true);
                }
            }
            break;
        }
        case 4:show(0,false);show(1,false);show(2,false);if(goals_[n].text)goals_[n].text->text.clear();break;
        default:break;
        }
        require(goals_.at(n).allied);frames_.update_scene_object(goals_[n].allied);
        require(goals_.at(n).axis);frames_.update_scene_object(goals_[n].axis);
    }
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
