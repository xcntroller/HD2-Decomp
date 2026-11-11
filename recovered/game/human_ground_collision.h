#ifndef HD2_GAME_HUMAN_GROUND_COLLISION_H
#define HD2_GAME_HUMAN_GROUND_COLLISION_H

#include "game/collision_motion.h"
#include "game/human_state_frame_timers.h"
#include "game/human_joint_owners.h"

namespace hd2::game {

//00766050 fills the native multisphere request from the actual DD0 posture.
//The separate simple remote-pose branch still requires its original receiver.
template<class Control,class Look,class Human,class Services,class Hit>
void query_human_ground_collision(Control& control,const Look& look,Human& human,Services& services,
        CollisionMotion<Hit>& request,std::optional<Hit>& identity,bool& supported,
        std::array<float,4>& offset,float height){
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const auto word=[&](unsigned address){return state.template get<std::int32_t>(address);};
    const auto move_word=[&](unsigned address){return movement.template get<std::int32_t>(address);};
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    const bool prone=posture==2||(word(0x9a4)==2&&word(0x9a0)>0);
    const bool floor=look.state().template get<std::uint32_t>(0xb8)==5;
    const auto flags=state.template get<std::uint32_t>(0x22c);
    const bool player=control.human_is_player(),authority=control.animation_authority_actor();
    const bool offline=(control.mode_flags()&7u)==7u;
    if(!(player||((!(flags&2u)||move_word(0xec)>0||prone||floor)&&(offline||authority))))
        throw std::runtime_error("Human ground collision requires the original simple remote-pose query receiver");
    const auto count=floor?physical.primary_count():(flags&1u)?1:physical.primary_count();
    if(count<1||static_cast<std::size_t>(count)>request.centers.size())
        throw std::runtime_error("Human ground query requires actual posture spheres");
    request.count=static_cast<std::size_t>(count);
    for(std::size_t index=0;index<request.count;++index){
        request.centers[index]=physical.posture_spheres().at(index);
        request.centers[index][1]+=height;
        request.displacement=movement.template get<HumanVector>(0x10);
        request.excluded_layers=0xc8002u;
        if(!player||(std::abs(static_cast<long double>(request.displacement[0]))<=9.99999993922529e-09&&std::abs(static_cast<long double>(request.displacement[2]))<=9.99999993922529e-09))
            request.excluded_layers|=0x4000u;
        if(!player)request.excluded_layers|=0x100u;
        request.excluded_flags=human.properties.field19c==1?0x230000u:0xa30000u;
        if(!player)request.excluded_flags|=0x800000u;
        const auto sphere=physical.posture_spheres().at(index);
        if(move_word(0x60)==-10000||movement.template get<float>(0x5c)==-10000.099609375f||
                std::abs(movement.template get<float>(0x5c))<=2.){
            request.radii[index]=sphere[3];
        }else{
            request.radii[index]=static_cast<float>(static_cast<long double>(sphere[3])*.7f);
            request.centers[index][1]=static_cast<float>(request.centers[index][1]-static_cast<long double>(sphere[3])*.3f);
            if(!index)offset[1]=static_cast<float>(static_cast<long double>(sphere[3])*.3f+offset[1]);
        }
    }
    auto& process=physical.frame_globals();
    process.motion_iterations=prone?6.f:(flags&0x100u)?2.f:(flags&0x40u)?3.f:4.f;
    if(!prone&&player)process.motion_iterations+=1;
    struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
    physical.exclude_query(false);
    identity=prone||floor?advance_prone_collision_motion(request,services,process.motion_iterations):
        advance_standing_collision_motion(request,services,process.motion_iterations);
    if((player||(!offline&&!authority))&&move_word(0x68)<0){
        auto origin=physical.posture_spheres().at(0);
        const auto hit=services.human_geometry_ray(origin,{{0,static_cast<float>(-.35f-origin[3]),0,0}},0xc8002u,
            human.properties.field19c==1?0x230000u:0xa30000u);
        supported=hit.has_value();
        if(hit){process.ground_normal=hit->normal;movement.set(0x68,!offline&&!player&&!authority?150u:50u);}
    }
}

} //namespace hd2::game
#endif
