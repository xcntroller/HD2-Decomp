#ifndef HD2_GAME_DOG_FALL_RESPONSE_H
#define HD2_GAME_DOG_FALL_RESPONSE_H
#include "game/dog_ground_query.h"
namespace hd2::game {
//005C2260 after the SAME multisphere/step policy. Hit is only tested for NULL;
//the collision identity itself is retained by the caller, not mirrored here.
template<class Dog,class World>
void apply_dog_fall_response(Dog& dog,bool hit,bool supported,std::int32_t elapsed,const std::array<float,4>& movement,World& world){
    auto& b=dog.body;auto& a=dog.body_actions;
    constexpr float sentinel=-10000.099609375f;
    const auto square=[&]{return (static_cast<long double>(movement[0])*movement[0]+static_cast<long double>(movement[1])*movement[1])+static_cast<long double>(movement[2])*movement[2];};
    const auto move=[&]{
        auto position=dog.base().world_position();
        position[0]=static_cast<float>(static_cast<long double>(movement[0])*.8999999761581421f+position[0]);
        position[1]=static_cast<float>(static_cast<long double>(movement[1])*.8999999761581421f+position[1]);
        const float z=static_cast<float>(static_cast<long double>(movement[2])*.8999999761581421f);
        position[2]=static_cast<float>(static_cast<long double>(position[2])+z);dog.base().world_position(position);
    };
    if((!hit||!supported)&&b.field250!=0.f){
        advance_dog_gravity(elapsed,b.field250,b.field254);
        if(b.field25c==-10000){
            //Normal clear occurs at the native point, including a NULL first
            //descriptor. Failure cleanup retracts borrowers without hiding the
            //first query/producer exception.
            struct Restore{World& world;bool active=true;~Restore(){if(active)try{world.sight_restore_dog();}catch(...){}}void finish(){world.sight_restore_dog();active=false;}} restore{world};
            world.sight_exclude_dog(&dog);
            if(!dog.collisions)throw std::runtime_error("Dog fall response requires actual Body collision owner");
            const auto first=world.human_geometry_ray(dog.collisions->anchor(0),{{0,-100,0,0}},0x80002u,0x10000u);
            float depth=50.f;
            if(first)depth=static_cast<float>(std::abs(static_cast<long double>(dog.base().world_position()[1])-first->position[1])+.5);
            if((!first||depth>1.f)&&!a.death374){
                const float radius=static_cast<float>(static_cast<long double>(dog.collisions->radius(0))-.10000000149011612f);
                const auto floor=world.human_geometry_sweep().sweep(dog.collisions->anchor(0),{{0,-depth,0,0}},radius,0x80002u,0x10000u,true);
                if(!floor)b.field258=sentinel;
                else{
                    b.field258=static_cast<float>(static_cast<long double>(floor->position[1])-dog.collisions->radius(0)+.10000000149011612f);
                    const auto position=dog.base().world_position();b.field25c=0;
                    const auto distance=std::abs(static_cast<long double>(position[1])-b.field258);
                    if(position[1]<b.field258||distance<1.2999999523162842f){if(std::abs(static_cast<long double>(depth)-distance)<1.)b.field25c=0;}
                    else if(distance>4.5)b.field25c=1;
                }
            }
            restore.finish();
        }
        if(!(square()<1.0000000168623835e-16f)&&!hit){move();b.field264+=static_cast<std::uint32_t>(elapsed);b.flag232=true;}
    }else if(hit){if(supported)b.field258=sentinel;}
    else{
        advance_dog_gravity(elapsed,b.field250,b.field254);
        if(!(square()<1.0000000168623835e-16f)){move();b.flag232=true;}
    }
    if(b.field25c!=-10000&&b.field258!=sentinel&&!hit&&!a.death374&&b.field25c==0)b.field258=sentinel;
}
}
#endif
