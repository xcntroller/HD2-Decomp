#ifndef HD2_GAME_HUD_COMMAND_LOADING_H
#define HD2_GAME_HUD_COMMAND_LOADING_H
#include "game/hud_text_loading.h"
#include "game/hud_command_routes.h"
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
//eight inline texts, world/collision borrowers, separate nested roll receiver,
//SAME camera/input owners. No command interactions or generated route producer.
template<class Frames,class Factory,class Hud,class Dialogs,class Camera>
class HudCommandLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using Owner=decltype(std::declval<Frames&>().create_hud_info_model());
    struct Button {Frame normal{},alternate{};std::uint32_t tag=0;};
    HudCommandLoading(Frames& frames,Factory& factory,Hud& hud,Dialogs& roll,Camera& camera)
        :frames_(frames),factory_(factory),hud_(hud),roll_(roll),camera_(camera){}
    void bind(Frame scene,Frame world,void* collision,std::array<void*,5> contexts) {
        if(bound_)throw std::logic_error("HUD command rebound before borrower retirement");
        require(scene);require(world);bound_=true;scene_=scene;world_=world;collision_=collision;contexts_=contexts;
        border_=find("Border");frames_.object_visible(border_,false);frames_.update_scene_object(border_);
        fields10_.fill(0);actor1c_=nullptr;bind_buttons();bind_texts();
        for(unsigned n=0;n!=10;++n){std::array<char,32> name{};std::snprintf(name.data(),name.size(),"info%02u",n);frames_.hud_frame_tag(find(name.data()),n+20);}
        auto model=frames_.create_hud_info_model();if(!model)throw std::runtime_error("HUD command model factory returned null");
        //Original tolerates missing l_aicomm: release the attempted owner and
        //continue with NULL748. This model is never added to the HUD scene.
        if(frames_.load_hud_info_model(model.get(),"l_aicomm",0)==0)model_.emplace(std::move(model));
        else model.reset();
        roll_.bind_roll(static_cast<Hud*>(nullptr),scene_,1);flag5c_=0;
        routes_.clear_counts();
        flags5a7b_.fill(0);
        constexpr std::array<const char*,10> names{{"lie","kneel","stay","slow","walk","run","sprint","passive","defensive","aggresive"}};
        for(unsigned n=0;n!=names.size();++n) {
            const auto visual=find(names[n]);const auto lod=frames_.first_person_material_lod(visual);
            textures_[n]=frames_.first_person_material_texture(frames_.hud_first_material_from_lod(lod));
        }
        field54_=0;flag5a7b0_=0;
    }
    void close() { //Input mode BEFORE text/model/route/roll cleanup, even after a partial bind.
        frontend::CleanupErrors errors;errors.attempt([&]{factory_.hud_binding_mode(2);});
        for(const auto n:{0u,1u,2u,3u,4u,5u,7u})inline_text(n).text.clear(); //Camera-motion caption6 is deliberately retained.
        model_.reset();while(!route_models_.empty())route_models_.pop_back();roll_.close_roll();errors.rethrow();
    }
    void discard_borrowers()noexcept {
        border_=scene_=world_=Frame{};collision_=nullptr;contexts_.fill(nullptr);textures_.fill(Frame{});
        buttons_.fill(Button{});speed_icons_.fill(Frame{});roll_.discard_borrowers();bound_=false;
        actor1c_=nullptr;
    }
    void retire_actor_reference(const void* actor)noexcept {if(actor1c_==actor)actor1c_=nullptr;} //0049EF70, HUD10B3C = command+1C.
    const auto& buttons()const{return buttons_;}const auto& speed_icons()const{return speed_icons_;}
    const auto& textures()const{return textures_;}auto route_counts()const{return routes_.counts();}
    auto& route_buffers(){return routes_;}
    std::uint32_t speed_kind()const{return speed_kind_;}Frame model()const{return model_?model_->get():Frame{};}
    bool visible(){return border_&&frames_.hud_frame_visible(border_);} //004B6180 reads rootE0 bit1; null returns0.
    void draw_prepass(){routes_.draw(factory_);} //004B7780 uses SAME native route buffers/counts.
    void draw_after_scene(){ //004B7830: nested roll then SAME8inline texts ascending, no caption visibility gate.
        if(roll_.roll().flag30)roll_.draw_roll();
        for(unsigned n=0;n!=8;++n)factory_.hud_draw_text(inline_text(n));
    }
private:
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
        fields740_.fill(0);
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
    Frame scene_{},world_{},border_{};void* collision_=nullptr;std::array<void*,5> contexts_{};
    std::array<Button,18> buttons_{};std::array<Frame,5> speed_icons_{};std::array<Frame,10> textures_{};
    std::optional<Owner> model_;std::vector<Owner> route_models_;
    std::array<std::uint32_t,3> fields10_{};const void* actor1c_=nullptr;std::array<std::uint32_t,2> fields740_{};
    HudCommandRoutes routes_;
    std::array<std::uint8_t,6> flags5a7b_{};std::uint8_t flag5c_=0,flag5a7b0_=0;std::uint32_t speed_kind_=0,field54_=0;bool bound_=false;
};
} //namespace hd2::game
#endif
