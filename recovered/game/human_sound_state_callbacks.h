#ifndef HD2_GAME_HUMAN_SOUND_STATE_CALLBACKS_H
#define HD2_GAME_HUMAN_SOUND_STATE_CALLBACKS_H

#include "game/human_loading_state_factory.h"
#include "game/human_perception_records.h"
#include "game/human_sound_look_reaction.h"
#include <optional>

namespace hd2::game {

//Actual A8/AC/B0/B4 derived dispatch, using the containing SAME state factory.
//Services supplies actual HUD004947E0;
//Contexts supplies actual interrupt/shared CRT rand. No empty callbacks.
template<class Frames,class Human,class Factory>class HumanSoundStateCallbacks {
public:
    using Base=typename Factory::Base;using AI=typename Factory::AI;using Script=typename Factory::Script;
    using Actor=typename std::decay_t<decltype(std::declval<Human&>().perception.lists()[0])>::value_type;
    using Record=HumanPerceptionRecord<Actor>;
    HumanSoundStateCallbacks(Frames& frames,Factory& factory):frames_(frames),factory_(factory){}
    void a8(Base& state,Actor actor) { //00574280; script00590250.
        const auto kind=state_kind(state);if(kind==1||kind==7)return; //005831B0 genuine RET4.
        queue(state,0x20000012u,actor,nullptr,nullptr); //Zero XYZ, leave W untouched.
        if(kind==5){auto& script=as<Script>(state);if(script_gate(script,0x80)){begin_interrupt(script,0x80);script.state().set(0x140,position(actor));interrupt(script);}}
    }
    void ac(Base& state,Actor actor) { //00574310; script00590170.
        const auto kind=state_kind(state);if(kind==1||kind==7)return; //005831C0 genuine RET4.
        if(actor) {
            const auto threat=actor_a4(actor);
            if(threat){const auto point=position(threat);queue(state,0x20000013u,actor,threat,&point);}
            else queue(state,0x20000013u,actor,nullptr,nullptr);
        }
        if(kind!=5)return;
        auto& script=as<Script>(state);if(!script_gate(script,0x100))return;
        auto& other=human(actor);auto selected=actor;const auto& enemies=other.perception.lists()[0];
        if(!enemies.empty()) {
            const std::int64_t random=factory_.contexts().human_script_random();
            if(random<0||random>0x7fff)throw std::runtime_error("Human script sound requires shared CRT rand");
            selected=enemies[static_cast<std::uint32_t>(random)%enemies.size()];
        }
        script.state().set(0x140,position(selected)); //Native AC copies position BEFORE flags/interrupt mask.
        begin_interrupt(script,0x100);interrupt(script);
    }
    void b0(Base& state,Actor actor) { //00574480; active0052E750; script005900E0.
        const auto kind=state_kind(state);if(kind==1||kind==6||kind==7)return; //Genuine5831D0/5A4380.
        if(kind==5&&actor&&friendly(human(actor),state.human()))return; //REVERSE friendship, before common producer.
        if(actor&&!friendly(bound_human(state),actor)) {
            const auto point=position(actor);queue(state,0x40000011u,nullptr,actor,&point);
        }
        if(kind==3)factory_.services().queue_active_ai_speech(as<typename Factory::Active>(state),0x6eu,1,5000,0);
        if(kind==5){auto& script=as<Script>(state);if(script_gate(script,1)){begin_interrupt(script,1);script.state().set(0x140,position(actor));interrupt(script);}}
    }
    void b4(Base& state,Record& record) { //00574D70; script005902D0; player005449B0.
        const auto kind=state_kind(state);if(kind==6||kind==7)return; //005A4390/005831E0, genuine RET4.
        if(kind==1){player_record(as<typename Factory::Player>(state),record);return;}
        std::uint32_t event=0;
        switch(record.kind) {
        case 0:event=0x3000000bu;break;
        case 1:case 3:event=0x3000000du;break;
        case 2:event=0x3000000cu;break;
        case 6:event=0x3000000eu;break;
        case 8:case 9:
            if(record.actor&&!friendly(bound_human(state),record.actor))event=0x3000000fu;
            break;
        default:break;
        }
        if(event)queue(state,event,nullptr,record.actor,&record.position); //005750F0 initializes this SAME acquired record.
        if(kind!=3&&kind!=4&&kind!=8&&(record.kind==0||record.kind==2))
            human_sound_look_reaction(as<AI>(state),record,record.kind==0,frames_,factory_.control());
        if(kind!=5)return;
        auto& script=as<Script>(state);const auto mask=record.kind==0?4u:record.kind==1?8u:record.kind==2?2u:record.kind==6?0x10u:0u;
        if(mask&&script_gate(script,mask)){begin_interrupt(script,mask);script.state().set(0x140,record.position);interrupt(script);}
    }
private:
    template<class T>static T& as(Base& state){auto* result=dynamic_cast<T*>(&state);if(!result)throw std::logic_error("Human sound state kind/type disagree");return *result;}
    static std::uint32_t state_kind(Base& state){return state.words().template get<std::uint32_t>(0x48);}
    static Human& human(Actor actor){auto* result=dynamic_cast<Human*>(actor);if(!result)throw std::runtime_error("Human sound state requires its actual Human source");return *result;}
    static Human& bound_human(Base& state){auto* result=state.human();if(!result)throw std::logic_error("Human sound state has no bound Human");return *result;}
    bool friendly(Human& observer,Actor actor){return factory_.services().human_sound_friendly(observer,actor);}
    HumanVector position(Actor actor){if(!actor)throw std::runtime_error("Human sound position requires an actual source");const auto m=frames_.actor_world_matrix(actor->base().frame());return {{m[12],m[13],m[14],m[15]}};}
    Actor actor_a4(Actor actor) { //005831A0 or00524DD0. D4's actual runtime producer is not exposed yet.
        auto* view=human(actor).loading_state;
        auto* state=dynamic_cast<Base*>(view);if(!state)throw std::runtime_error("Human sound source has no actual shared B0");
        const auto kind=state_kind(*state);if(kind==1||kind==7)return nullptr;
        if(as<AI>(*state).resources().state().template get<std::uint32_t>(0xd4))
            throw std::runtime_error("Human sound A4 requires its actual typed threat producer");
        return nullptr; //Native D4 constructor0, no successful populated-threat fallback.
    }
    void queue(Base& state,std::uint32_t kind,Actor first,Actor second,const HumanVector* point) {
        auto* command=state.command();if(!command)throw std::logic_error("Human sound event has no actual command queue");
        auto& queue=command->first.events;auto event=queue.pool().acquire();event->words[0]=kind;event->words[1]=10000;event->words[2]=0;
        event->words[3]=first?static_cast<std::uint32_t>(first->base().id10()):0xffffffffu;
        event->words[4]=second?static_cast<std::uint32_t>(second->base().id10()):0xffffffffu;event->words[12]=0;
        if(point)std::memcpy(event->words.data()+8,point->data(),16);
        else event->words[8]=event->words[9]=event->words[10]=0; //Native leaves word11/W untouched.
        queue.push(std::move(event));
    }
    static bool script_gate(Script& script,std::uint32_t mask){return !(script.words().template get<std::uint32_t>(0x50)&0x80000000u)&&!(script.state().template get<std::uint32_t>(0x1b8)&mask);}
    static void begin_interrupt(Script& script,std::uint32_t mask){script.state().set(0x1b4,mask);script.words().set(0x50,script.words().template get<std::uint32_t>(0x50)|0x80000000u);}
    void interrupt(Script& script){if(auto* context=script.context())factory_.contexts().interrupt_human_script_context(*context);}
    void player_record(typename Factory::Player& player,const Record& record) {
        if(record.kind!=0&&record.kind!=6&&record.kind!=2)return;
        const auto m=frames_.actor_world_matrix(bound_human(player).base().frame());
        HumanVector direction{{static_cast<float>(static_cast<long double>(record.position[0])-m[12]),0,
            static_cast<float>(static_cast<long double>(record.position[2])-m[14]),0}}; //W is an unclaimed stack guard.
        const auto x=static_cast<long double>(direction[0]),z=static_cast<long double>(direction[2]);
        const auto square=x*x+z*z;const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-09f;
        if(!(std::abs(square-1)<epsilon)) {
            if(!(stored<epsilon)){const auto scale=1/std::sqrt(static_cast<long double>(stored));for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(direction[n]*scale);}
            else {const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
        }
        if(factory_.services().mode_flags()&2u)factory_.services().hud_sound_direction(direction,record.kind); //0049DE80→004947E0.
    }
    Frames& frames_;Factory& factory_;
};
} //namespace hd2::game
#endif
