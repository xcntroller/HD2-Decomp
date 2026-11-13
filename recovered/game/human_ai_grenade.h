#ifndef HD2_GAME_HUMAN_AI_GRENADE_H
#define HD2_GAME_HUMAN_AI_GRENADE_H
#include "game/human_ai_cover.h"
#include "game/human_ai_frame_metrics.h"
namespace hd2::game {
//005584B0: predict reverse hostiles, retain10..50m candidates and trim the
//farthest point until the cluster fits10m. Selection performs no projectile creation.
template<class Guard,class World>bool select_human_guard_grenade(Guard& guard,World& world){
    auto* human=guard.human();const auto item=world.human_feedback_kind9(*human);auto& derived=guard.state();
    const auto age=derived.template get<std::int32_t>(0x1ac);
    if(!item||age<10000||!derived.template get<std::uint32_t>(0x1a8))return false;
    if((guard.words().template get<std::uint32_t>(0x50)&0x200001u)||!world.human_feedback_kind9(*human))return false; //00573210 repeats the actual holder query.
    const auto& hostiles=human->perception.lists()[0];
    if(!hostiles.empty()&&static_cast<float>(age)<(40.L/static_cast<float>(hostiles.size()))*1000.f)return false;
    const auto nearest=human_ai_closest_enemy_distance(*human,world.human_command_frames());
    const auto speed=world.human_frame_output_item(item).primary_payload_ballistic_speed();
    const float lead=static_cast<float>(nearest*1.149999976158142f/speed);
    std::vector<std::array<float,3>> positions;
    for(auto n=hostiles.size();n;){const auto p=world.human_grenade_lead_position(*hostiles[--n],lead);const auto origin=world.human_command_frames().actor_world_matrix(human->base().frame());
        const auto x=static_cast<long double>(origin[12])-p[0],y=static_cast<long double>(origin[13])-p[1],z=static_cast<long double>(origin[14])-p[2];const float square=static_cast<float>((z*z+y*y)+x*x);
        if(square<=2500.f&&square>=100.f)positions.push_back(p);
    }
    const auto publish=[&](const auto& p){for(unsigned n=0;n<3;++n)guard.resources().state().set(0xe0+n*4,p[n]);};
    if(positions.size()==1){publish(positions.front());return true;}
    while(!positions.empty()){
        std::array<float,3> sum{};for(auto n=positions.size();n;){const auto& p=positions[--n];for(unsigned axis=0;axis<3;++axis)sum[axis]=static_cast<float>(static_cast<long double>(sum[axis])+p[axis]);}
        const float inverse=static_cast<float>(1.L/static_cast<float>(positions.size()));
        std::array<float,3> centre;for(unsigned n=0;n<3;++n)centre[n]=static_cast<float>(static_cast<long double>(sum[n])*inverse);
        float farthest=0;std::size_t selected=positions.size();
        for(auto n=positions.size();n;){const auto i=--n;const auto x=static_cast<long double>(positions[i][0])-static_cast<long double>(inverse)*sum[0],y=static_cast<long double>(positions[i][1])-static_cast<long double>(sum[1])*inverse,z=static_cast<long double>(positions[i][2])-static_cast<long double>(sum[2])*inverse;const float square=static_cast<float>((x*x+y*y)+z*z);if(square>farthest){farthest=square;selected=i;}}
        if(farthest<=100.f){publish(centre);return true;}
        if(selected==positions.size())throw std::logic_error("Grenade cluster reached native invalid farthest index");
        positions.erase(positions.begin()+static_cast<std::ptrdiff_t>(selected));
    }
    derived.set(0x1ac,0u);return false;
}
//00572DE0: actual kind9 ballistic aim, output40 and animation deadline.
template<class AI,class World>void aim_human_ai_grenade(AI& ai,World& world){
    auto* human=ai.human();const auto item=world.human_feedback_kind9(*human);if(!item)return;
    const auto speed=world.human_frame_output_item(item).primary_payload_ballistic_speed();
    const auto matrix=world.human_command_frames().actor_world_matrix(human->base().frame());HumanVector origin{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto flags=ai.words().template get<std::uint32_t>(0x50);
    if(flags&4u)origin[1]=static_cast<float>(static_cast<long double>(origin[1])+1.7999999523162842f);
    else if(flags&2u)origin[1]=static_cast<float>(static_cast<long double>(origin[1])+1.100000023841858f);
    else if(flags&1u)return;
    auto& state=ai.resources().state();const auto target=state.template get<HumanVector>(0xe0);
    const auto x=static_cast<long double>(target[0])-origin[0],y=static_cast<long double>(target[1])-origin[1],z=static_cast<long double>(target[2])-origin[2];
    const float distance_square=static_cast<float>((z*z+y*y)+x*x);
    const float flight=static_cast<float>(human_ai_cover::length(distance_square)*1.100000023841858f/speed);
    state.set(0xe4,0u);HumanVector direction=target;direction[0]=static_cast<float>(x);direction[1]=0;direction[2]=static_cast<float>(z);
    const float planar_square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
    const float planar=static_cast<float>(human_ai_cover::length(planar_square));
    const float height=static_cast<float>(static_cast<long double>(target[1])-origin[1]);
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
    const float tangent=static_cast<float>((static_cast<long double>(flight)*flight*4.90500020980835f+height)*(1.L/planar));
    const auto angle=std::atan(static_cast<long double>(tangent));
    const float cosine=static_cast<float>(std::cos(angle));direction[0]=static_cast<float>(static_cast<long double>(direction[0])*cosine);direction[2]=static_cast<float>(static_cast<long double>(direction[2])*cosine);
    const auto sine=std::sin(static_cast<long double>(static_cast<float>(angle)));direction[1]=static_cast<float>(sine);
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+sine*direction[1]);
    ai.resources().direction().set_target(direction);
    auto* input=ai.second_frame_inputs();if(!input)throw std::logic_error("Grenade aim requires actual second input owner");input->set(0x40,1.f);
    const auto duration=world.human_grenade_animation_duration((~flags>>2)&1u);
    state.set(0xc8,duration);state.set(0xcc,static_cast<std::uint32_t>(duration)+100u);
}
} //namespace hd2::game
#endif
