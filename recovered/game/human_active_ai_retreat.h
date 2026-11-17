#ifndef HD2_GAME_HUMAN_ACTIVE_AI_RETREAT_H
#define HD2_GAME_HUMAN_ACTIVE_AI_RETREAT_H
#include "game/human_ai_cover.h"
#include "game/human_active_ai_avoidance.h"
#include "game/human_active_ai_group.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//00525260: spatial cover candidates append ascending; filter preserves that
//order, then unsigned identity modulo chooses one. Engine yaw uses SIGNED id.
template<class Frames>HumanVector choose_active_retreat_position(MissionNavigation& navigation,Frames& frames,
        HumanVector proposed,HumanVector direction,float range,std::int32_t identity){
    const auto square=[](const HumanVector& v){return (static_cast<long double>(v[1])*v[1]+static_cast<long double>(v[2])*v[2])+static_cast<long double>(v[0])*v[0];};
    normalize_human_ai_shot(direction,square(direction));std::vector<std::uint16_t> nearby,accepted;
    if(navigation.append_spatial_cover({{proposed[0],proposed[1],proposed[2]}},range,nearby))for(auto index:nearby){
        const auto* point=navigation.record(index);if(!point)throw std::logic_error("Retreat cover lost actual navigation record");
        if(human_ai_cover::blocked(*point,human_ai_cover::direction(direction),3u))accepted.push_back(index);
    }
    if(accepted.empty())return proposed;
    const auto index=accepted[static_cast<std::uint32_t>(identity)%accepted.size()];const auto* point=navigation.record(index);
    const auto rotation=frames.human_y_rotation(static_cast<float>(identity));
    auto offset=frames.collision_sweep_rotate(direction,rotation);normalize_human_ai_shot(offset,square(offset));
    const float radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;const auto centre=navigation.position(index);
    for(unsigned n=0;n!=3;++n){const float scaled=static_cast<float>(static_cast<long double>(offset[n])*radius);proposed[n]=static_cast<float>(static_cast<long double>(scaled)+centre[n]);}
    //Native result W is unwritten in this branch; retain typed input W, no value claim.
    return proposed;
}
//00528950 task40, after its ordinary completion flags are published. True
//means task initialization reset206 and the caller must wait another frame.
template<class AI,class Frames,class World>bool advance_active_retreat_task(AI& ai,Frames& frames,World& world){
    auto& state=ai.state();if(state.template get<std::uint32_t>(0x220)!=40||!state.template get<std::uint32_t>(0x190))return false;
    auto* human=ai.human();if(!human)throw std::logic_error("Retreat task lost Human44");
    const auto matrix=frames.actor_world_matrix(human->base().frame());const HumanVector own{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto direction=state.template get<HumanVector>(0x150);HumanVector proposed=state.template get<HumanVector>(0x1e0);
    constexpr float third=.3333333432674408f;
    const float x_offset=static_cast<float>(static_cast<long double>(direction[0])*24.f*third),z_first=static_cast<float>(static_cast<long double>(direction[2])*24.f);
    proposed[0]=static_cast<float>(static_cast<long double>(own[0])+x_offset);
    proposed[1]=static_cast<float>(static_cast<long double>(direction[1])*24.f*third+own[1]);
    proposed[2]=static_cast<float>(static_cast<long double>(z_first)*third+own[2]);
    auto& navigation=world.active_task_navigation();proposed=choose_active_retreat_position(navigation,frames,proposed,direction,state.template get<float>(0x1c0),human->base().id10());
    const auto index=navigation.nearest_volume({{proposed[0],proposed[1],proposed[2]}},10.f,-1.f);
    if(index!=0xffffu){
        const auto* point=navigation.record(index);const auto centre=navigation.position(index);
        long double x=static_cast<long double>(proposed[0])-centre[0];float y=0,z=static_cast<float>(static_cast<long double>(proposed[2])-centre[2]);
        const float radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;const auto square=static_cast<long double>(z)*z+x*x;proposed[1]=0;
        if(static_cast<long double>(radius)*radius<square){
            human_active_avoidance::normalize(x,y,z,square);
            proposed[0]=static_cast<float>(x*radius+centre[0]);proposed[1]=static_cast<float>(static_cast<long double>(y)*radius);
            const float z_offset=static_cast<float>(static_cast<long double>(z)*radius);proposed[2]=static_cast<float>(static_cast<long double>(z_offset)+centre[2]);
        }
        const auto dx=static_cast<long double>(own[0])-proposed[0],dy=-static_cast<long double>(proposed[1]),dz=static_cast<long double>(own[2])-proposed[2];
        proposed[1]=centre[1];
        if(!((dz*dz+dy*dy)+dx*dx<4.f)){
            ai.initialize_task(nullptr,40u,proposed,state.template get<HumanVector>(0x1f0),true,state.template get<float>(0x21c));
            const std::uint32_t count=world.active_retreat_live_count();std::int32_t slot=0;
            if(count!=1u){const auto divisor=human_frame_signed_word(count-1u),identity=ai.human()->base().id10();
                if(identity==std::numeric_limits<std::int32_t>::min()&&divisor==-1)throw std::runtime_error("Retreat stagger reaches native signed division overflow");
                slot=identity%divisor;
            }
            std::uint32_t delay=static_cast<std::uint32_t>(slot)*800u;
            if(state.template get<std::uint32_t>(0x190)!=3u)delay+=(count-2u)*500u;
            state.set(0x214,delay);state.set(0x204,std::uint8_t{1});state.set(0x190,state.template get<std::uint32_t>(0x190)-1u);
            if(delay){
                ai.set_target_direction(state.template get<HumanVector>(0x150));ai.set_current_direction(state.template get<HumanVector>(0x150));
                if(state.template get<std::int32_t>(0x19c)<human_frame_signed_word(delay))state.set(0x19c,delay);
                world.adjust_active_ai_stance(ai,true);world.active_follow_ready_item(ai);
            }
            return true;
        }
    }
    state.set(0x190,0u);return false;
}
}
#endif
