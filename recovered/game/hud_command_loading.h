#ifndef HD2_GAME_HUD_COMMAND_LOADING_H
#define HD2_GAME_HUD_COMMAND_LOADING_H
#include "game/hud_text_loading.h"
#include "game/hud_command_routes.h"
#include "game/hud_command_frame.h"
#include "game/hud_command_input.h"
#include "game/hud_command_highlights.h"
#include "frontend/cleanup.h"
#include <array>
#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
namespace hd2::game {
//004B4980/004B4DE0/004B5790 and004B4D40/004B5780/004B59C0. Actual HUD
//eight inline texts, world/navigation borrowers, separate nested roll receiver,
//SAME camera/input owners. Command actions and Human route production remain
//separate from these presentation, pointer and policy receivers.
template<class Frames,class Factory,class Hud,class Dialogs,class Camera>
class HudCommandLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using Owner=decltype(std::declval<Frames&>().create_hud_info_model());
    struct Button {Frame normal{},alternate{};std::uint32_t tag=0;};
    using Presentation=HudCommandPresentation;
    HudCommandLoading(Frames& frames,Factory& factory,Hud& hud,Dialogs& roll,Camera& camera)
        :frames_(frames),factory_(factory),hud_(hud),roll_(roll),camera_(camera){}
    void bind(Frame scene,Frame world,void* navigation,std::array<void*,5> contexts) {
        if(bound_)throw std::logic_error("HUD command rebound before borrower retirement");
        require(scene);require(world);bound_=true;scene_=scene;world_=world;navigation_=navigation;contexts_=contexts;
        border_=find("Border");frames_.object_visible(border_,false);frames_.update_scene_object(border_);
        fields10_.fill(0);presentation_=Presentation{};bind_buttons();bind_texts();
        for(unsigned n=0;n!=10;++n){std::array<char,32> name{};std::snprintf(name.data(),name.size(),"info%02u",n);frames_.hud_frame_tag(find(name.data()),n+20);}
        auto model=frames_.create_hud_info_model();if(!model)throw std::runtime_error("HUD command model factory returned null");
        //Original tolerates missing l_aicomm: release the attempted owner and
        //continue with NULL748. This model is never added to the HUD scene.
        if(frames_.load_hud_info_model(model.get(),"l_aicomm",0)==0)model_.emplace(std::move(model));
        else model.reset();
        roll_.bind_roll(static_cast<Hud*>(nullptr),scene_,1);frame_state_=HudCommandFrameState{};
        routes_.clear_counts();
        input_state_=HudCommandInputState{};
        constexpr std::array<const char*,10> names{{"lie","kneel","stay","slow","walk","run","sprint","passive","defensive","aggresive"}};
        for(unsigned n=0;n!=names.size();++n) {
            const auto visual=find(names[n]);const auto lod=frames_.first_person_material_lod(visual);
            textures_[n]=frames_.first_person_material_texture(frames_.hud_first_material_from_lod(lod));
        }
    }
    void close() { //Input mode BEFORE text/model/route/roll cleanup, even after a partial bind.
        frontend::CleanupErrors errors;errors.attempt([&]{factory_.hud_binding_mode(2);});
        for(const auto n:{0u,1u,2u,3u,4u,5u,7u})inline_text(n).text.clear(); //Camera-motion caption6 is deliberately retained.
        model_.reset();while(!route_models_.empty())route_models_.pop_back();roll_.close_roll();errors.rethrow();
    }
    void discard_borrowers()noexcept {
        border_=scene_=world_=Frame{};navigation_=nullptr;contexts_.fill(nullptr);textures_.fill(Frame{});
        buttons_.fill(Button{});speed_icons_.fill(Frame{});roll_.discard_borrowers();bound_=false;
        presentation_.previous14=presentation_.current18=presentation_.target1c=nullptr;
    }
    void retire_actor_reference(const void* actor) { //0049EF70 target1C; current/previous are also retracted as source lifetime guards.
        if(presentation_.previous14==actor)input_state_.order_drag5b0=false;
        for(auto** slot:{&presentation_.previous14,&presentation_.current18,&presentation_.target1c})if(*slot==actor)*slot=nullptr;
        roll_.retire_roll_actor(actor,false); //Same nested command590 target, before its actor can die.
    }
    const auto& buttons()const{return buttons_;}const auto& speed_icons()const{return speed_icons_;}
    const auto& textures()const{return textures_;}auto route_counts()const{return routes_.counts();}
    auto& route_buffers(){return routes_;}
    auto& frame_state(){return frame_state_;}const auto& frame_state()const{return frame_state_;}
    auto& input_state(){return input_state_;}auto& policies(){return policies_;}
    bool input_bound()const{return world_!=Frame{};}Frame border()const{return border_;}
    bool button_visible(unsigned tag){require_bound();const auto normal=buttons_.at(tag-1).normal;require(normal);return frames_.hud_frame_visible(normal);}
    void input_button(unsigned tag,bool enabled,bool click){require_bound();button_state(tag,enabled,click);}
    template<class Queries,class Pointer,class Input>void process_controls(Queries& queries,Pointer& pointer,Input& input){HudCommandInput worker(*this,pointer,input,queries);worker.run();}
    bool nested_roll_visible()const{return roll_.roll().flag30;}
    template<class Queries>void show(bool enabled,Queries& queries){ //004B5E40; SAME existing camera, roster, selection and scene actors.
        if(!border_)return;
        frames_.object_visible(border_,enabled);frames_.update_scene_object(border_);
        queries.enable_command_camera(enabled);input_state_.order_drag5b0=false;
        if(enabled){
            frame_state_.selection_changed5a7b1=true;frame_state_.cooldown58=0;
            input_state_.spare5bd=frame_state_.point_gate5a7b8=frame_state_.point_gate5a7b9=presentation_.project_pointer=input_state_.captured5bc=false;
            auto* current=queries.current_player();presentation_.current18=current?queries.identity(*current):nullptr;
            auto* previous=queries.previous_human();presentation_.previous14=previous?queries.identity(*previous):nullptr;
            button_state(18,frame_state_.mode5c,true);
            queries.prepare_command_sight(); //0045F080 reaches real Sight0046F280; it does NOT clear selection.
            queries.request_realtime(!frame_state_.mode5c);update_policies(queries);queries.selection_notifications(true);
            const auto selected=queries.selected_humans();
            for(auto n=queries.scene_actor_count();n;){auto* actor=queries.scene_actor(--n);if(!actor||queries.kind(*actor)!=3)continue;
                if(!queries.in_roster(*actor)&&!queries.script_suspended(*actor)&&!queries.unavailable(*actor)&&queries.health(*actor)>0){
                    const auto kind=queries.human_kind(*actor);if(kind==3||kind==4)continue;
                    bool retained=false;for(auto* value:selected)if(value==actor){retained=true;break;}if(!retained)queries.hide_human(*actor,100);
                }
            }
            if(!frame_state_.mode5c)queries.retire_command_special_model();factory_.hud_binding_mode(3);
        }else{
            for(std::int32_t n=0;n<queries.roster_count();++n){auto* human=queries.roster_human(n);if(!human)throw std::logic_error("Command hide requires actual roster Human");
                for(std::int32_t order=0;order<queries.order_count(*human);++order){const auto loan=queries.order(*human,order);require(loan.model);frames_.object_visible(loan.model,false);}}
            queries.publish_signal_selection(queries.null_actor(),5);queries.show_nested_roll(false,nullptr);
            queries.request_realtime(false);queries.camera_mode(0,nullptr);queries.selection_notifications(false);
            for(auto n=queries.scene_actor_count();n;){auto* actor=queries.scene_actor(--n);if(actor&&queries.kind(*actor)==3&&!queries.script_suspended(*actor))queries.show_human(*actor,200);}
            factory_.hud_binding_mode(2);
        }
    }
    template<class Queries,class Pointer>void advance_nested_roll(std::uint32_t elapsed,Queries& queries,Pointer& pointer){
        roll_.hover_roll(pointer.pointer_position(),queries);roll_.advance_roll(elapsed,queries);
    }
    void toggle_feedback_button(std::uint32_t tag){require_bound();button_state(tag,!frames_.hud_frame_visible(buttons_.at(tag-1).normal),false);}
    template<class Human,class Queries>void update_routes(Human& human,std::size_t index,Queries& queries){
        require_bound();update_hud_command_routes(routes_,frames_,frame_state_,presentation_,human,index,queries);
    }
    template<class Queries,class Pointer>void update(std::uint32_t elapsed,Queries& queries,Pointer& pointer){
        if(!world_)return;advance_hud_command_frame(*this,frame_state_,presentation_,elapsed,queries,pointer);
    }
    template<class Controls>void activate_misc_button(std::uint32_t tag,Controls& controls){ //004B86D0, tags16..18.
        require_bound();
        if(tag==16){
            button_state(tag,!frames_.hud_frame_visible(buttons_[tag-1].normal),true);
            frame_state_.feedback_tag=tag;frame_state_.feedback740=200;controls.request_options();
        }else if(tag==17){
            button_state(tag,false,true);cycle_speed();frame_state_.feedback_tag=tag;frame_state_.feedback740=200;
        }else if(tag==18){
            frame_state_.mode5c=!frame_state_.mode5c;button_state(tag,frame_state_.mode5c,true);
            controls.request_realtime(!frame_state_.mode5c);
        }
    }
    std::uint32_t speed_kind()const{return speed_kind_;}Frame model()const{return model_?model_->get():Frame{};}
    bool visible(){return border_&&frames_.hud_frame_visible(border_);} //004B6180 reads rootE0 bit1; null returns0.
    template<class Pointer>void update_hover_caption(Pointer& pointer){ //004B6280, elapsed argument is unused natively.
        require_bound();const auto tag=pointer.pointer_hit_tag();unsigned index=8;std::uint32_t id=0;
        switch(tag){
        case 2:index=5;id=0x1cb7;break;case 3:index=5;id=0x1cb8;break;
        case 4:index=1;id=0x1c8f;break;case 5:index=1;id=0x1c90;break;
        case 6:case 7:case 8:index=2;id=0x1c99+tag-6;break;
        case 9:case 10:case 11:case 12:index=3;id=0x1ca3+tag-9;break;
        case 13:case 14:case 15:index=4;id=0x1caf-(tag-13);break;
        case 16:index=7;id=0x1cc3;break;case 18:index=7;id=0x1cc1;break;default:break;
        }
        set_caption(index,id);
    }
    void set_caption(unsigned index,std::uint32_t id){ //004B61A0; reset options BEFORE camera caption, then replace the hovered row.
        require_bound();if(index>8)throw std::out_of_range("HUD command caption index exceeds native table");
        constexpr std::array<std::uint32_t,8> ids{{0x1c84,0x1c8e,0x1c98,0x1ca2,0x1cac,0x1cb6,0x1cc2,0x1cc0}};
        for(const auto n:{0u,1u,2u,3u,4u,5u,7u,6u})inline_text(n).text.set(factory_.hud_localized(ids[n]),0x47,0);
        if(!roll_.roll().flag30&&index!=8)inline_text(index).text.set(factory_.hud_localized(id),0x47,0);
    }
    void update_policy_buttons(std::uint32_t movement,std::uint32_t mind,std::uint32_t pose){ //004B5C10, priority and call order are observable.
        require_bound();
        update_button_group<4>(movement,{{0x10000,0x4000,0x2000,0x1000}},{{9,10,11,12}});
        update_button_group<3>(mind,{{0x400,0x200,0x100}},{{13,14,15}});
        update_button_group<3>(pose,{{4,2,1}},{{6,7,8}});
    }
    void update_order_buttons(std::uint32_t kind){ //004B5B90: regroup, wait01, wait02, no click on these calls.
        require_bound();const bool enabled=kind!=6&&kind!=7&&kind!=4;
        for(const unsigned tag:{1u,4u,5u})button_state(tag,enabled,false);
    }
    void lock_buttons(bool locked,bool paused){ //004B5AD0, reverse native18-button order when locking.
        require_bound();if(locked){for(unsigned tag=18;tag;--tag)button_state(tag,false,false);return;}
        update_policy_buttons(policies_[0],policies_[1],policies_[2]);for(const unsigned tag:{1u,2u,3u,4u,5u})button_state(tag,true,false);
        button_state(16,false,false);button_state(17,true,false);button_state(18,!paused,false);
    }
    template<class Actor,class Queries,class Pointer>void show_nested_roll(bool enabled,Actor* target,Queries& queries,Pointer& pointer,bool paused){ //004BB0E0.
        require_bound();if(!enabled){roll_.restore_roll_position();roll_.hide_roll();lock_buttons(false,paused);return;}
        auto position=pointer.pointer_position();position[2]=static_cast<float>(static_cast<long double>(position[2])+1.f);
        position[0]=static_cast<float>(static_cast<long double>(position[0])+(position[0]>=.5f?-.10000000149011612f:.10000000149011612f));
        if(position[1]>-.20000000298023224f)position[1]=-.20000000298023224f;else if(position[1]<-.6000000238418579f)position[1]=-.6000000238418579f;
        roll_.position_roll(position);roll_.publish_roll_target(target,1.f,queries);roll_.show_roll(true,queries);pointer.select_pointer(0);
        if(roll_.roll().flag30)lock_buttons(true,paused);
    }
    template<class Queries>void update_policies(Queries& queries){ //004B5A20; the selected order's flags are the actual Human-owned word44.
        require_bound();auto* flags=queries.selected_order_flags();
        if(flags){
            if(!(*flags&0x1f000u))*flags|=0x4000u;policies_[0]=*flags&0x1f000u;
            if(!(*flags&0x700u))*flags|=0x200u;policies_[1]=*flags&0x700u;
            if(!(*flags&7u))*flags|=4u;policies_[2]=*flags&7u;
        }else if(presentation_.previous14){
            policies_[0]=queries.human_movement(presentation_.previous14);
            policies_[1]=queries.human_mind(presentation_.previous14);
            policies_[2]=queries.human_pose(presentation_.previous14);
        }else policies_.fill(0xffffffffu);
        update_policy_buttons(policies_[0],policies_[1],policies_[2]);factory_.hud_click();
    }
    template<class Queries,class Pointer>void update_point_target(Queries& queries,Pointer& pointer){ //004BA070; native elapsed is unused.
        require_bound();if(roll_.roll().flag30)return;
        fields10_[0]=0;if(!pointer.pointer_in_command_area(pointer.pointer_position()))return; //Out-of-area keeps the last target/position.
        const auto screen=pointer.pointer_screen_position();const auto ray=frames_.hud_world_screen_ray(world_,screen[0],screen[1]);
        auto travel=ray.second;for(unsigned n=0;n!=3;++n)travel[n]=static_cast<float>(static_cast<long double>(ray.second[n])*100.f);
        const auto hit=queries.command_ray(ray.first,travel);
        if(!hit){
            for(unsigned n=0;n!=3;++n){
                const auto step=static_cast<long double>(ray.second[n])*100.f;
                presentation_.position20[n]=static_cast<float>(static_cast<long double>(ray.first[n])+(n==2?static_cast<float>(step):step));
            }
            presentation_.position20[3]=ray.second[3]; //Original copies local2C, the engine direction's fourth word.
            presentation_.target1c=nullptr;return;
        }
        //Original ray writes XYZ only; preserve native position2C on the hit path.
        for(unsigned n=0;n!=3;++n){const auto step=static_cast<long double>(ray.second[n])*.2f;
            presentation_.position20[n]=static_cast<float>(static_cast<long double>(hit->position[n])-(n==2?static_cast<float>(step):step));}
        if(!(hit->flags&0x40u)){presentation_.target1c=nullptr;return;}
        auto* actor=hit->actor;presentation_.target1c=actor?queries.identity(*actor):nullptr;if(!actor)return;
        switch(queries.kind(*actor)){
        case 3:
            if(queries.human_hidden(*actor)){presentation_.target1c=nullptr;return;}
            if(auto* attachment=queries.attachment(*actor)){presentation_.target1c=queries.identity(*attachment);return;}
            if(queries.human_kind(*actor)==3&&queries.health(*actor)>0&&!queries.in_roster(*actor))presentation_.target1c=nullptr;
            return;
        case 21:if(!queries.static_weapon_ready(*actor))presentation_.target1c=nullptr;return;
        case 25:if(queries.animation_excludes_target(*actor))presentation_.target1c=nullptr;return;
        default:return;
        }
    }
    void draw_prepass(){routes_.draw(factory_);} //004B7780 uses SAME native route buffers/counts.
    template<class Queries,class Pointer>void draw_before_scene(Queries& queries,Pointer& pointer){ //004B6720.
        if(!bound_)throw std::logic_error("Command draw before SAME HUD/world binding");
        auto selected=queries.selected_humans(); //Actual0045EB30 temporary list, no persistent roster copy.
        for(auto n=selected.size();n;){auto* human=selected[--n];if(!human)throw std::logic_error("Command selection contains null Human");
            const auto colour=queries.health(*human)<1||queries.unavailable(*human)?std::array<float,3>{{0,0,0}}:std::array<float,3>{{1,0,0}};
            auto* actor=human;bool admitted=true;
            if(auto* attachment=queries.attachment(*human)){
                auto count=queries.occupant_count(*attachment);
                for(std::int32_t i=0;i<count;){
                    if(auto* occupant=queries.occupant(*attachment,static_cast<std::size_t>(i))){if(occupant!=human)admitted=false;else actor=attachment;break;}
                    ++i;count=queries.occupant_count(*attachment); //Fresh after every empty seat query.
                }
            }
            if(admitted)draw_hud_command_highlight(frames_,factory_,queries.base(*actor),HudCommandHighlight::selected,colour);
        }
        if(presentation_.current18){auto* actor=queries.resolve(presentation_.current18);if(auto* attachment=queries.attachment(*actor))actor=attachment;
            draw_hud_command_highlight(frames_,factory_,queries.base(*actor),HudCommandHighlight::current,{{0,1,0}});
        }
        if(presentation_.previous14){auto* actor=queries.resolve(presentation_.previous14);if(auto* attachment=queries.attachment(*actor))actor=attachment;
            draw_hud_command_highlight(frames_,factory_,queries.base(*actor),HudCommandHighlight::previous,{{0,0,1}});
        }
        if(presentation_.target1c){auto* actor=queries.resolve(presentation_.target1c);
            draw_hud_command_highlight(frames_,factory_,queries.base(*actor),HudCommandHighlight::target,{{1,1,1}});
        }
        if(presentation_.project_pointer)pointer.project_pointer(presentation_.position20);
    }
    auto& presentation(){return presentation_;}const auto& presentation()const{return presentation_;}
    void draw_after_scene(){ //004B7830: nested roll then SAME8inline texts ascending, no caption visibility gate.
        if(roll_.roll().flag30)roll_.draw_roll();
        for(unsigned n=0;n!=8;++n)factory_.hud_draw_text(inline_text(n));
    }
private:
    void require_bound()const{if(!bound_)throw std::logic_error("HUD command operation before actual HUD/world binding");}
    void button_state(unsigned tag,bool enabled,bool click){ //004A8BD0.
        if(click)factory_.hud_click();auto& button=buttons_.at(tag-1);
        require(button.normal);frames_.object_visible(button.normal,enabled);require(button.alternate);frames_.object_visible(button.alternate,!enabled);
    }
    template<std::size_t N>void update_button_group(std::uint32_t flags,const std::array<std::uint32_t,N>& masks,const std::array<unsigned,N>& tags){
        std::size_t selected=N;
        if(flags!=0xffffffffu){for(std::size_t n=0;n<N;++n)if(flags&masks[n]){selected=n;break;}if(selected==N)return;}
        for(std::size_t n=0;n<N;++n)button_state(tags[n],n==selected,n==selected);
    }
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD command artwork is missing");}
    Frame find(const char* name){const auto frame=frames_.hud_find(scene_,name,0xffff);require(frame);return frame;}
    Text& inline_text(unsigned n) {
        for(const auto& owner:hud_.text_owners())if(owner.original_offset==0x10bc8+n*0x9c)return *owner.text;
        throw std::logic_error("HUD command lost its actual inline text owner");
    }
    void bind_buttons() {
        struct Definition {const char* normal;const char* alternate;unsigned tag;bool enabled;};
        constexpr std::array<Definition,18> definitions{{
            {"regroup","~regroup",1,true},{"delete","~delete",2,true},{"deleteall","~deleteall",3,true},
            {"stay","~stay",6,false},{"kneel","~kneel",7,false},{"lie","~lie",8,false},{"sprint","~sprint",9,false},
            {"run","~run",10,false},{"walk","~walk",11,false},{"slow","~slow",12,false},{"passive","~passive",13,false},
            {"aggresive","~aggresive",15,false},{"defensive","~defensive",14,false},
            {"wait01","~wait01",4,true},{"wait02","~wait02",5,true},{"options","~options",16,false},
            {"TMCamera","~TMCamera",17,true},{"realtime","~realtime",18,false}
        }};
        for(const auto& d:definitions) {
            auto& b=buttons_[d.tag-1];b.normal=find(d.normal);b.alternate=find(d.alternate);b.tag=d.tag;
            frames_.hud_frame_tag(b.normal,d.tag);frames_.object_visible(b.alternate,false);frames_.hud_frame_tag(b.alternate,d.tag);
            const auto matrix=frames_.actor_world_matrix(b.normal);
            frames_.hud_scroll_position(b.alternate,{{matrix[12],matrix[13],matrix[14],matrix[15]}});
            frames_.object_visible(b.normal,d.enabled);frames_.object_visible(b.alternate,!d.enabled); //004A8BD0(?,false).
        }
        frame_state_.feedback740=frame_state_.feedback_tag=0;
        constexpr std::array<const char*,5> names{{"TMcamera1x","TMcamera2x","TMcamera4x","TMcamera8x","TMcamera16x"}};
        for(unsigned n=0;n!=5;++n)speed_icons_[n]=find(names[n]);
        speed_kind_=1;cycle_speed(); //004BF130 increments1→2, writes detached40=1.
    }
    void cycle_speed() {
        constexpr std::array<float,5> speeds{{.25f,.5f,1.f,2.f,4.f}};
        speed_kind_=(speed_kind_+1)%5;
        for(unsigned n=0;n!=5;++n)frames_.object_visible(speed_icons_[n],n==speed_kind_);
        camera_.set_detached_speed(speeds[speed_kind_]);
    }
    void bind_texts() {
        constexpr std::array<const char*,8> names{{"text_regroup","text_signals","text_pose","text_movement","text_mind","text_delete","text_cameramotion","text_options"}};
        constexpr std::array<std::uint32_t,8> captions{{0x1c84,0x1c8e,0x1c98,0x1ca2,0x1cac,0x1cb6,0x1cc2,0x1cc0}};
        for(unsigned n=0;n!=8;++n) {
            const auto frame=find(names[n]);auto& text=inline_text(n);text.text.inset=0;
            bind_hud_text_geometry(frames_,text,scene_,frame);
            text.text.set(factory_.hud_localized(captions[n]),0x47,0);frames_.object_visible(frame,false);
        }
    }
    Frames& frames_;Factory& factory_;Hud& hud_;Dialogs& roll_;Camera& camera_;
    Frame scene_{},world_{},border_{};void* navigation_=nullptr;std::array<void*,5> contexts_{};
    std::array<Button,18> buttons_{};std::array<Frame,5> speed_icons_{};std::array<Frame,10> textures_{};
    std::optional<Owner> model_;std::vector<Owner> route_models_;
    std::array<std::uint32_t,3> fields10_{},policies_{};Presentation presentation_;HudCommandFrameState frame_state_;
    HudCommandRoutes routes_;
    HudCommandInputState input_state_;std::uint32_t speed_kind_=0;bool bound_=false;
};
} //namespace hd2::game
#endif
