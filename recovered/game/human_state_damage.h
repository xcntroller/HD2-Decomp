#ifndef HD2_GAME_HUMAN_STATE_DAMAGE_H
#define HD2_GAME_HUMAN_STATE_DAMAGE_H
#include "game/human_damage.h"

namespace hd2::game {
//00545DD0 ignores its three stack arguments. It perturbs B0+10, with three
//independent engine draws, then publishes through virtual54=00583120. The
//last Z result remains extended for one multiply after its FLOAT store.
template<class State,class World,class Frames>
void perturb_player_damage_direction(State& state,World& world,Frames& frames){
    auto* human=state.human();if(!human)throw std::logic_error("Player hit direction requires actual Human");
    auto direction=state.words().template get<HumanVector>(0x10);
    const bool easy=(world.mode_flags()&7u)==7u&&world.human_difficulty()==1;
    const auto conditioning=static_cast<long double>(human_frame_signed_word(human->properties.stats_ec[2]))*.009999999776482582f; //005A6530(2).
    const auto remainder=1.L-conditioning;
    const float horizontal=easy?.009999999776482582f:static_cast<float>(remainder*.09000000357627869f+.009999999776482582f);
    const auto jitter=[&](unsigned axis){
        const auto draw=static_cast<long double>(frames.volume_random_unit())-.5f;
        long double offset;
        if(axis==1)offset=easy?draw*.019999999552965164f:(static_cast<float>(remainder)*.18000000715255737f+.019999999552965164f)*draw;
        else offset=draw*horizontal;
        return static_cast<long double>(direction[axis])+(easy?offset:offset+offset);
    };
    direction[0]=static_cast<float>(jitter(0));direction[1]=static_cast<float>(jitter(1));
    const auto z=jitter(2);direction[2]=static_cast<float>(z);
    const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+z*direction[2];
    constexpr float epsilon=9.99999993922529e-09f;
    if(epsilon<=std::abs(square-1.L)){
        const float stored=static_cast<float>(square);
        if(stored>=epsilon){const float scale=static_cast<float>(1.L/std::sqrt(static_cast<long double>(stored)));
            for(unsigned axis=0;axis!=3;++axis)direction[axis]*=scale;
        }else{const unsigned axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
    }
    state.set_target_direction(direction);
}
//005836E0, shared virtual84 before the Player-specific tail. Death cancellation
//uses the same B0 action receiver with success=false; ownership remains live.
template<class State,class World,class Frames>
void retire_human_dead_state(State& state,World& world,Frames& frames){
    auto* human=state.human();if(!human)throw std::logic_error("Human death requires bound B0");
    bool seated=false;
    if(human->field160){const auto type=world.human_pose_actor_type(human->field160);seated=type==4||type==20||type==21||type==34;}
    if(!seated){
        if((world.mode_flags()&7u)==7u){
            auto* storage=human->base().storage();auto* hand=storage?storage->holder(3):nullptr;
            if(!hand)throw std::logic_error("Human death drop requires actual hand holder");
            if(const auto entry=hand->entry();entry&&world.human_frame_output_item(entry).definition().kind()!=9){
                auto velocity=state.words().template get<HumanVector>(0x10);velocity[1]+=1.f;
                const auto square=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[2])*velocity[2])+static_cast<long double>(velocity[1])*velocity[1];
                normalize_human_ai_shot(velocity,square);
                const auto speed=static_cast<long double>(frames.volume_random_unit())+2.f;
                for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(velocity[axis]*speed);
                world.drop_human_damage_item(*human,entry,velocity);
            }
        }else world.drop_human_dead_inventory(*human); //005A2FE0, distinct from the offline hand-only operation.
    }
    auto& words=state.words();words.set(0x50,words.template get<std::uint32_t>(0x50)|0x800u);
    state.retire_special_music_reference();
    if(words.template get<std::int32_t>(0x64)>=0){
        world.abort_human_dead_action(state);
        state.action_target(nullptr);
        for(unsigned offset:{0x60u,0x64u,0x6cu})words.set(offset,std::int32_t{-1});
    }
}
//0053DB90/00574560 shared B0+BC work. Uses existing inventory, frame-command and AI-event
//queues. Positive drops dispatch through the existing Items and runtime actor owners.
template<class State,class Human,class World,class Frames>
void notify_human_ai_damage(State& state,std::uint32_t kind,std::uint32_t hit_kind,std::int32_t health,
        Human* source,const HumanVector& point,const HumanVector& direction,World& world,Frames& frames){
    auto* human=state.human();if(!human)throw std::logic_error("AI damage notification requires bound Human");
    const auto state_kind=state.words().template get<std::uint32_t>(0x48);const bool player=state_kind==1;
    if(player||(world.mode_flags()&7u)!=7u)state.queue_damage_command(kind,hit_kind,health,source,point,direction);
    bool protected_seat=false;
    if(human->field160)protected_seat=(world.human_reaction_seat_flags(human->field160,human->field164)&0x10u)||
        world.human_pose_actor_has_control(human->field160,static_cast<std::uint32_t>(human->field164));
    const auto drop_velocity=[&]{
        auto value=direction;
        const auto square=(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[1])*value[1];
        normalize_human_ai_shot(value,square);
        const float speed=player?2.f:frames.volume_random_unit()+2.f;
        for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(static_cast<long double>(speed)*value[axis]+(axis==1?1.f:0.f));
        return value;
    };
    if(player||state_kind==4||state_kind==5||state_kind==6||state_kind==8){
        if(hit_kind==11){
            auto* storage=human->base().storage();auto* holder=storage?storage->holder(0):nullptr;
            if(!holder)throw std::logic_error("AI damage drop requires actual holder0");
            if(const auto entry=holder->entry())world.drop_human_damage_item(*human,entry,drop_velocity());
        }
        if(!protected_seat&&state_kind!=5&&hit_kind==3&&world.human_frame_random_word()%20==0){
            state.refresh_item();
            if(const auto entry=state.item()){
                const auto payload=world.human_frame_output_item(entry).primary_payload_kind();
                if(payload==1||payload==3||payload==4){world.drop_human_damage_item(*human,entry,drop_velocity());state.clear_cached_item();}
            }
        }
    }
    if(player){world.human_player_damage_feedback(state,kind,source);return;}
    if(!source)return;
    const bool dead=human_frame_signed_word(human->properties.stats_ec[0])<1;
    if(!dead&&kind!=5)return;
    auto* command=state.command();if(!command)throw std::logic_error("AI damage event requires actual command owner");
    const auto source_world=frames.actor_world_matrix(source->base().frame());
    auto& queue=dead?command->second.events:command->first.events;
    auto event=queue.pool().acquire();event->words[0]=dead?0x10000016u:0x40000015u;
    event->words[1]=10000;event->words[2]=0;event->words[12]=0;
    event->words[3]=dead?static_cast<std::uint32_t>(source->base().id10()):0xffffffffu;
    event->words[4]=static_cast<std::uint32_t>(source->base().id10());
    const HumanVector source_point{{source_world[12],source_world[13],source_world[14],source_world[15]}};
    std::memcpy(event->words.data()+8,source_point.data(),16);
    if(dead)queue.merge(std::move(event),50000,false);else queue.push(std::move(event));
}
} //namespace hd2::game
#endif
