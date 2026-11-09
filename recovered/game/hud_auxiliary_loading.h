#ifndef HD2_GAME_HUD_AUXILIARY_LOADING_H
#define HD2_GAME_HUD_AUXILIARY_LOADING_H
#include "game/hud_artwork_loading.h"
#include "game/hud_text_loading.h"
#include "frontend/cleanup.h"
#include <memory>
#include <stdexcept>
namespace hd2::game {
//004AB860 world-pointer,004AB230 info icons,004AAF20 progress loading and
//matching004AB950/004AB2D0/004AB0F0. These are real HUD scene borrowers;
//containing HUD must close progress/discard borrowers before scene release.
template<class Frames,class Factory>
class HudAuxiliaryLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    struct Pointer {
        Frame field00{},hud_scene{},world_scene{};std::int32_t field0c=-1;
        std::array<float,4> position10{},position20{};std::array<Frame,16> frames30{};
        float field70=0,field74=0;std::uint32_t field78=0;
    };
    struct Info {
        std::uint32_t field00=0;std::array<Frame,10> frames{};
        const std::array<Frame,20>* states=nullptr;const std::array<Frame,39>* actions=nullptr;Frame selected{};
    };
    struct Progress {Frame background{},bar{};std::uint32_t field08=0,field0c=0,field10=0xffffffffu;std::unique_ptr<Text> text;};
    HudAuxiliaryLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    void bind_pointer(Frame hud_scene,Frame world_scene) { //004AB860; literal table repeats k_move at slots1/14/15.
        require(hud_scene);pointer_.hud_scene=hud_scene;pointer_.world_scene=world_scene;
        constexpr std::array<const char*,16> names{{"kurzor","k_move","k_rotate","t_rotleft","t_rotright","t_rotup","t_rotdown","t_left","t_right","t_forward","t_backward","t_up","t_down","k_movepos","k_move","k_move"}};
        for(unsigned i=0;i!=names.size();++i){pointer_.frames30[i]=frames_.hud_find(hud_scene,names[i],1);show(pointer_.frames30[i],false);}
        pointer_.field70=pointer_.field74=1;pointer_.field00=0;pointer_.field0c=-1;pointer_.field78=0;
        for(auto* position:{&pointer_.position20,&pointer_.position10}) {
            (*position)[0]=.5f;(*position)[1]=-.375f;(*position)[2]=-10;
            //Both native fourth stack lanes are unwritten. Keep known lanes;
            //their cold zeroes are guards, not constructor recovery credit.
        }
    }
    void bind_info(Frame scene,const HudArtworkLoading<Frames>& artwork) { //004AB230.
        require(scene);
        for(unsigned i=0;i!=info_.frames.size();++i) {
            std::array<char,32> name{};std::snprintf(name.data(),name.size(),"info%02u",i);
            auto& frame=info_.frames[i];frame=frames_.hud_find(scene,name.data(),0xffff);require(frame);
            frames_.hud_group18(frame);show(frame,false);
        }
        info_.field00=0;info_.states=&artwork.first_states();info_.actions=&artwork.actions();
        info_.selected=frames_.hud_find(scene,"infosel",0xffff);show(info_.selected,false);
    }
    void bind_progress(Frame scene) { //004AAF20.
        if(progress_.text)throw std::logic_error("HUD progress rebound before teardown");
        require(scene);progress_.background=frames_.hud_find(scene,"prgbrbk",1);
        progress_.bar=frames_.hud_find(scene,"prgbr01",1);progress_.field08=progress_.field0c=0;
        require(progress_.background);
        frames_.hud_position(progress_.background,{{.5f,-.579999983310699462890625f,-10}});
        frames_.update_scene_object(progress_.background);
        const auto name=frames_.actor_find(progress_.background,"progress_name",0xffff);show(name,false);
        progress_.field10=0xffffffffu;progress_.text=factory_.make_hud_text();
        if(!progress_.text)throw std::runtime_error("HUD progress text factory returned null");
        bind_hud_text_geometry(frames_,*progress_.text,scene,progress_.background);show(progress_.background,false);
    }
    void close_pointer()noexcept{} //004AB950 actual RET.
    void hide_selected_pointer() { //004ABAF0(-1), reached by004A5630(false,false) on multiplayer map bind.
        if(pointer_.field0c==-1)return;
        if(pointer_.field00)frames_.object_visible(pointer_.field00,false);
        pointer_.field0c=-1; //Original retains field00; no update on this branch.
    }
    void close_info()noexcept{} //004AB2D0 actual RET; texture tables are borrowed.
    void close_progress() { //004AB0F0 hides before releasing text, with defined failure cleanup.
        frontend::CleanupErrors errors;errors.attempt([&]{show(progress_.background,false);});
        progress_.text.reset();errors.rethrow();
    }
    void draw_progress(){ //004AB220 unconditional text virtual+04.
        if(!progress_.text)throw std::logic_error("HUD progress draw before text initialization");
        factory_.hud_draw_text(*progress_.text);
    }
    void discard_borrowers()noexcept {
        pointer_.field00=pointer_.hud_scene=pointer_.world_scene=Frame{};pointer_.frames30.fill(Frame{});
        info_.frames.fill(Frame{});info_.selected=Frame{};info_.states=nullptr;info_.actions=nullptr;
        progress_.background=progress_.bar=Frame{};
    }
    const auto& pointer()const{return pointer_;}const auto& info()const{return info_;}const auto& progress()const{return progress_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD auxiliary artwork is missing");}
    void show(Frame frame,bool value){require(frame);frames_.object_visible(frame,value);}
    Frames& frames_;Factory& factory_;Pointer pointer_;Info info_;Progress progress_;
};
} //namespace hd2::game
#endif
