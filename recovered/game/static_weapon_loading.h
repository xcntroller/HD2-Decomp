#ifndef HD2_GAME_STATIC_WEAPON_LOADING_H
#define HD2_GAME_STATIC_WEAPON_LOADING_H
#include "game/static_weapon_actor.h"
#include "game/static_weapon_metadata.h"
#include "game/actor_world_corners.h"
#include "game/rigid_body.h"
#include "game/mounted_weapon_control_loading.h"
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
    using Control=MountedWeaponControlLoading<Frames,Services,Items>;
    using Actor=StaticWeaponSceneActor<Frames>;using Frame=typename Frames::Frame;using Entry=typename Frames::Entry;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Clip=decltype(std::declval<Frames&>().load_supply_animation(std::declval<const std::string&>()));
    using Model=decltype(std::declval<Frames&>().create_supply_helper(9));
    struct Seat {void* occupant=nullptr;bool flag=false;Frame frame{};};
    StaticWeaponLoading(Frames& frames,Services& services,Collisions& collisions,Items& items,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),items_(items),actor_(actor),control_(frames,services,items,actor.control){}
    ~StaticWeaponLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    const auto& seats()const{return seats_;}const auto& barrels()const{return control_.barrels();}
    const Descriptor& descriptor()const{return descriptor_;}Entry weapon_item()const{return control_.weapon_item();}
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
        (void)control_.initialize(frame,frame,0);
        actor_.base().name(services_.actor_localized(control_.failed()?0xffffffffu:items_.runtime_item(control_.weapon_item()).definition().caption)); //00604D90→004C57D0→006E4FB0.
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
        if(control_.occupied()||actor_.control.flagd4||actor_.control.flagdc)throw std::runtime_error("StaticWeapon occupied control requires actual targeting/firing receivers");
        (void)elapsed;
        if(!authority&&!seats_.empty()&&seats_.front().occupant)throw std::runtime_error("StaticWeapon occupied seat requires model/seat simulation receivers");
        //004C55E0 and004C56A0 both return after their true null controlFC gate.
    }
    void fire_marker()override{control_.fire_marker();}
    std::array<float,4> aim_direction()override{return control_.aim_direction();}

private:
    static std::string indexed(const char* format,unsigned first,unsigned second=0) {
        char text[64];const auto size=std::snprintf(text,sizeof(text),format,first,second);
        if(size<0||static_cast<std::size_t>(size)>=sizeof(text))throw std::runtime_error("StaticWeapon authored name exceeds native bounds");
        return text;
    }
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
    void retire() {
        frontend::CleanupErrors errors;ready_=false;seats_.clear();errors.attempt([&]{control_.retire();});
        if(actor_.flag159){actor_.flag159=false;errors.attempt([&]{set_state(3);});}
        if(descriptor_ready_)collisions_.remove_for_cleanup(descriptor_);
        actor_.flag150=false;descriptor_ready_=false;
        helper_.reset();errors.rethrow();
    }
    Frames& frames_;Services& services_;Collisions& collisions_;Items& items_;Actor& actor_;
    std::vector<Seat> seats_;std::optional<Model> helper_;Control control_;
    typename Frames::Matrix inverse_{};typename Frames::Vector navigation_position_{};
    Descriptor descriptor_{};bool ready_=false,descriptor_ready_=false;
};
}
#endif
