#ifndef HD2_GAME_STATIC_WEAPON_LOADING_H
#define HD2_GAME_STATIC_WEAPON_LOADING_H
#include "game/static_weapon_actor.h"
#include "game/static_weapon_metadata.h"
#include "game/actor_world_corners.h"
#include "game/rigid_body.h"
#include "frontend/cleanup.h"
#include <optional>
#include <cstdio>
namespace hd2::game {
//006036D0/004C1AD0/00604120, reached offline mode7 loading boundary.
//Real Items/model/marker/navigation/bucket owners are required, never defaults.
//Occupied-seat simulation, firing and non7 network/input are separate receivers.
template<class Frames,class Services,class Collisions,class Items>
class StaticWeaponLoading final:public StaticWeaponLoadingLifetime {
public:
    using Actor=StaticWeaponSceneActor<Frames>;using Frame=typename Frames::Frame;using Entry=typename Frames::Entry;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Clip=decltype(std::declval<Frames&>().load_supply_animation(std::declval<const std::string&>()));
    using Model=decltype(std::declval<Frames&>().create_supply_helper(9));
    struct Seat {void* occupant=nullptr;bool flag=false;Frame frame{};};
    struct Barrel {Frame frame{},shell{},cartridge{};std::optional<Clip> clip;};
    struct AxisAngle {typename Frames::Vector axis{};float angle=0;};
    StaticWeaponLoading(Frames& frames,Services& services,Collisions& collisions,Items& items,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),items_(items),actor_(actor){}
    ~StaticWeaponLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    const auto& seats()const{return seats_;}const auto& barrels()const{return barrels_;}
    const Descriptor& descriptor()const{return descriptor_;}Entry weapon_item()const{return item_;}
    void activate(bool authority)override { //006036D0 common-first; once guard covers derived pass only.
        actor_.base().activate_frame(authority,services_);
        if(actor_.flag159){if(!ready_)throw std::logic_error("Retire partial StaticWeapon activation before retry");return;}
        actor_.flag159=true;ready_=false;
        if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("StaticWeapon requires actual non7 input/network loading owners");
        const auto frame=actor_.base().frame();
        for(unsigned n=0;;++n) {
            if(n>10000)throw std::runtime_error("StaticWeapon seat count exceeds source bounds");
            const auto seat=frames_.actor_find(frame,indexed("SEAT%02u",n).c_str(),0x20);if(!seat)break;
            seats_.push_back({nullptr,false,seat});
        }
        initialize_control();
        actor_.base().name(services_.actor_localized(failed_?0xffffffffu:items_.runtime_item(item_).definition().caption)); //00604D90→004C57D0→006E4FB0.
        initialize_collision();
        navigation_position_=services_.static_weapon_navigation_position(actor_); //006055C0 over same session table.
        helper_.emplace(frames_.create_supply_helper(9));
        if(!helper_->get())throw std::runtime_error("Cannot create StaticWeapon camera model");
        //Native optional C<full original resource> model, flags10; missing clears154.
        if(frames_.load_supply_model(helper_->get(),"C"+frames_.car_model_resource(frame),0x10)!=0)helper_.reset();
        set_state(1);ready_=true;
    }
    void deactivate(bool authority)override { //00604120 common first, seats clear, control then state3.
        frontend::CleanupErrors errors;errors.attempt([&]{actor_.base().deactivate_frame(authority,services_);});
        seats_.clear();errors.attempt([&]{retire();});errors.rethrow();
    }
    void update(bool authority,std::uint32_t elapsed)override { //00603070/006034D0 cold ownerFC=0 path.
        if(!ready_)throw std::logic_error("StaticWeapon first update requires completed loading");
        if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("StaticWeapon update requires actual non7 input/network receivers");
        if(control_occupant_||actor_.control.flagd4||actor_.control.flagdc)throw std::runtime_error("StaticWeapon occupied control requires actual targeting/firing receivers");
        (void)elapsed;
        if(!authority&&!seats_.empty()&&seats_.front().occupant)throw std::runtime_error("StaticWeapon occupied seat requires model/seat simulation receivers");
        //004C55E0 and004C56A0 both return after their true null controlFC gate.
    }
    void fire_marker()override {throw std::runtime_error("StaticWeapon S marker requires actual004C5080 firing receiver");}
private:
    static std::string indexed(const char* format,unsigned first,unsigned second=0) {
        char text[64];const auto size=std::snprintf(text,sizeof(text),format,first,second);
        if(size<0||static_cast<std::size_t>(size)>=sizeof(text))throw std::runtime_error("StaticWeapon authored name exceeds native bounds");
        return text;
    }
    int metadata(Frame frame,const char* key){return frame?static_weapon_metadata(frames_.actor_user_data(frame),key):-1;}
    void initialize_control() { //004C1AD0(actor,model,model,0).
        if(actor_.control.flag104)throw std::logic_error("Retire actual StaticWeapon control before reinitializing");
        actor_.control.flag104=true;failed_=false;control_occupant_=nullptr;
        auto& c=actor_.control;c.fieldc8=0;c.field13c=c.field140=c.field144=0;
        const auto model=actor_.base().frame();root_=frames_.actor_parent(model);
        if(!root_||frames_.actor_short_name(root_).compare(0,6,"HORT00")!=0)root_=model;
        view_=frames_.actor_find(model,"VIEW00",0x10001);
        if(view_){frames_.object_visible(view_,false);frames_.update_scene_object(view_);} //004C6D90(0).
        const auto first=frames_.actor_find(model,"BARREL00_00",0x10001);const auto definition=metadata(first,"itemid");
        if(definition>=0)item_=items_.create_static_weapon_item(definition);
        if(!item_){failed_=true;return;}
        const auto& payload=items_.runtime_item(item_).definition().payloads[0];
        if(payload.bytes.size()<28)throw std::runtime_error("StaticWeapon item requires its actual weapon payload");
        std::memcpy(&ammunition_,payload.bytes.data()+24,4); //Serialized payload begins at native+04; native48+1C is byte24.
        c.flag168=items_.runtime_item(item_).primary_payload_kind()==2;
        for(unsigned n=0;;++n) {
            if(n>10000)throw std::runtime_error("StaticWeapon barrel count exceeds source bounds");
            const auto frame=frames_.actor_find(model,indexed("BARREL%02u_%02u",0,n).c_str(),0x10001);if(!frame)break;
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
            key+=indexed("_f%02u_%02u",0,n);
            std::optional<Clip> clip;clip.emplace(frames_.load_supply_animation(key));
            if(!shell)continue; //Original append/hook guard is SHDUM, not clip availability.
            barrels_.push_back({frame,shell,cartridge,std::move(clip)});
            services_.install_static_weapon_events(frame,*this); //Publishes same frameEC word7 and actual004C1990 hook.
        }
        horizontal_=frames_.actor_find(model,"HORT00*",0x10020);vertical_=frames_.actor_find(model,"VERT00*",0x10020);aim_=frames_.actor_find(model,"AIM00*",0x10020);
        shells_.emplace(frames_.static_weapon_shells());if(shells_->get())frames_.object_visible(shells_->get(),false);
        if(!horizontal_||!vertical_||barrels_.empty()){failed_=true;return;}
        frames_.static_weapon_axis_angle(vertical_,vertical_pose_.axis,vertical_pose_.angle);
        frames_.update_scene_object(vertical_);frames_.update_scene_object(horizontal_);
        frames_.static_weapon_axis_angle(horizontal_,horizontal_pose_.axis,horizontal_pose_.angle);poses_=true;
        if(metadata(horizontal_,"soundm")>=0||metadata(horizontal_,"sounds")>=0)throw std::runtime_error("StaticWeapon authored movement sounds require actual004C6E50 sound owners");
        projectile_=frames_.actor_find(model,"projectile",1);
    }
    static std::uint32_t bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    static float radians(){const std::uint32_t word=0x3c8efa35;float value;std::memcpy(&value,&word,4);return value;}
    void initialize_collision() {
        const auto frame=actor_.base().frame();const auto bounds=actor_corners_detail::local_bounds(frames_,frame,true);
        typename Frames::Vector centre{};float radius=0;
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("StaticWeapon bounds leave native sphere centre undefined");
        {
            typename Frames::Vector half{};
            for(unsigned a=0;a!=3;++a){const float extent=static_cast<float>(static_cast<long double>(bounds.maximum[a])-bounds.minimum[a]);half[a]=static_cast<float>(static_cast<long double>(extent)*.5f);centre[a]=static_cast<float>(static_cast<long double>(bounds.minimum[a])+half[a]);}
            radius=static_cast<float>(std::sqrt(static_cast<long double>(half[2])*half[2]+static_cast<long double>(half[1])*half[1]+static_cast<long double>(half[0])*half[0]));
        }
        descriptor_={};descriptor_.flags=0x40;descriptor_.mask=0x800000;descriptor_.frame=frame;descriptor_.actor=&actor_;descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=frames_.actor_world_matrix_pointer(frame);inverse_=frames_.car_inverse_matrix(*descriptor_.matrix);descriptor_.inverse=&inverse_;
        const auto world=frames_.actor_world_matrix(frame);descriptor_.position={{world[12],world[13],world[14],world[15]}};
        const auto scale=frames_.static_weapon_world_scale(frame);descriptor_.radius=static_cast<float>(static_cast<long double>(std::max(scale[0],std::max(scale[1],scale[2])))*radius);
        descriptor_.position=frames_.car_transform(centre,world);descriptor_ready_=true;
    }
    void set_state(unsigned state) { //00605AE0 reached1/3. Native3 restores only when flag158 has an actual producer.
        if(actor_.field2f0==state)return;
        actor_.field2f0=state;
        if(state==1){frames_.object_visible(actor_.base().frame(),true);if(!actor_.flag150)collisions_.insert(descriptor_);actor_.flag150=true;}
        else if(state==3){if(actor_.flag158)throw std::runtime_error("StaticWeapon retirement requires actual destroyed-pose restore receiver");if(actor_.flag150)collisions_.remove(descriptor_);actor_.flag150=false;}
        else throw std::runtime_error("StaticWeapon state requires its actual destruction receiver");
    }
    void retire_control() { //004C2380; callback/backlink retirement additionally guards native C++ lifetimes.
        frontend::CleanupErrors errors;
        if(actor_.control.flag104){actor_.control.flag104=false;
            if(poses_&&!failed_) {
                errors.attempt([&]{frames_.static_weapon_restore_axis(horizontal_,horizontal_pose_.axis,horizontal_pose_.angle);});
                errors.attempt([&]{frames_.static_weapon_restore_axis(vertical_,vertical_pose_.axis,vertical_pose_.angle);});
            }
            errors.attempt([&]{frames_.update_scene_object(actor_.base().frame());});
        }
        if(item_){const auto item=std::exchange(item_,Entry{});errors.attempt([&]{items_.remove_runtime_item(item);});}
        for(auto& barrel:barrels_)errors.attempt([&]{services_.remove_static_weapon_events(barrel.frame,*this);});
        barrels_.clear();shells_.reset();horizontal_=vertical_=aim_=Frame{};poses_=false;errors.rethrow();
    }
    void retire() {
        frontend::CleanupErrors errors;ready_=false;seats_.clear();errors.attempt([&]{retire_control();});
        if(actor_.flag159){actor_.flag159=false;errors.attempt([&]{set_state(3);});}
        if(descriptor_ready_)collisions_.remove_for_cleanup(descriptor_);
        actor_.flag150=false;descriptor_ready_=false;
        helper_.reset();errors.rethrow();
    }
    Frames& frames_;Services& services_;Collisions& collisions_;Items& items_;Actor& actor_;
    std::vector<Seat> seats_;std::vector<Barrel> barrels_;Entry item_{};std::optional<Model> shells_,helper_;
    Frame root_{},view_{},horizontal_{},vertical_{},aim_{},projectile_{};void* control_occupant_=nullptr;
    AxisAngle horizontal_pose_,vertical_pose_;typename Frames::Matrix inverse_{};typename Frames::Vector navigation_position_{};
    Descriptor descriptor_{};std::uint32_t ammunition_=0;bool ready_=false,descriptor_ready_=false,failed_=true,poses_=false;
};
}
#endif
