#ifndef HD2_GAME_VERSION_DEBRIS_H
#define HD2_GAME_VERSION_DEBRIS_H
#include "game/physics_object_parameters.h"
#include "game/fowl_bird_random.h"
#include "game/human_joint_owners.h"
namespace hd2::game {
//0043B140. Shared Physics module owns emitted type4 fragments, not Versions.
//The frame's factory reference remains alive through create/three CRT draws/
//optional impulse and retires afterwards; the stable Physics handle is borrowed.
template<class Frames,class Physics,class Random>void emit_version_debris(Frames& frames,Physics& physics,Random& random,
        typename Frames::Frame source,typename Frames::Frame group,std::uint32_t material,
        typename Frames::Frame master,bool effects){
    if(!group)return;
    frames.prepare_version_debris(group,source);
    if(!effects)return;
    for(auto n=frames.actor_child_count(group);n;){
        const auto child=frames.actor_child(group,--n);if(!child||frames.frame_kind(child)!=1)continue;
        auto fragment=frames.create_version_fragment(child);if(!fragment)return;
        PhysicsObjectParameters<typename Frames::Frame> parameters;parameters.frame=fragment.get();parameters.type=4;
        parameters.first_list=true;parameters.sight_pass=true;parameters.material10=material;
        auto* handle=physics.create(parameters);
        const float z=static_cast<float>(fowl_random(random,1.5f)-.75f);
        const float y=static_cast<float>(fowl_random(random,1.5f)-.75f);
        const float x=static_cast<float>(fowl_random(random,1.5f)-.75f);
        const auto world=frames.actor_world_matrix(fragment.get());
        if(master){
            const auto origin=frames.actor_world_matrix(master);HumanVector force{},point{};
            const float dx=static_cast<float>(static_cast<long double>(world[12])-origin[12]);
            const auto dy=static_cast<long double>(world[13])-origin[13],dz=static_cast<long double>(world[14])-origin[14];
            force[0]=static_cast<float>(static_cast<long double>(static_cast<float>(static_cast<long double>(dx)*3.f))+x);
            force[1]=static_cast<float>(static_cast<long double>(static_cast<float>(dy*3.f))+y);
            force[2]=static_cast<float>(static_cast<long double>(static_cast<float>(dz*3.f))+z);
            point={{static_cast<float>(static_cast<long double>(world[12])+x),static_cast<float>(static_cast<long double>(world[13])+y),static_cast<float>(static_cast<long double>(world[14])+z),0}};
            if(handle)physics.impulse(handle,force,point,1.f);
        }
    }
}
}
#endif
