#ifndef HD2_GAME_DOG_BODY_ACTIONS_H
#define HD2_GAME_DOG_BODY_ACTIONS_H
#include "game/dog_body_orders.h"
#include "game/dog_damage.h"
#include <limits>
namespace hd2::game {
//The action portion of the SAME Dog Body. Offsets here use Body=Actor+90.
//Cold construction does not write278/27C/280. Keep their provenance explicit;
//005C5030's actual common tail produces zero, not an invented constructor.
class DogBodyActions {
public:
    explicit DogBodyActions(DogBodyOrders& orders):orders_(orders){}
    std::optional<std::int32_t> priority278,priority27c,priority280;
    std::optional<std::uint32_t> requested26c,posture270,stance274;
    std::uint32_t current364=23,next368=23;
    std::int32_t timer370=0,death374=0;
    std::int32_t voice37c=-1,voice380=-1,voice384=-1;
    std::uint32_t flags3e0=0,animation2_168=0;
    bool flag36c=false;
    float speed248=1;
    std::array<float,3> animation2_position170{}; //Undefined W is not source data.
    std::optional<std::int32_t> death_timer378;
    void set_speed(float value){speed248=value;} //005C4B50, direct Body248 store.
    void initialize_scalars(){ //Known stores within005C7D30; priorities deliberately untouched.
        current364=next368=23;timer370=death374=0;voice37c=voice380=voice384=-1;
        flags3e0=animation2_168=0;flag36c=false;speed248=1;animation2_position170.fill(0);scalars_produced_=true;
    }
    void request_posture(std::uint32_t value,std::int32_t priority){ //005C8540.
        if(require(priority27c,"Dog posture requires actual Body27C priority producer")<=priority){priority27c=priority;posture270=value;}
    }
    void request_stance(std::uint32_t value,std::int32_t priority){ //005C8570.
        if(require(priority280,"Dog stance requires actual Body280 priority producer")<=priority){priority280=priority;stance274=value;}
    }
    template<class Receivers>void request_action(std::uint32_t value,std::int32_t priority,Receivers& receivers){ //005C8490.
        require_scalars();if(death374)return;
        const auto chosen=value==5?std::numeric_limits<std::int32_t>::max():priority;
        if(require(priority278,"Dog action requires actual Body278 priority producer")>chosen)return;
        priority278=chosen;
        if(value==5){
            orders_.discard_all();receivers.dog_ground_death_setup(); //005C88B0 before collision retirement.
            for(unsigned n=0;n!=2;++n)receivers.dog_remove_body_collision(n); //Remove/free/null each actual1DC/1E0 in order.
        }
        requested26c=value;
    }
    //005C5030. Queue metadata is read again AFTER append at the native sites;
    //do not replace these with a pre-enqueue snapshot. An unwritten new record
    //therefore remains an honest native-invalid-state failure.
    template<class Receivers>void compose(Receivers& receivers){
        require_scalars();auto previous=orders_.next_action(),action=orders_.next_action();
        auto stance=orders_.next_stance(),posture=orders_.next_posture();
        DogBodyOrderRecord* record=nullptr;
        const auto desired_stance=[&]{return require(stance274,"Dog compositor requires actual Body274 stance producer");};
        const auto desired_posture=[&]{return require(posture270,"Dog compositor requires actual Body270 posture producer");};
        const auto desired_action=[&]{return require(requested26c,"Dog compositor requires actual Body26C action producer");};
        const auto category=[&]{return dog_action_category(dog_order_signed(orders_.next_action()));};
        const auto append=[&](std::uint32_t value){if(!record)record=&orders_.enqueue();record->append(value);};
        if(orders_.next_stance()!=desired_stance()){
            record=&orders_.enqueue();
            if(orders_.next_stance()==0){if(category()!=3)append(9);append(21);append(7);action=7;}
            else{if(category()!=3)append(7);append(22);append(9);action=9;}
            stance=desired_stance();
        }
        if(orders_.next_posture()!=desired_posture()){
            const auto kind=category();
            if(static_cast<std::uint32_t>(kind)==desired_action()&&orders_.next_posture()!=2&&(category()==1||category()==0)){
                const auto next_category=category();
                if(desired_posture()==1){if(next_category==1){append(4);posture=desired_posture();action=4;}}
                else if(desired_posture()==0){action=next_category==1?3u:2u;append(action);posture=desired_posture();}
                else{action=next_category==1?1u:0u;append(action);posture=desired_posture();}
            }
        }
        const auto requested=desired_action();
        if(static_cast<std::uint32_t>(category())!=requested||(current364==23&&next368==23&&requested!=23)){
            if(!record)record=&orders_.enqueue();
            if(desired_stance()==1&&requested!=3&&requested!=6&&requested!=5){
                if(category()!=3)append(7);
                append(22);append(9);action=previous=9;request_stance(0,0);stance=0;
            }
            switch(requested){
            case 0:action=desired_posture()==2?0u:2u;append(action);posture=desired_posture();break;
            case 1:action=desired_posture()==2?1u:desired_posture()==1?4u:3u;append(action);posture=desired_posture();break;
            case 2:action=dog_order_signed(previous)<5?5u:6u;append(action);append(previous);break;
            case 3:action=desired_stance()==1?7u:9u;append(action);break;
            case 4:
                if(dog_order_signed(current364)<7||dog_order_signed(current364)>12){
                    orders_.discard_last();if(timer370>0)return;
                    if(voice380!=-1)receivers.dog_stop_body_voice(voice380);
                    voice380=-1;timer370=500;
                    voice380=receivers.dog_emit_body_voice(5u,4);return; //Native EARLY RETURN: priorities/metadata intentionally untouched.
                }
                action=18;append(action);break;
            case 5:{
                const bool special=orders_.next_stance()==7;
                const auto random=dog_order_signed(receivers.dog_action_random());
                //Native signed IDIV/bit-sign remainder, not unsigned modulo.
                append(static_cast<std::uint32_t>(special?random%2+13:random%3+15));
                action=13;(void)receivers.dog_emit_body_voice(1u,2);break;
            }
            case 6:action=orders_.next_stance()==1?19u:20u;append(action);append(previous);break;
            default:break; //Native switch has no default receiver.
            }
        }
        priority278=priority280=priority27c=0; //Tail runs even with no queued record.
        if(record)record->publish_metadata(action,posture,stance);
    }
    static bool action_blocks_transition(std::int32_t action){ //005C8880, signed comparison includes negative values.
        return !((action<13||action==18)&&action!=5&&action!=6);
    }
    //005C4CB0. The clip receiver is005C7430 at Body+10; it is required,
    //never replaced by a successful model-only animation shortcut.
    template<class Tables,class Frames,class Receivers>void advance_animation(Tables& tables,Frames& frames,Receivers& receivers){
        require_scalars();
        if(orders_.current.exhausted()){
            if(orders_.empty())return;
            orders_.take_first();
        }
        auto action=orders_.current.action();
        bool skip=action==23;
        if(!skip&&(action==5||action==6))skip=!receivers.dog_control_target();
        if(skip){
            orders_.current.consume();
            if(orders_.current.exhausted()){
                if(!orders_.empty())orders_.take_first();
                return; //Copied next record is deferred to the NEXT invocation.
            }
            action=orders_.current.action();
        }
        if(action==6&&receivers.dog_control_speed()>=1.7999999523162842f)action=5;
        const auto clip=tables.row(action).animation.get();
        auto blend=dog_order_signed(tables.row(action).metadata);
        auto time=receivers.dog_clip_time(0);const auto duration=receivers.dog_clip_duration(0);
        if(current364==5){time=dog_order_signed(static_cast<std::uint32_t>(time)+1200u);blend=400;}
        if(time!=-1&&dog_order_signed(static_cast<std::uint32_t>(time)+static_cast<std::uint32_t>(blend))<duration&&!flag36c&&action_blocks_transition(dog_order_signed(current364)))return;
        if(clip){
            frames.human_clip_word8(clip,action_blocks_transition(dog_order_signed(action))?1u:2u);
            receivers.dog_bind_body_clip(clip,tables.row(action).stride,1.f,static_cast<float>(blend),true); //Second five-word clip descriptor is zero.
            next368=current364;current364=action;
            flags3e0=tables.row(action).stride==0.f?flags3e0|1u:flags3e0&~1u;
            start_action_effects(action,receivers);
        }
        flag36c=false;orders_.current.consume();
        if(orders_.current.exhausted()&&!orders_.empty())orders_.take_first();
    }
    //005C5BF0. The actual body-relative SOUND value and marker are constructed
    //by the mandatory emission endpoint. Native37C/380/384 are distinct voices.
    template<class Receivers>void start_action_effects(std::uint32_t action,Receivers& receivers){
        require_scalars();const auto category=dog_action_category(dog_order_signed(action));
        const auto stop=[&](std::int32_t& voice){if(voice!=-1)receivers.dog_stop_body_voice(voice);};
        stop(voice37c);voice37c=-1;set_speed(1.f);
        if(action==4){voice37c=receivers.dog_emit_body_voice(13u,6);return;}
        switch(category){
        case 2:
            stop(voice384);stop(voice380);voice380=-1;animation2_168=0;voice384=-1;
            receivers.dog_clear_animation2_position(); //Actual164 frame static80 gate/XYZ dirty stores.
            animation2_position170.fill(0);voice384=receivers.dog_emit_body_voice(5u,5);return;
        case 4:
            if(timer370<1){stop(voice384);stop(voice380);voice380=voice384=-1;timer370=500;voice380=receivers.dog_emit_body_voice(5u,4);}
            return;
        case 5:stop(voice384);voice384=-1;(void)receivers.dog_emit_body_voice(1u,8);return;
        case 6:stop(voice384);stop(voice380);voice380=voice384=-1;(void)receivers.dog_emit_body_voice(1u,7);return;
        default:return;
        }
    }
private:
    template<class T>static T require(const std::optional<T>& value,const char* message){if(!value)throw std::runtime_error(message);return *value;}
    void require_scalars()const{if(!scalars_produced_)throw std::runtime_error("Dog actions require actual005C7D30 scalar producer");}
    DogBodyOrders& orders_;bool scalars_produced_=false;
};
//005C75C0 uses clip base=Body+10: these become Body110/120/124.
struct DogClipForcedMotion {
    std::optional<std::array<float,4>> direction100;
    std::optional<std::uint32_t> field110,timer114;
    void set(const std::array<float,4>& direction,std::uint32_t duration){direction100=direction;field110=0;timer114=duration;}
};
//005C88B0 reads Actor40 BEFORE either frame callback, updates Body404 even
//though that matrix is unused, then queries the real Body3EC bone position.
//Sweep owns the actual00718530 tree/buckets; no hit-success fallback exists.
template<class Actor,class Frame,class Frames,class Sweep>
void prepare_dog_ground_death(DogBodyActions& actions,DogClipForcedMotion& motion,
        Actor& actor,Frame body404,Frame body3ec,Frames& frames,Sweep& sweep){
    const auto direction=actor.base().world_direction();
    if(!body404)throw std::runtime_error("Dog ground death reaches native missing Body404 marker");
    (void)frames.actor_world_matrix(body404);
    if(!body3ec)throw std::runtime_error("Dog ground death reaches native missing Body3EC marker");
    const auto matrix=frames.actor_world_matrix(body3ec);
    const std::array<float,4> origin{{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])+.5f),matrix[14],0}};
    const auto hit=sweep.sweep(origin,{{0,-1,0,0}},.20000000298023224f,0xffffffffu,0x10000u,true);
    if(!hit){motion.set(direction,800);actions.death374=1;}else actions.death374=2;
    actions.death_timer378=5000;
}
}
#endif
