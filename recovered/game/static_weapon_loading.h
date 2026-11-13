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
    bool presentation_failed()const{return control_.failed();} //004C57B0, SAME shared mounted owner.
    std::array<float,4> mounted_position()override{return control_.mounted_position();} //00604D40.
    bool can_target(const std::array<float,4>& target)override{return control_.can_target(target);} //006055A0.
    void seat_frame_inputs(std::int32_t seat,const HumanFrameInputBuffer& first,const HumanFrameInputBuffer& second)override{ //00604CF0 has no signed bounds gate.
        if(seats_.at(static_cast<std::size_t>(seat)).occupant)control_.frame_inputs(first,second);
    }
    void presentation_view(bool enabled){control_.view(enabled);} //004C6D90.
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
        control_.advance(authority,elapsed);
        if(!authority&&!seats_.empty()&&seats_.front().occupant){frames_.supply_model_tick(actor_.base().frame(),elapsed);refresh_seat_collision();}
    }
    void fire_marker(void* barrel)override{control_.fire_marker(barrel);}
    std::array<float,4> aim_direction()override{return control_.aim_direction();}
    void* seat_occupant(std::int32_t index)const override{
        return index<0||static_cast<std::size_t>(index)>=seats_.size()?nullptr:seats_[static_cast<std::size_t>(index)].occupant;
    }
    void assign_seat(std::int32_t index,void* value)override{ //006041A0, unlike Car, even the SAME existing occupant rejects.
        if(index<0||static_cast<std::size_t>(index)>=seats_.size())return;
        auto& seat=seats_[static_cast<std::size_t>(index)];if(seat.occupant)return;
        if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("StaticWeapon seat entry requires actual non7 packet owner");
        if(value){
            auto* actor=static_cast<SceneActor<Frames>*>(value);const auto model=actor->base().frame();
            frames_.human_seat_link(model,seat.frame);frames_.human_seat_origin(model);
            auto world=frames_.actor_world_matrix(seats_.front().frame);actor->base().world_position({{world[12],world[13],world[14],world[15]}}); //Native reads FIRST seat for base pose.
            world=frames_.actor_world_matrix(seats_.front().frame);typename Frames::Vector direction{{world[8],world[9],world[10],world[11]}};
            const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
            normalize_human_ai_shot(direction,square);actor->base().world_direction(direction);
            services_.seat_human_direction(actor,direction);frames_.update_scene_object(actor_.base().frame());
        }
        seat.occupant=value;control_.set_occupant(value);refresh_seat_collision();services_.seat_script_use(actor_.base().frame());
    }
    bool exit_seat(std::int32_t index)override{ //006046D0, fixed navigation fallback; no Car collision search.
        if(index<0||static_cast<std::size_t>(index)>=seats_.size())return false;
        auto& seat=seats_[static_cast<std::size_t>(index)];if(!seat.occupant)return false;
        auto placed=navigation_position_;const auto square=(static_cast<long double>(placed[2])*placed[2]+static_cast<long double>(placed[1])*placed[1])+static_cast<long double>(placed[0])*placed[0];
        if(!(square>=1.0000000168623835e-16f)){
            auto world=frames_.actor_world_matrix(actor_.base().frame());typename Frames::Vector offset=placed;
            for(unsigned axis=0;axis!=3;++axis)offset[axis]=-world[8+axis];
            world=frames_.actor_world_matrix(actor_.base().frame());for(unsigned axis=0;axis!=3;++axis)placed[axis]=offset[axis]+world[12+axis];
        }
        const auto world=frames_.actor_world_matrix(actor_.base().frame());typename Frames::Vector direction{{world[8],world[9],world[10],world[11]}};
        const float length=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
        normalize_human_ai_shot(direction,length);
        auto* actor=static_cast<SceneActor<Frames>*>(seat.occupant);frames_.human_seat_link(actor->base().frame(),frames_.camera_root_sector());
        frames_.human_seat_exit_pose(actor->base().frame(),placed,direction);
        seat.occupant=nullptr;control_.clear_occupant();refresh_seat_collision();return true;
    }
    std::optional<std::uint32_t> seat_weapon_kind()override{
        if(control_.failed())return std::nullopt;
        return items_.runtime_item(control_.weapon_item()).primary_payload_kind(); //00604DA0→004C57B0, native definition48 virtual0C.
    }
    void remove_reaction_seat(std::int32_t index)override{ //00604630, absent frozen inventory.
        if(index<0||static_cast<std::size_t>(index)>=seats_.size())return;
        auto& seat=seats_[static_cast<std::size_t>(index)];
        if(seat.occupant)frames_.human_reaction_link(static_cast<SceneActor<Frames>*>(seat.occupant)->base().frame(),frames_.camera_root_sector());
        seat.occupant=nullptr;seat.flag=false;control_.clear_occupant();
        if(actor_.flag150){ //00604E90 updates the SAME registered descriptor.
            frames_.update_scene_object(actor_.base().frame());initialize_collision();collisions_.move(descriptor_);
        }
    }
    bool seat_claimed(std::int32_t index)const override{return index<0||static_cast<std::size_t>(index)>=seats_.size()?true:seats_[static_cast<std::size_t>(index)].flag;} //00604A90 invalid seats are CLOSED.
    void claim_seat(std::int32_t index,bool value)override{if(index>=0&&static_cast<std::size_t>(index)<seats_.size())seats_[static_cast<std::size_t>(index)].flag=value;} //00604AD0.
    std::uint32_t reaction_seat_flags(std::int32_t index)const override{ //00604A10, first case-sensitive SEAT substring.
        if(index<0||static_cast<std::size_t>(index)>=seats_.size())return 0;
        const auto text=frames_.actor_user_data(seats_[static_cast<std::size_t>(index)].frame);const auto at=text.find("SEAT");
        if(at==std::string::npos)return 0;
        std::uint32_t flags=0;
        for(auto offset=at+4;offset<text.size()&&text[offset]!=','&&text[offset]!=' ';++offset)
            flags|=1u<<((static_cast<unsigned char>(text[offset])-0x30u)&31u);
        return flags;
    }

private:
    void refresh_seat_collision(){ //00604E90, SAME stable descriptor.
        if(actor_.flag150){frames_.update_scene_object(actor_.base().frame());initialize_collision();collisions_.move(descriptor_);}
    }
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
