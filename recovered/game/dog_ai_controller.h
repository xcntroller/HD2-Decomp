#ifndef HD2_GAME_DOG_AI_CONTROLLER_H
#define HD2_GAME_DOG_AI_CONTROLLER_H
#include "game/dog_controller_lifetime.h"
#include "game/dog_damage.h"
#include "game/dog_route_geometry.h"
#include "game/dog_controller_inputs.h"
#include "game/dog_controller_follow.h"
#include "game/dog_controller_targets.h"
#include "game/dog_controller_actions.h"
#include "game/dog_controller_movement.h"
#include "game/dog_route_rectangle.h"
#include <cmath>
#include <exception>
#include <limits>
namespace hd2::game {
//Actual kind4/5 controller owner on the SAME Body, process scratch/pools and
//World. Native unproduced detour payloads remain explicit provenance failures.
template<class Frames,class World>class DogAIController final:public DogControllerLifetime<Frames> {
public:
    using Actor=SceneActor<Frames>;using State=DogControllerState<Frames>;
    DogAIController(std::int32_t kind,Frames& frames,World& world):state_(kind),frames_(frames),world_(world){
        if(kind!=4&&kind!=5)throw std::logic_error("Dog AI controller requires actual kind4/5");
        state_.navigation70=world_.dog_navigation_owner(); //005CB0D0: actual game54 BEFORE90/A0 stores.
        state_.target90.xyz=state_.followa0.xyz=std::array<float,3>{{-10000,-10000,-10000}};
    }
    ~DogAIController()override{ //Source fallback, distinct from normal0C then08 ordering.
        if(!state_.route78.empty())try{world_.dog_controller_process().release_route(state_.route78);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    State& dog_control_state()override{return state_;}
    std::int32_t dog_control_kind()const override{return state_.kind14;}
    Actor* dog_control_target()override{return state_.target88;} //005CB390.
    bool dog_control_bind(Actor& actor)override{ //005CE630→04/005CE5D0, then kind-specific00 tail.
        state_.actor10=&actor;
        const auto direction=actor.base().world_direction();state_.direction40.assign(direction);state_.saved50.assign(direction);
        state_.move1c=nullptr;state_.flags20=0;state_.follow18=nullptr;state_.first60=state_.second64=nullptr;
        if(state_.kind14==4)return true; //005CF0A0.
        state_.timerc4=-1; //005D5B80 AFTER common04.
        const auto* data=frames_.find_actor_frame_data(state_.actor10->base().frame());
        state_.scriptc0=data?reinterpret_cast<void*>(data->words[1]):nullptr; //005CF6C0, actual modelEC second word.
        return state_.scriptc0!=nullptr;
    }
    void dog_control_restore(DogControllerLifetime<Frames>& old)override{ //005CE650, no route/input/target88 copying.
        auto& previous=old.dog_control_state();state_.flags20=0;
        state_.movement30=previous.movement30;state_.direction40=previous.direction40;state_.saved50=previous.saved50;
        dog_control_follow(previous.follow18);
        state_.first60=state_.second64=nullptr;
    }
    void dog_control_follow(Actor* follow)override{ //005CD720, including005CD690/005CB500 positive continuation.
        state_.follow18=follow;
        if(!follow){state_.flags20&=~0x10u;state_.followa0.xyz=std::array<float,3>{{-10000,-10000,-10000}};state_.followa0.w.reset();return;}
        state_.flags20|=0x10u;
        state_.followa0=follow_position();
        if(!state_.actor10)throw std::runtime_error("Dog follow requires actual controller Actor10");
        const auto origin=state_.actor10->base().world_position();DogControllerVector from;from.assign(origin);
        (void)request_route(from,state_.followa0);
    }
    void dog_control_close()override{state_.actor10=nullptr;world_.dog_controller_process().release_route(state_.route78);} //005D5B20/5BB0.
    void dog_control_script(void* context)override{if(state_.kind14==5)state_.scriptc0=context;} //Kind4 CB2E0 genuine RET;kind5 D5BC0 store.
    void dog_control_dead()override{state_.flags20|=0x800u;} //005CE6E0.
    void dog_control_damage(std::uint32_t,Actor* source)override{
        if(state_.kind14==4){ //005CDF30.
            if(state_.flags20&0x80000u){state_.target88=nullptr;state_.flags20&=~0x80000u;state_.timerb4=200-dog_signed_word(world_.dog_control_random())%1500;}
            if(!(state_.flags20&0x200u))return;
            if(!source)throw std::runtime_error("Dog kind4 damage reaches native NULL source with flag200");
        }else if(!(state_.flags20&0x200u)||!source)return; //005CFB40 has native NULL guard.
        if(source->base().type()==3){state_.damage_source8c=source;state_.flags20|=0x400u;}
    }
    long double dog_control_speed()override{ //005CDE60: distance to target88, NOT a velocity getter.
        if(!state_.target88)return std::numeric_limits<float>::max(); //0080F7B0=7F7FFFFF.
        if(!state_.actor10)throw std::runtime_error("Dog distance requires actual controller Actor10");
        const auto own_model=state_.actor10->base().frame();(void)frames_.actor_world_matrix(own_model);
        auto* target=state_.target88;if(!target)throw std::runtime_error("Dog distance lost actual target after own-model callback");
        const auto other=frames_.actor_world_matrix(target->base().frame());
        const auto own=frames_.dog_cached_world_matrix(own_model); //Native reads BOTH cached models AFTER target update.
        const auto x=static_cast<long double>(other[12])-own[12],y=static_cast<long double>(other[13])-own[13],z=static_cast<long double>(other[14])-own[14];
        const float square=static_cast<float>((z*z+y*y)+x*x);std::uint32_t bits;std::memcpy(&bits,&square,4);
        return dog_signed_word(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
    }
    void dog_control_authority(std::uint32_t elapsed)override{DecisionQueries q{*this};advance_dog_selected_controller(state_,elapsed,q);}
    void dog_control_presentation(std::uint32_t)override{} //005CF0C0/005CFA10→005CE180, genuine RET4.
    void dog_control_sight_acquired(Actor*)override{ //005CF0F0 genuine RET4;005CFBA0→kind5 virtual38(20).
        if(state_.kind14==5)move(reinterpret_cast<void*>(std::uintptr_t{0x20}));
    }
    void dog_control_sight_lost(Actor*)override{} //005CB3B0 genuine RET4.
    void dog_control_loud_sound()override{} //005CDDC0 genuine RET.
    void dog_control_heard(DogHeardRecord& record)override{ //005CF0E0 genuine RET4;005CFB70 selected kind5.
        if(state_.kind14==4)return;
        if(!record.strength1c)throw std::runtime_error("Dog heard callback requires actual record1C producer");
        if(*record.strength1c<=10)return;
        if(!record.kind10)throw std::runtime_error("Dog heard callback requires actual record10 producer");
        move(reinterpret_cast<void*>(std::uintptr_t{*record.kind10?1u:4u}));
    }
    void dog_control_publish_inputs()override{publish_dog_controller_inputs(state_);}
    bool consume_flag(){const bool result=state_.flagb8;state_.flagb8=false;return result;} //005CB380.
    void move(void* movement){ //005CE700/005CF8E0: native bitwise complement test is tautologically nonzero.
        state_.move1c=movement;if(movement)state_.flags20|=0x80000000u;else state_.flags20&=~0x80000000u;
        if(movement&&state_.kind14==5)world_.dog_script_alarm(state_.scriptc0); //AFTER1C/20 stores; native has no context-NULL guard.
    }
    void advance_timers(std::uint32_t elapsed){ //005CDF90.
        for(auto* timer:{&state_.timerb0,&state_.timerbc,&state_.timer84})if(*timer>0)*timer=dog_signed_word(static_cast<std::uint32_t>(*timer)-elapsed);
        if(state_.timer84<0)state_.timer84=0;
    }
    void advance_follow(){
        struct Queries {
            DogAIController& owner;
            auto follow_position(){return owner.follow_position().full();}
            auto actor_position(){if(!owner.state_.actor10)throw std::runtime_error("Dog follow frame requires actual Actor10");return owner.state_.actor10->base().world_position();}
            auto actor_direction(){if(!owner.state_.actor10)throw std::runtime_error("Dog follow frame requires actual Actor10 direction");return owner.state_.actor10->base().world_direction();}
            auto follow_direction(){if(!owner.state_.follow18)throw std::runtime_error("Dog follow frame lost actual follow18 direction");return owner.state_.follow18->base().world_direction();}
            auto follow_actor_position(){if(!owner.state_.follow18)throw std::runtime_error("Dog follow frame lost actual follow18 position");return owner.state_.follow18->base().world_position();}
            void set_speed(float value){if(!owner.state_.actor10)throw std::runtime_error("Dog follow speed requires actual Actor10");owner.world_.dog_set_body_speed(*owner.state_.actor10,value);}
            void clear_route(){owner.clear_route();}
        } queries{*this};
        advance_dog_controller_follow(state_,queries);
    }
    void select_target(bool force){
        struct Queries {
            DogAIController& owner;
            auto& dog(){if(!owner.state_.actor10)throw std::runtime_error("Dog target scan requires actual Actor10");return owner.world_.dog_controller_actor(*owner.state_.actor10);}
            auto health(Actor& actor){return owner.world_.dog_sighted_human_health(actor);}
            bool attached(Actor& actor){return owner.world_.dog_target_attached(actor);}
            Actor* source_actor(void* source){return static_cast<Actor*>(source);}
            void target_event(){owner.world_.dog_target_selected_event(owner.state_);}
        } queries{*this};
        select_dog_controller_target(state_,force,queries);
    }
    void clear_target(){
        struct Queries {DogAIController& owner;void target_finished(void* context){owner.world_.dog_script_route_finished(context);}} queries{*this};
        clear_dog_controller_target(state_,queries);
    }
    void publish_actions(){
        struct Queries {
            DogAIController& owner;
            auto& dog(){if(!owner.state_.actor10)throw std::runtime_error("Dog actions require actual Actor10");return owner.world_.dog_controller_actor(*owner.state_.actor10);}
            void action(std::uint32_t value){owner.world_.dog_controller_action(dog(),value);}
            void posture(std::uint32_t value){dog().body_actions.request_posture(value,0);}
            auto next_stance(){return dog().body_orders.next_stance();}
            void stance(std::uint32_t value){dog().body_actions.request_stance(value,0);}
            void turn_body(){turn_dog_controller_body(dog(),owner.world_.dog_controller_process().turn_count4a8,owner.frames_);}
        } queries{*this};
        publish_dog_controller_actions(state_,queries);
    }
    void publish_body_direction(){
        if(!state_.actor10)throw std::runtime_error("Dog direction publication requires actual Actor10");
        auto& dog=world_.dog_controller_actor(*state_.actor10);request_dog_body_direction(dog.body,dog.body_orders,state_.direction40);
    }
    void retire_heard(){
        struct Queries {
            DogAIController& owner;
            auto& dog(){if(!owner.state_.actor10)throw std::runtime_error("Dog heard retirement requires actual Actor10");return owner.world_.dog_controller_actor(*owner.state_.actor10);}
            auto microseconds(){return owner.world_.dog_sound_microseconds();}
            void release(DogHeardRecord* record){owner.world_.dog_controller_process().heard.release(record);}
        } queries{*this};
        retire_dog_controller_heard(queries);
    }
    void advance_route(){DecisionQueries q{*this};advance_dog_controller_route(state_,q);}
    void advance_decisions(std::uint32_t elapsed){DecisionQueries q{*this};advance_dog_controller_decisions(state_,elapsed,q);}
    bool target_route(){DecisionQueries q{*this};return refresh_dog_target_route(state_,q);}
    bool follow_route(){DecisionQueries q{*this};return refresh_dog_follow_route(state_,q);}
    void retarget(){DecisionQueries q{*this};retarget_dog_controller(state_,q);}
    void advance_pursuit(std::uint32_t elapsed){DecisionQueries q{*this};advance_dog_controller_pursuit(state_,elapsed,q);}
    bool complete_route(){DecisionQueries q{*this};return complete_dog_controller_route(state_,q);}
    bool validate_route(){ //005CA5A0 on SAME route/Car/body frames and process pool.
        if(!state_.route78.empty()&&(dog_route_required(dog_route_point(state_.route78[0]).flags24)&0x1000u))return true;
        if(state_.route78.size()>1&&(dog_route_required(dog_route_point(state_.route78[1]).flags24)&0x1000u))return true;
        std::size_t first=0,last=0;if(!dog_route_marked_span(state_.route78,first,last))return true;
        auto* car=world_.dog_route_marking_car(state_.route78,first,last);if(!car)return true;
        const auto bounds=world_.dog_route_car_rectangle(*car);
        const auto matrix=world_.dog_route_car_matrix(*car);world_.dog_controller_process().obstacle_matrix5d0=matrix;
        const auto inverse=frames_.car_inverse_matrix(matrix);
        struct Queries {
            DogAIController& owner;Actor& car;const typename Frames::Matrix& matrix;const typename Frames::Matrix& inverse;
            auto model_position(){return DecisionQueries{owner}.model_position();}
            auto obstacle_position(){return car.base().world_position();}
            auto to_local(const std::array<float,4>& point){return owner.frames_.car_transform(point,inverse);}
            auto to_world(const std::array<float,4>& point){return owner.frames_.car_transform(point,matrix);}
            auto& acquire(const DogControllerVector& point){return owner.world_.dog_controller_process().acquire_point(0xffffffffu,point,0,0,0);}
            void release(DogRoutePoint* point){owner.world_.dog_controller_process().route.release(point);}
        } queries{*this,*car,matrix,inverse};
        return validate_dog_route_rectangle(state_.route78,bounds,first,last,queries);
    }
    DogControllerVector follow_position(){ //005CD690. NULL genuinely leaves the caller output untouched.
        if(!state_.follow18)throw std::runtime_error("Dog follow position requires actual nonNULL follow18");
        auto direction=state_.follow18->base().world_direction();
        frames_.car_transform_in_place(direction,frames_.human_axis_rotation({{0,1,0,0}},1.5707963705062866f)); //00788490.
        if(!state_.follow18)throw std::runtime_error("Dog follow position lost actual follow after rotation");
        auto result=state_.follow18->base().world_position();
        for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(static_cast<long double>(direction[axis])+result[axis]);
        DogControllerVector value;value.assign(result);return value;
    }
    void clear_route(){ //005CBE60: pool retirement BEFORE flags and possible script callback.
        world_.dog_controller_process().release_route(state_.route78);
        const auto flags=state_.flags20;state_.flags20=flags&0xdffffff3u;
        if((flags&0x200000u)&&state_.kind14==5&&state_.scriptc0&&(flags&0x40000u)){
            world_.dog_script_route_finished(state_.scriptc0);state_.flags20&=~0x200000u;
        }
    }
    bool request_route(const DogControllerVector& origin,const DogControllerVector& destination){ //005CB500.
        if(!state_.navigation70)return false;
        auto from=origin;
        if((state_.flags20&0xcu)&&(state_.flags20&0x4000u)){
            if(!state_.actor10)throw std::runtime_error("Dog route origin requires actual controller Actor10");
            const auto direction=state_.actor10->base().world_direction();auto xyz=from.position();
            const float z=static_cast<float>(static_cast<long double>(direction[2])*.20000000298023224f);
            xyz[0]=static_cast<float>(static_cast<long double>(direction[0])*.20000000298023224f+xyz[0]);xyz[1]=static_cast<float>(static_cast<long double>(direction[1])*.20000000298023224f+xyz[1]);xyz[2]=static_cast<float>(static_cast<long double>(z)+xyz[2]);from.xyz=xyz;
        }
        auto& navigation=world_.dog_navigation();
        const auto start=navigation.nearest_volume(from.position(),5.f,-1.f);
        const auto end=navigation.nearest_volume(destination.position(),5.f,-1.f);
        return build_route(start,end,from,destination);
    }
    bool build_route(std::uint16_t start,std::uint16_t end,const DogControllerVector& origin,const DogControllerVector& destination){ //005CB610.
        if(!state_.navigation70)return false;
        auto& navigation=world_.dog_navigation();std::vector<std::uint16_t> route;
        if(!navigation.find_dog_route(start,end,route,[&](std::uint16_t from){return world_.dog_navigation_door_phase(from);}))return false;
        clear_route();state_.flags20=(state_.flags20&~8u)|4u;
        for(const auto index:route){
            const auto* point=navigation.record(index);if(!point)throw std::runtime_error("Dog path lost actual navigation record");
            DogControllerVector position;position.xyz=navigation.position(index);
            auto& output=world_.dog_controller_process().acquire_point(index,position,static_cast<std::uint16_t>(point->flags&0xfb40u),point->byte16,point->prefix[0x13]);
            state_.route78.push_back(&output);
        }
        smooth_dog_route(state_.route78,origin.position(),destination.position());
        if(!state_.route78.empty())state_.route78.back()->position0=destination;
        if(!state_.actor10)throw std::runtime_error("Dog path movement limit requires actual controller Actor10");
        if(world_.dog_body_movement_limit(*state_.actor10)!=0){
            if(state_.route78.empty()||!state_.route78[0])throw std::runtime_error("Dog path trimming reaches native missing first point");
            const auto first=state_.route78.size()==1?origin.position():state_.route78[0]->position0.position();
            if(state_.route78.size()>1&&!state_.route78[1])throw std::runtime_error("Dog path trimming reaches native NULL second point");
            const auto second=state_.route78[state_.route78.size()==1?0u:1u]->position0.position();
            std::array<float,3> a{},b{};for(unsigned axis=0;axis!=3;++axis){a[axis]=static_cast<float>(static_cast<long double>(first[axis])-origin.position()[axis]);b[axis]=static_cast<float>(static_cast<long double>(second[axis])-origin.position()[axis]);}
            const auto model=frames_.actor_world_matrix(state_.actor10->base().frame());
            auto* point=state_.route78[0];const auto current=point->position0.position();const auto radius=dog_route_required(point->radius28);
            const auto x=static_cast<long double>(current[0])-model[12],z=static_cast<long double>(current[2])-model[14];
            const auto dot=(static_cast<long double>(b[2])*a[2]+static_cast<long double>(b[1])*a[1])+static_cast<long double>(b[0])*a[0];
            if(x*x+z*z<=static_cast<long double>(radius)*radius||dot<0){world_.dog_controller_process().route.release(point);state_.route78.erase(state_.route78.begin());}
        }
        return true;
    }
private:
    struct DecisionQueries {
        DogAIController& owner;
        auto& state(){return owner.state_;}
        auto& dog(){if(!state().actor10)throw std::runtime_error("Dog decisions require actual Actor10");return owner.world_.dog_controller_actor(*state().actor10);}
        Actor& target(){if(!state().target88)throw std::runtime_error("Dog decisions require actual target88");return *state().target88;}
        auto actor_position(){if(!state().actor10)throw std::runtime_error("Dog decision position requires actual Actor10");return state().actor10->base().world_position();}
        auto model_position(){if(!state().actor10)throw std::runtime_error("Dog decision model requires actual Actor10");const auto matrix=owner.frames_.actor_world_matrix(state().actor10->base().frame());return std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}};}
        auto follow_position(){return owner.follow_position().full();}
        auto actor_direction(){if(!state().actor10)throw std::runtime_error("Dog route angle requires actual Actor10");return state().actor10->base().world_direction();}
        auto vector_angle(const std::array<float,4>& first,const std::array<float,4>& second){return owner.frames_.car_vector_angle(first,second);}
        auto history_velocity(){return dog().frame_history.velocity580;}
        auto history_position(){const auto& value=dog().frame_history.position590;if(!value)throw std::runtime_error("Dog route requires actual history590 producer");return *value;}
        auto history_elapsed(){const auto& value=dog().frame_history.elapsed5a0;if(!value)throw std::runtime_error("Dog route requires actual history5A0 producer");return *value;}
        void release(DogRoutePoint* point){owner.world_.dog_controller_process().route.release(point);}
        bool complete_route(){return owner.complete_route();}
        auto random_unit(){return owner.world_.human_sound_random();}
        void decisions(std::uint32_t elapsed){owner.advance_decisions(elapsed);}
        void timed_voice(){if(!state().actor10)throw std::runtime_error("Dog kind5 timed voice requires actual Actor10");owner.world_.dog_controller_timed_voice(*state().actor10);}
        void timers(std::uint32_t elapsed){owner.advance_timers(elapsed);}
        void bind_zeroed_inputs(){auto& process=owner.world_.dog_controller_process();process.first710.fill(0);process.second610.fill(0);state().first60=&process.first710;state().second64=&process.second610;}
        void retire_heard(){owner.retire_heard();}
        void pursuit(std::uint32_t elapsed){owner.advance_pursuit(elapsed);}
        void follow(){owner.advance_follow();}
        void route(){owner.advance_route();}
        void publish_inputs(){owner.dog_control_publish_inputs();}
        void publish_actions(){owner.publish_actions();}
        void publish_direction(){owner.publish_body_direction();}
        auto target_distance(){return owner.dog_control_speed();}
        auto movement_limit(){if(!state().actor10)throw std::runtime_error("Dog decision movement limit requires actual Actor10");return owner.world_.dog_body_movement_limit(*state().actor10);}
        auto target_health(){return owner.world_.dog_sighted_human_health(target());}
        bool target_attached(){return owner.world_.dog_target_attached(target());}
        auto target_velocity(){return owner.world_.dog_target_velocity(target());}
        auto current_category(){return dog_action_category(dog_signed_word(dog().body_actions.current364));}
        auto next_category(){return dog_action_category(dog_signed_word(dog().body_orders.next_action()));}
        bool perception_nonempty(){return !dog().spotted574.empty()||!dog().heard564.empty();}
        void set_speed(float value){dog().body_actions.set_speed(value);}
        void cancel_bite(){dog().body_actions.cancel_bite();}
        void clear_target(){owner.clear_target();}
        void clear_route(){owner.clear_route();}
        void select_target(bool force){owner.select_target(force);}
        void retarget(){owner.retarget();}
        bool target_route(){return owner.target_route();}
        bool follow_route(){return owner.follow_route();}
        bool validate_route(){return owner.validate_route();}
        bool request_route(const std::array<float,4>& from,const std::array<float,4>& to){DogControllerVector origin,destination;origin.assign(from);destination.assign(to);return owner.request_route(origin,destination);}
        auto random_word(){return owner.world_.dog_control_random();}
        void route_finished(void* context){owner.world_.dog_script_route_finished(context);}
    };
    State state_;Frames& frames_;World& world_;
};
//005D5890 selected kind4/5. Construction keeps old534 published;00 is ALWAYS
//called after new publication; copy rereads actual new534, then old0C/delete08.
template<class Controller,class Dog,class Factory>void replace_dog_controller(Dog& dog,std::int32_t kind,Factory&& factory){
    if(kind!=4&&kind!=5)throw std::runtime_error("Dog controller replacement requires selected kind1/7 source");
    std::unique_ptr<Controller> next=factory(kind);
    auto old=std::move(dog.field534);dog.field534=std::move(next);
    if(!dog.field534)throw std::runtime_error("Dog controller replacement reaches native NULL allocated534");
    (void)dog.field534->dog_control_bind(dog);
    if(old){dog.require_controller().dog_control_restore(*old);old->dog_control_close();old.reset();}
}
}
#endif
