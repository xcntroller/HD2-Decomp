#ifndef HD2_GAME_FIRST_PERSON_VIEW_LOADING_H
#define HD2_GAME_FIRST_PERSON_VIEW_LOADING_H
#include "game/first_person_hands_material.h"
#include "game/first_person_animation_table.h"
#include "frontend/cleanup.h"
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>

namespace hd2::game {
//0045D1D0, borrowing the actual control owner's58/08/2C state. Index-1 is
//genuine no-player; other bad indices reject the native out-of-bounds read.
template<class Human,class List>Human* first_person_current_player(Human* override58,const List& list08,std::int32_t index2c) {
    if(override58)return override58;
    if(index2c==-1)return nullptr;
    if(index2c<0||static_cast<std::size_t>(index2c)>=list08.size())
        throw std::runtime_error("Current-player index exceeds actual control list");
    return list08[static_cast<std::size_t>(index2c)];
}

//004922A0/00492500/00492930/00491640 and00491FD0/004920B0/00492130/
//004921F0, composed with the actual model/table/items/material/playback/camera/
//effects receivers. Players supplies real current-player/inline/B0/attachment/
//camera-retirement lookups; Cursor supplies actual0049DC60. No default services.
//The containing session must compose these owners before any activation claim.
template<class Frames,class View,class Models,class Items,class Players,class Camera,class Playback,class Cursor,class Effects>
class FirstPersonViewLoading {
public:
    using Frame=typename Frames::Frame;
    using Human=typename Players::Human;
    FirstPersonViewLoading(Frames& frames,View& view,Models& models,Items& items,Players& players,Camera& camera,
                          Playback& playback,Cursor& cursor,Effects& effects,const float& game_fov)
        :frames_(frames),view_(view),models_(models),items_(items),players_(players),camera_(camera),playback_(playback),
         cursor_(cursor),effects_(effects),game_fov_(game_fov),materials_(frames) {}
    std::uint32_t state()const{return state60_;}
    bool item_controls_available()const{ //00493500, actual state60 and animation94, independent of view58.
        const auto animation=models_.animation();
        return state60_!=3&&state60_!=4&&state60_!=5&&animation!=6&&!(animation>=8&&animation<=12);
    }
    bool enabled()const{return view_.bytes58[0]!=0;}
    bool secondary_enabled()const{return view_.bytes58[1]!=0;} //Native59 secondary view; camera tests58/59 independently.
    float bob()const{return bob64_;}
    float fov()const{return fov30_;}
    float parameter78()const{return parameter34_;}
    std::uint32_t argument()const{return argument98_;}
    void set_state(std::uint32_t mode,std::uint32_t state) {if((mode&2u)&&state60_!=state)state60_=state;}
    Human* player()const{return static_cast<Human*>(models_.player());}

    const auto& position()const{return position00_;}
    const auto& rotation()const{return rotation10_;}
    const auto& offset()const{return offset20_;}
    float scale()const{return scale6c_;}
    std::array<float,4> projectile_direction() { //00492880: camera virtual68 selector1, not raw FPV rotation10.
        std::array<float,4> result;
        if(!camera_.query_direction(1,result))throw std::runtime_error("Projectile direction requires actual mission camera1; native output is unwritten on failure");
        return result;
    }
    std::array<float,4> projectile_position(std::uint32_t mode) { //00492790: camera virtual64 selector0, except actual kind9 weapon48.
        if(mode&2u){
            const auto id=models_.definition();
            if(id>=0&&id<500)if(const auto* definition=items_.definition_by_id(id))if(definition->kind()==9){
                const auto frame=models_.weapon();
                if(!frame)throw std::runtime_error("Kind9 projectile position requires actual first-person weapon48");
                const auto matrix=frames_.actor_world_matrix(frame);
                return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
            }
        }
        std::array<float,4> result;
        if(!camera_.query_vector(0,result))throw std::runtime_error("Projectile position requires actual active camera0; native output is unwritten on failure");
        return result;
    }
    template<class Files,class Clock>void activate(std::uint32_t mode,Files& files,Clock& clock) { //004909D0.
        if(!(mode&2u))return;
        if(view_.byte3c||models_.model()||models_.hands())throw std::logic_error("First-person view activation requires cold models");
        view_.byte3c=1;state60_=0;
        try {
            //The original discards BOTH clock results; its local stack slots
            //are reused for vectors. Do not invent a persistent timer owner.
            (void)clock.performance_frequency();(void)clock.performance_counter();
            models_.prepare_loading();view_.bytes58[0]=0;
            for(unsigned axis=0;axis!=3;++axis)position00_[axis]=0;
            //Native position00 fourth lane copies an unwritten stack word.
            //Keep its known lane as a defined guard; actual engine makes quat.
            rotation10_=frames_.first_person_neutral_rotation();
            view_.words84.clear();view_.words70[0]=0;argument98_=0;
            view_.bytes58.fill(0);scale6c_=1;
            (void)FirstPersonAnimationTable::load(files,view_);
            for(unsigned axis=0;axis!=3;++axis)offset20_[axis]=0;
            offset20_[3]=rotation10_[3]; //Same reused stack word AFTER actual quaternion creation.
            field9c_=0;models_.initialize_flash();
            bind_player(mode);reset_zoom(mode); //Native resets zoom again after the bind's own reset.
        }catch(...) {
            frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{models_.close(mode);});errors.rethrow();
        }
    }
    void bind_player(std::uint32_t mode) { //004922A0.
        if(!(mode&2u)||!view_.byte3c)return;
        set_state(mode,0);
        if(auto* old=player())if(old->field160)players_.attached_first_person(*old,false);
        auto* human=players_.current_player();models_.player(human);
        if(!human)return;
        if(human->field160)players_.attached_first_person(*human,view_.bytes58[0]!=0);
        reset_zoom(mode);
        const auto clothing=items_.clothing(*human);
        std::string hands="FPV_hands";
        if(clothing) {
            hands=items_.definition(clothing).resources[0];
            if(hands.find("w_default")!=std::string::npos)hands="FPV_hands";
            else if(const auto dot=hands.rfind('.');dot!=std::string::npos)hands.resize(dot);
        }
        models_.load_hands(mode,hands);
        const auto weapon=items_.weapon(*human);
        if(!weapon||players_.inline_control(*human).first_person_weapon_suppressed()) {
            models_.reset_tracks(mode);models_.bind_weapon(mode,Frame{},Frame{},-1);
        }else {models_.reset_tracks(mode);bind_item(mode,weapon);request(mode,0,0);}
        if(models_.model()) {
            model_visibility(*human);materials_.apply(mode,human,models_.model());frames_.update_scene_object(models_.model());
        }
        set_state(mode,players_.state(*human).first_person_state());update_bob();
    }
    void refresh_weapon(std::uint32_t mode) { //00492500 intentionally does not consult005AC400.
        if(!(mode&2u)||!view_.byte3c||!player())return;
        reset_zoom(mode);const auto weapon=items_.weapon(*player());if(!weapon)return;
        models_.reset_tracks(mode);bind_item(mode,weapon);
        if(models_.model())model_visibility(*player());
        update_bob();
    }
    void request(std::uint32_t mode,std::int32_t animation,std::uint32_t argument) { //00492930.
        if(!(mode&2u)||!view_.byte3c||state60_==3||state60_==4||state60_==5)return;
        if(animation==8)refresh_weapon(mode);
        else if(animation==9) {if(view_.bytes58[2])set_zoom(mode,false,game_fov_);}
        else if(animation==20) {
            reset_zoom(mode);models_.bind_weapon(mode,Frame{},Frame{},-1);models_.playback().animation=-1;set_state(mode,0);return;
        }
        const auto active=models_.animation();
        if((active==11||active==12)&&(animation==7||animation==2||animation==3))return;
        if(!models_.model())return;
        if(!view_.bytes58[2]) {if(animation==21){set_state(mode,3);return;}}
        else {
            if(animation==2)animation=4;else if(animation==3)animation=5;
            else if(animation==0||animation==7||animation==21)return;
        }
        if(animation==0) {
            auto* human=player();if(!human)throw std::runtime_error("First-person idle has no actual player");
            const auto weapon=items_.weapon(*human);
            if(!weapon)throw std::runtime_error("First-person idle has no actual weapon entry");
            if(items_.amount(weapon)==0)animation=1;
        }
        (void)playback_.play(mode,animation);argument98_=argument;
    }
    void reset_zoom(std::uint32_t mode) { //004920B0 always resets camera/cursor on the enabled path.
        if(!(mode&2u)||!view_.byte3c)return;
        view_.bytes58[2]=0;camera_.set_zoom_control(false);fov30_=game_fov_;parameter34_=real(0x3f91361e);
        (void)camera_.set_fov(fov30_);(void)camera_.set_parameter78(parameter34_);cursor_.set_mode(0);
    }
    void set_zoom(std::uint32_t mode,bool enabled,float fov) { //00492130; repeated state makes no calls.
        if(!(mode&2u)||!view_.byte3c||bool(view_.bytes58[2])==enabled)return;
        view_.bytes58[2]=static_cast<std::uint8_t>(enabled);camera_.set_zoom_control(enabled);fov30_=fov;
        (void)playback_.play(mode,enabled?11:12);parameter34_=real(enabled?0x3f32b8c3:0x3f91361e);
        if(!enabled)cursor_.set_mode(0);
        else {
            auto* human=player();if(!human)throw std::runtime_error("First-person zoom has no actual player state");
            if(players_.player_mode(*human,mode)==2)cursor_.set_mode(players_.player_mode(*human,mode));
        }
    }
    void enable(std::uint32_t mode,bool enabled) { //00491FD0 has NO3C activation gate.
        if(!(mode&2u)||bool(view_.bytes58[0])==enabled)return;
        if((mode&7u)!=7u&&players_.game_phase()==2)return;
        view_.bytes58[0]=static_cast<std::uint8_t>(enabled);
        if(auto* human=player())if(human->field160)players_.attached_first_person(*human,enabled);
        update_player_visibility();
        if(const auto model=models_.model()) {
            if(!enabled){frames_.human_equipment_link(model,Frame{},0);reset_zoom(mode);}
            else frames_.human_equipment_link(model,frames_.first_person_scene_root(),0);
            frames_.object_visible(model,enabled);frames_.update_scene_object(model);
        }
        effects_.lensflare_visible(!enabled);
    }
    void secondary_view(std::uint32_t mode,bool enabled) { //004921F0.
        if(!(mode&2u)||!view_.byte3c)return;
        view_.bytes58[1]=static_cast<std::uint8_t>(enabled);
        if(!enabled){enable(mode,view_.bytes58[0]!=0);(void)camera_.set_fov(game_fov_);}
        else {
            (void)camera_.set_fov(real(0x3e32b8c3));
            if(const auto model=models_.model()){frames_.object_visible(model,false);frames_.update_scene_object(model);}
        }
        update_player_visibility();
        auto* human=player();if(!human)throw std::runtime_error("Secondary view cursor has no actual player");
        cursor_.set_mode(players_.player_mode(*human,mode));
    }
private:
    template<class Entry>void bind_item(std::uint32_t mode,Entry entry) {
        const auto model=items_.model(entry);const auto magazine=items_.magazine(entry); //Cache getter precedes native64 read.
        const auto definition=items_.definition(entry).identifier;
        models_.bind_weapon(mode,model,magazine,static_cast<std::int32_t>(definition));
        if(definition==0x26) {
            if(!models_.model())throw std::runtime_error("First-person ammunition has no actual view model");
            const auto ammo=frames_.actor_find(models_.model(),"ammo",1);
            if(!ammo)throw std::runtime_error("First-person ammunition frame is missing");
            frames_.object_visible(ammo,items_.amount(entry)>0);frames_.update_scene_object(ammo);
        }
    }
    void model_visibility(Human& human) {
        const auto model=models_.model();
        if(human.field160&&players_.attached_driver(human))frames_.object_visible(model,false);
        else if(view_.bytes58[1])frames_.object_visible(model,false);
        else if(view_.bytes58[0])frames_.object_visible(model,true);
    }
public:
    //Also called by actual Players0045E5D0 and Human005A5D50 resume, using
    //this SAME view/camera/current-player owner rather than a second facade.
    void update_player_visibility() { //0045DAC0 uses current control player, not necessarily view38.
        auto* human=players_.current_player();if(!human)return;
        if(!camera_.state().flag26d&&camera_.frame())
            camera_.select_loading_human_target(*human,players_.control_sector(*human),view_.bytes58[0]!=0,
                                               view_.bytes58[1]!=0,players_.camera_retirement());
        const bool hidden=human->field160&&players_.attachment_hides_human(*human);
        frames_.object_visible(human->base().frame(),!hidden&&!view_.bytes58[0]&&!view_.bytes58[1]);
    }
private:
    void update_bob(){bob64_=state60_==1?real(0xbe99999a):0.f;}
    static float real(std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;}
    Frames& frames_;View& view_;Models& models_;Items& items_;Players& players_;Camera& camera_;Playback& playback_;Cursor& cursor_;Effects& effects_;
    const float& game_fov_;FirstPersonHandsMaterial<Frames> materials_;
    //Defined cold guards.004909D0 owns original activation stores; no constructor credit.
    std::uint32_t state60_=0,argument98_=0,field9c_=0;float bob64_=0,fov30_=0,parameter34_=0,scale6c_=0;
    std::array<float,4> position00_{},rotation10_{},offset20_{};
};
} //namespace hd2::game
#endif
