#ifndef HD2_FRONTEND_SOUND_PORTAL_ROUTING_H
#define HD2_FRONTEND_SOUND_PORTAL_ROUTING_H

#include "frontend/sound_runtime.h"

namespace hd2::frontend {

//00432BE0 has distinct native vector-spill and direct-x87 distance branches.
//Same-sector returns extended sqrt; portal hits spill the final result to FLOAT.
inline long double sound_portal_length(const SoundPosition& first,const SoundPosition& second,bool spill_difference) {
    std::array<long double,3> difference{};
    for(unsigned axis=0;axis!=3;++axis) {
        const auto value=static_cast<long double>(first[axis])-second[axis];
        difference[axis]=spill_difference?static_cast<long double>(static_cast<float>(value)):value;
    }
    //Constructor branches use (X²+Z²)+Y²; the direct second-side branch
    //at00433235 uses (X²+Y²)+Z² without spilling the coordinate differences.
    const auto x=difference[0]*difference[0],y=difference[1]*difference[1],z=difference[2]*difference[2];
    const float square=static_cast<float>(spill_difference?(x+z)+y:(x+y)+z);
    std::uint32_t bits;std::memcpy(&bits,&square,4);
    if(sound_signed_word(bits)<0x322bcc77)return 0;
    if(std::abs(static_cast<long double>(square)-1.L)<1.0e-8f)return 1;
    return std::sqrt(static_cast<long double>(square));
}

//Original00432BE0: inspect the two directed arrays in reverse, accepting the
//first direct or shared-neighbour match. This is a bounded TWO-hop lookup,
//not a shortest-path traversal. Sector/portal pointers remain actual borrowers.
template<class Backend>
long double sound_portal_distance(float& scale,const SoundPosition& first,const SoundPosition& second,
                                  void* first_sector,void* second_sector,float range,Backend& backend) {
    (void)range; //Native passes this through00433330, whose body does not consume it.
    const auto first_copy=first,second_copy=second;
    const auto scene=[&](){
        const auto* world=backend.sound_world();
        if(!world||!world->scene)throw std::runtime_error("Sound portal routing requires the actual world scene");
        return world->scene;
    };
    if(!first_sector)first_sector=backend.sound_sector(scene(),first_copy);
    if(!second_sector)second_sector=backend.sound_sector(scene(),second_copy);
    scale=1;
    if(first_sector==second_sector)return sound_portal_length(first,second,true);
    const auto root=backend.primary_sound_sector(scene());
    if(first_sector==root||second_sector==root) {
        const auto distance=sound_sector_distance(scale,first,second,first_sector,second_sector,backend);
        return distance*scale; //Native returns attenuation in BOTH the output scale and this distance.
    }
    if(!first_sector||!second_sector)throw std::runtime_error("Sound portal routing has a null resolved sector");
    struct Neighbour {void* sector;float weight;};
    std::vector<Neighbour> neighbours;
    for(unsigned side=0;side!=2;++side)for(auto n=backend.sound_portal_count(first_sector,side);n;) {
        const auto portal=backend.sound_portal(first_sector,side,--n);
        const auto sector=backend.sound_portal_sector(portal,side);
        const bool opened=backend.sound_portal_opened(portal); //Called even before a direct-match test.
        if(sector==second_sector) {
            if(!opened)scale=.5f;
            return static_cast<float>(sound_portal_length(first,second,true));
        }
        neighbours.push_back({sector,opened?1.f:.5f});
    }
    for(unsigned side=0;side!=2;++side)for(auto n=backend.sound_portal_count(second_sector,side);n;) {
        const auto portal=backend.sound_portal(second_sector,side,--n);
        const auto sector=backend.sound_portal_sector(portal,side);
        for(std::size_t i=neighbours.size();i;)if(neighbours[--i].sector==sector) {
            const auto weight=neighbours[i].weight;
            scale=static_cast<float>(static_cast<long double>(weight)*scale);
            if(!backend.sound_portal_opened(portal))
                scale=static_cast<float>(static_cast<long double>(weight)*.5f*scale); //Native repeats FIRST weight: both closed gives .125.
            return static_cast<float>(sound_portal_length(first,second,side==0));
        }
    }
    scale=1;return std::numeric_limits<float>::max();
}

} //namespace hd2::frontend
#endif
