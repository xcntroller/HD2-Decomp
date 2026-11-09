#ifndef HD2_GAME_FIRST_PERSON_MODEL_LOADING_H
#define HD2_GAME_FIRST_PERSON_MODEL_LOADING_H

#include "game/first_person_playback_state.h"
#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace hd2::game {

//00490930/00491130/004932A0/00490B50 model-loading receiver. The containing
//view supplies the SAME OverlayOwner and must outlive this receiver. Item-cache
//weapon/magazine borrowers must outlive it. No player binding, clip selection,
//material transfer00492610 or containing004909D0 activation is supplied here.
template<class Frames,class View>
class FirstPersonModelLoading {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_item_template_model());
    FirstPersonModelLoading(Frames& frames,View& view):frames_(frames),view_(view) {}
    FirstPersonModelLoading(const FirstPersonModelLoading&)=delete;
    FirstPersonModelLoading& operator=(const FirstPersonModelLoading&)=delete;
    ~FirstPersonModelLoading() {try{close_models();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    void* player()const{return player_;}
    void player(void* actor){player_=actor;} //004922A0 owns the containing publication.
    void prepare_loading() { //004909D0 scalar/borrower stores; caller requires cold models.
        weapon_=magazine_=Frame{};definition_=0;playback_.current.clip=Frame{};
        playback_.fading.clear();playback_.animation=playback_.variant=0;
    }
    void initialize_flash() {flash_=Frame{};flash_time_=-1.f;}
    Frame model() const {return model_?model_->get():Frame{};}
    Frame hands() const {return hands_?hands_->get():Frame{};}
    Frame weapon() const {return weapon_;}
    Frame magazine() const {return magazine_;}
    Frame flash() const {return flash_;}
    float flash_time() const {return flash_time_;}
    std::int32_t definition() const {return definition_;}
    std::int32_t animation() const {return playback_.animation;}
    FirstPersonPlaybackState<Frame>& playback() {return playback_;}
    void flash_time(float value) {flash_time_=value;}

    void load_hands(std::uint32_t mode,const std::string& key) { //00490930 accepts any nonnegative resource result.
        if(!(mode&2u)||!view_.byte3c)return;
        hands_.reset();auto owner=frames_.create_item_template_model();
        if(!owner)throw std::runtime_error("First-person hands model factory returned null");
        if(frames_.load_item_template_model(owner.get(),key)<0)return;
        hands_.emplace(std::move(owner));
        frames_.human_equipment_name(hands(),"FPV_HANDS");
        frames_.human_equipment_link(hands(),frames_.first_person_scene_root(),0);
        frames_.object_visible(hands(),false);
    }
    void reset_tracks(std::uint32_t mode) { //004932A0, descending slots.
        if(!(mode&2u)||!model())return;
        for(unsigned slot=8;slot;) {
            --slot;frames_.human_model_time(model(),slot,0);
            frames_.human_model_remove_animation(model(),slot);
            frames_.human_model_animation(model(),Frame{},slot,1.f,0);
        }
        view_.words84.clear();view_.words70[0]=0;playback_.fading.clear();playback_.current.clip=Frame{};
        frames_.first_person_model_advance(model(),100);frames_.update_scene_object(model());playback_.animation=0;
    }
    void bind_weapon(std::uint32_t mode,Frame weapon,Frame magazine,std::int32_t definition) { //00491130.
        if(!(mode&2u)||!view_.byte3c||!hands())return;
        if(model()) {
            if(weapon_) {
                frames_.first_person_model_remove(model(),weapon_);
                frames_.human_equipment_link(weapon_,frames_.camera_root_sector(),0);
                children(weapon_,[&](Frame child){frames_.first_person_model_remove(model(),child);});
                frames_.object_visible(weapon_,false);
            }
            if(magazine_) {
                frames_.first_person_model_remove(model(),magazine_);
                frames_.human_equipment_link(magazine_,frames_.camera_root_sector(),0);
                frames_.object_visible(magazine_,false);
            }
            model_.reset();
        }
        weapon_=weapon;magazine_=magazine;definition_=definition;
        if(!weapon_) {flash_=Frame{};flash_time_=-1.f;return;}
        auto owner=frames_.create_item_template_model();
        if(!owner)throw std::runtime_error("First-person view model factory returned null");
        model_.emplace(std::move(owner));frames_.copy_item_frame(model(),hands());
        if(view_.bytes58[0])frames_.human_equipment_link(model(),frames_.first_person_scene_root(),1);
        frames_.first_person_model_add(model(),weapon_);frames_.human_equipment_link(weapon_,model(),1);
        frames_.object_visible(weapon_,true);
        children(weapon_,[&](Frame child){frames_.first_person_model_add(model(),child);});
        if(magazine_) {
            frames_.first_person_model_add(model(),magazine_);frames_.human_equipment_link(magazine_,model(),1);
            frames_.object_visible(magazine_,true);
        }
        flash_=frames_.actor_find(model(),"flash",1);
        if(flash_)frames_.object_visible(flash_,false);else flash_time_=-1.f;
        reset_pose(weapon_);if(magazine_)reset_pose(magazine_);
        frames_.object_visible(model(),view_.bytes58[0]!=0);frames_.update_scene_object(model());
    }
    void close(std::uint32_t mode) { //00490B50; mode gate belongs to original explicit caller.
        if(!(mode&2u))return;
        close_models();
    }
private:
    template<class Visit>void children(Frame parent,Visit&& visit) {
        auto count=frames_.actor_child_count(parent);
        if(count>0x100000u)throw std::runtime_error("First-person weapon child count exceeds bounds");
        while(count) {const auto child=frames_.actor_child(parent,--count);
            if(!child)throw std::runtime_error("First-person weapon has a null child");
            visit(child);}
    }
    void reset_pose(Frame frame) {
        //Native writes XYZ only; preserve its fourth SIMD lane.
        frames_.first_person_zero_position(frame);
        frames_.first_person_forward(frame);
        frames_.object_visible(frame,true);frames_.update_scene_object(frame);
    }
    void close_models() {
        std::exception_ptr failure;
        auto attempt=[&](auto&& operation){try{operation();}catch(...){if(!failure)failure=std::current_exception();}};
        if(model()) {
            if(weapon_) {
                attempt([&]{children(weapon_,[&](Frame child){frames_.first_person_model_remove(model(),child);});});
                attempt([&]{frames_.first_person_model_remove(model(),weapon_);});
                attempt([&]{frames_.human_equipment_link(weapon_,Frame{},0);});
            }
            if(magazine_) {
                attempt([&]{frames_.first_person_model_remove(model(),magazine_);});
                attempt([&]{frames_.human_equipment_link(magazine_,Frame{},0);});
            }
            attempt([&]{reset_tracks(2);});
            attempt([&]{frames_.human_model_remove_animation(model(),0);});
            attempt([&]{frames_.update_scene_object(model());});model_.reset();
        }
        hands_.reset();weapon_=magazine_=flash_=Frame{};player_=nullptr;
        view_.bytes58[0]=0;view_.words84.clear();view_.words70[0]=0;view_.word38=0;view_.byte3c=0;
        if(failure)std::rethrow_exception(failure);
    }
    Frames& frames_;View& view_;
    std::optional<Owner> model_,hands_;
    Frame weapon_{},magazine_{},flash_{};
    void* player_=nullptr; //Native38 actor identity; never truncate it into a DWORD during native audits.
    std::int32_t definition_=0;
    FirstPersonPlaybackState<Frame> playback_;
    float flash_time_=-1.f; //Defined cold guard; activation owns native4C360 initialization.
};
} //namespace hd2::game
#endif
