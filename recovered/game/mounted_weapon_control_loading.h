#ifndef HD2_GAME_MOUNTED_WEAPON_CONTROL_LOADING_H
#define HD2_GAME_MOUNTED_WEAPON_CONTROL_LOADING_H

#include "game/mounted_weapon_control.h"
#include "game/static_weapon_metadata.h"
#include "game/human_ai_shot_direction.h"
#include "game/mounted_weapon_movement_sounds.h"
#include "frontend/cleanup.h"
#include <cstdio>
#include <optional>

namespace hd2::game {

//Shared004C1AD0/004C2380 resource lifetime. The actual actor owns control state,
//model and pivot. This owner retains its real item, barrel clips and shell model.
template<class Frames,class Services,class Items>
class MountedWeaponControlLoading final:public MountedWeaponControlLifetime {
public:
    using Frame=typename Frames::Frame;using Entry=typename Frames::Entry;
    using Clip=decltype(std::declval<Frames&>().load_supply_animation(std::declval<const std::string&>()));
    using Model=decltype(std::declval<Frames&>().create_supply_helper(9));
    struct Barrel {Frame frame{},shell{},cartridge{};std::optional<Clip> clip;};
    struct AxisAngle {typename Frames::Vector axis{};float angle=0;};
    MountedWeaponControlLoading(Frames& frames,Services& services,Items& items,StaticWeaponControl& control)
        :frames_(frames),services_(services),items_(items),control_(control),movement_sounds_(frames){}
    ~MountedWeaponControlLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    const auto& barrels()const{return barrels_;}Entry weapon_item()const{return item_;}
    bool failed()const{return failed_;}
    bool occupied()const{return control_occupant_!=nullptr;}
    auto& movement_sounds(){return movement_sounds_;}
    void clear_occupant()override{ //004C6DD0(NULL), actual null-only caller reached by reaction seat release.
        control_occupant_=nullptr;view(false);
        movement_sounds_.release_motion(control_.field14c);
    }
    void fire_marker()override{throw std::runtime_error("Mounted weapon S marker requires actual004C5080 firing receiver");}
    std::array<float,4> limits()const override{return {{control_.field128,control_.field12c,control_.field130,control_.field134}};}
    void view(bool visible)override{ //004C6D90, actual retained F4 frame.
        if(!control_.flag104||!view_)return;
        frames_.object_visible(view_,visible);frames_.update_scene_object(view_);
    }
    std::array<float,4> aim_direction()override{ //00604D70→004C5870, actual control140 is VERT00.
        if(failed_)return {};
        if(!vertical_)throw std::logic_error("StaticWeapon aim requires its actual vertical pivot");
        const auto world=frames_.actor_world_matrix(vertical_);
        std::array<float,4> direction{{world[8],world[9],world[10],world[11]}};
        const auto square=(static_cast<long double>(direction[1])*direction[1]+
            static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
        normalize_human_ai_shot(direction,square);
        return direction;
    }
    bool initialize(Frame model,Frame pivot,unsigned index) { //004C1AD0(actor,model,pivot,index).
        if(control_.flag104)throw std::logic_error("Retire actual StaticWeapon control before reinitializing");
        if(!model||!pivot)return false;
        model_=model;control_.flag104=true;failed_=false;control_occupant_=nullptr;
        auto& c=control_;c.fieldc8=index;c.field13c=c.field140=c.field144=c.field14c=0;
        root_=frames_.actor_parent(pivot);
        if(!root_||frames_.actor_short_name(root_).compare(0,6,indexed("HORT%02u",index))!=0)root_=pivot;
        view_=frames_.actor_find(model,indexed("VIEW%02u",index).c_str(),0x10001);
        view(false); //004C6D90(0).
        const auto first=frames_.actor_find(model,indexed("BARREL%02u_%02u",index,0).c_str(),0x10001);const auto definition=metadata(first,"itemid");
        if(definition>=0)item_=items_.create_static_weapon_item(definition);
        if(!item_){failed_=true;return false;}
        const auto& payload=items_.runtime_item(item_).definition().payloads[0];
        if(payload.bytes.size()<28)throw std::runtime_error("StaticWeapon item requires its actual weapon payload");
        std::memcpy(&ammunition_,payload.bytes.data()+24,4); //Serialized payload begins at native+04; native48+1C is byte24.
        c.flag168=items_.runtime_item(item_).primary_payload_kind()==2;
        for(unsigned n=0;;++n) {
            if(n>10000)throw std::runtime_error("StaticWeapon barrel count exceeds source bounds");
            const auto frame=frames_.actor_find(model,indexed("BARREL%02u_%02u",index,n).c_str(),0x10001);if(!frame)break;
            if(n==0) {
                const auto degrees=[](int value,float multiplier,float absent){return value==-1?absent:static_cast<float>(static_cast<long double>(static_cast<float>(value))*multiplier);};
                c.field164=bits(degrees(metadata(frame,"vs"),radians(),.7853981852531433f));
                c.field160=bits(degrees(metadata(frame,"hs"),radians(),.7853981852531433f));
                c.field128=degrees(metadata(frame,"up"),radians(),1000000.f);
                c.field12c=degrees(metadata(frame,"down"),-radians(),1000000.f);
                c.field130=degrees(metadata(frame,"left"),-radians(),1000000.f);
                c.field134=degrees(metadata(frame,"right"),radians(),1000000.f);
            }
            const auto shell=frames_.actor_find(frame,"SHDUM*",0x10020),cartridge=frames_.actor_find(frame,"CARDUM*",0x10020);
            auto key=frames_.car_model_resource(model);const auto slash=key.find_last_of('\\');if(slash!=std::string::npos)key.erase(0,slash+1);
            const auto dot=key.find_last_of('.');if(dot!=std::string::npos)key.erase(dot);
            key+=indexed("_f%02u_%02u",index,n);
            std::optional<Clip> clip;clip.emplace(frames_.load_supply_animation(key));
            if(!shell)continue; //Original append/hook guard is SHDUM, not clip availability.
            barrels_.push_back({frame,shell,cartridge,std::move(clip)});
            services_.install_static_weapon_events(frame,*this); //Publishes same frameEC word7 and actual004C1990 hook.
        }
        horizontal_=frames_.actor_find(model,indexed("HORT%02u*",index).c_str(),0x10020);vertical_=frames_.actor_find(model,indexed("VERT%02u*",index).c_str(),0x10020);aim_=frames_.actor_find(model,indexed("AIM%02u*",index).c_str(),0x10020);
        shells_.emplace(frames_.static_weapon_shells());if(shells_->get())frames_.object_visible(shells_->get(),false);
        if(!horizontal_||!vertical_||barrels_.empty()){failed_=true;return false;}
        frames_.static_weapon_axis_angle(vertical_,vertical_pose_.axis,vertical_pose_.angle);
        frames_.update_scene_object(vertical_);frames_.update_scene_object(horizontal_);
        frames_.static_weapon_axis_angle(horizontal_,horizontal_pose_.axis,horizontal_pose_.angle);poses_=true;
        movement_sounds_.load(horizontal_,[&](const char* key){return metadata(horizontal_,key);},services_);
        projectile_=frames_.actor_find(model,"projectile",1);return true;
    }
    void retire()override { //004C2380; callback/backlink retirement additionally guards native C++ lifetimes.
        frontend::CleanupErrors errors;
        if(control_.flag104){control_.flag104=false;
            if(poses_&&!failed_) {
                errors.attempt([&]{frames_.static_weapon_restore_axis(horizontal_,horizontal_pose_.axis,horizontal_pose_.angle);});
                errors.attempt([&]{frames_.static_weapon_restore_axis(vertical_,vertical_pose_.axis,vertical_pose_.angle);});
            }
            errors.attempt([&]{frames_.update_scene_object(model_);});
        }
        if(item_){const auto item=std::exchange(item_,Entry{});errors.attempt([&]{items_.remove_runtime_item(item);});}
        for(auto& barrel:barrels_)errors.attempt([&]{services_.remove_static_weapon_events(barrel.frame,*this);});
        barrels_.clear();shells_.reset();horizontal_=vertical_=aim_=Frame{};poses_=false;
        movement_sounds_.clear();errors.rethrow(); //004C7630 follows pivot/shell retirement.
    }
private:
    int metadata(Frame frame,const char* key){return frame?static_weapon_metadata(frames_.actor_user_data(frame),key):-1;}
    static std::uint32_t bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    static float radians(){const std::uint32_t word=0x3c8efa35;float value;std::memcpy(&value,&word,4);return value;}
    static std::string indexed(const char* format,unsigned first,unsigned second=0) {
        char text[64];const auto size=std::snprintf(text,sizeof(text),format,first,second);
        if(size<0||static_cast<std::size_t>(size)>=sizeof(text))throw std::runtime_error("StaticWeapon authored name exceeds native bounds");
        return text;
    }

    Frames& frames_;Services& services_;Items& items_;StaticWeaponControl& control_;
    std::vector<Barrel> barrels_;Entry item_{};std::optional<Model> shells_;
    Frame model_{},root_{},view_{},horizontal_{},vertical_{},aim_{},projectile_{};
    AxisAngle horizontal_pose_,vertical_pose_;
    void* control_occupant_=nullptr;
    std::uint32_t ammunition_=0;
    bool failed_=true,poses_=false;
    MountedWeaponMovementSounds<Frames> movement_sounds_;
};

} //namespace hd2::game
#endif
