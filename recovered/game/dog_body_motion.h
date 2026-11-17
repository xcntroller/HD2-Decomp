#ifndef HD2_GAME_DOG_BODY_MOTION_H
#define HD2_GAME_DOG_BODY_MOTION_H
#include "game/dog_fall_response.h"
#include "game/dog_damage.h"
#include <utility>
#include "game/dog_slope_response.h"
#include "game/dog_terrain_response.h"
#include <type_traits>
namespace hd2::game {
//005C3420 composes the SAME World's existing0071BAF0 solver/ray/sweep,
//actual Body/clip/descriptor owners and shared process budget/diagnostic queue.
//A separate native local-normal producer remains a late required dependency
//only where it can affect005C43D0. No unknown scratch normal is silently zeroed.
template<class Dog,class World,class Frames,class Controller>
void advance_dog_body_motion(Dog& dog,std::int32_t elapsed,World& world,Frames& frames,Controller& control){
    using Sweep=std::remove_reference_t<decltype(world.human_geometry_sweep())>;using Hit=typename Sweep::Hit;
    using Vector=std::array<float,4>;auto& b=dog.body;auto& actions=dog.body_actions;
    if(!dog.collisions)throw std::runtime_error("Dog Body motion requires actual collision anchor owner");
    Vector anchor=dog.collisions->anchor(0),offset{};const float original_budget=world.dog_physics_globals().motion_iterations;const auto category=dog_action_category(dog_order_signed(actions.current364));
    if(!actions.death_timer378)throw std::runtime_error("Dog Body motion requires actual378 timer producer");
    if(*actions.death_timer378!=-10000){if(*actions.death_timer378<=0){actions.death374=4;return;}actions.death_timer378=dog_order_signed(static_cast<std::uint32_t>(*actions.death_timer378)-static_cast<std::uint32_t>(elapsed));}
    if(dog_order_signed(b.field240)>0)--b.field240;
    if(!dog.clip)throw std::runtime_error("Dog Body motion requires actual clip displacement owner");
    auto movement=dog.clip->movement();std::optional<Hit> identity;bool rested=false,supported=false;
    const auto with_exclusions=[&](auto&& work){
        struct Restore{World& world;bool active=true;~Restore(){if(active)try{world.sight_restore_dog();}catch(...){}}void finish(){world.sight_restore_dog();active=false;}} restore{world};
        world.sight_exclude_dog(&dog);auto result=work();restore.finish();return result;
    };
    const auto square=(static_cast<long double>(movement[2])*movement[2]+static_cast<long double>(movement[0])*movement[0])+static_cast<long double>(movement[1])*movement[1];
    if(square<1.0000000168623835e-16f){
        b.field1f8=-10000.099609375f;
        if(b.field1f4>=140){
            if(b.field1f4>10000)b.field1f4=10000;
            auto pair=with_exclusions([&]{
                const float first_drop=static_cast<float>(-static_cast<long double>(dog.collisions->radius(0))-.10000000149011612f);
                auto first=world.human_geometry_ray(dog.collisions->anchor(0),{{0,first_drop,0,0}},0x80002u,0x10000u);
                const float second_drop=static_cast<float>(-static_cast<long double>(dog.collisions->radius(1))-.10000000149011612f);
                auto second=world.human_geometry_ray(dog.collisions->anchor(1),{{0,second_drop,0,0}},0x80002u,0x10000u);
                return std::make_pair(first,second);
            });
            identity=pair.first;rested=bool(pair.first)||bool(pair.second);
            if(rested){b.field264=0;b.material1f0=pair.first?dog_order_signed(world.human_geometry_material(*pair.first)):-1;if(!pair.first&&pair.second)b.material1f0=dog_order_signed(world.human_geometry_material(*pair.second));}
            b.field250=b.field254=0;
        }
        const bool zero=b.field250==0.f;b.field1f4=dog_order_signed(static_cast<std::uint32_t>(b.field1f4)+static_cast<std::uint32_t>(elapsed));
        if(zero&&!rested)b.field250=9.8100004196167f;
    }else if(b.field250==0.f)b.field250=9.8100004196167f;
    if(!rested){
        movement[1]=static_cast<float>(static_cast<long double>(movement[1])-dog_gravity_displacement(elapsed,b.field250,b.field254));
        CollisionMotion<Hit> request;
        if(!b.flag230){
            for(unsigned n=0;n!=2;++n){
                const auto direction=dog.base().world_direction();
                const bool both=!(std::abs(static_cast<long double>(direction[1]))<.05000000074505806)||category==2||b.field240!=0;
                if(!both&&((category==0&&n==0)||(category!=0&&n==1)))continue;
                const auto slot=request.count;request.radii[slot]=static_cast<float>(static_cast<long double>(dog.collisions->radius(n))*1.5f);
                const auto centre=dog.collisions->anchor(n);request.centers[slot]=centre;
                request.centers[slot][1]=static_cast<float>(static_cast<long double>(dog.collisions->radius(n))*.25f+centre[1]);
                if(n==0||(n==1&&category==0&&slot==0)){
                    const auto position=dog.base().world_position();for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<float>(static_cast<long double>(position[axis])-request.centers[slot][axis]);anchor=request.centers[slot];
                }
                ++request.count;
            }
            request.displacement=movement;request.excluded_flags=0xb10000u;request.excluded_layers=0x88002u;
            auto& budget=world.dog_physics_globals().motion_iterations;const auto saved=original_budget;
            struct RestoreBudget{float& value;float saved;bool active=true;~RestoreBudget(){if(active)value=saved;}void finish(){value=saved;active=false;}} restore_budget{budget,saved};
            identity=with_exclusions([&]{
                if(actions.current364==5){budget=4;if(request.displacement[1]<-.10000000149011612f)request.displacement[1]=movement[1]=-.1f;}
                else{const auto speed=control.dog_control_speed();budget=!(speed<=1.649999976158142f)?4.f:3.f;}
                return advance_standing_collision_motion(request,world,budget);
            });
            if(identity){
                const auto position=dog.base().world_position();Vector delta{};
                for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>((static_cast<long double>(offset[axis])+request.position[axis])-position[axis]);
                if(delta[1]>.20000000298023224f&&movement[1]<0.f&&static_cast<long double>(delta[1])-movement[1]>.30000001192092896f){
                    request.displacement={{0,-delta[1],0,0}};
                    for(std::size_t n=0;n!=request.count;++n){for(unsigned axis=0;axis!=3;++axis)request.centers[n][axis]=static_cast<float>(static_cast<long double>(request.centers[n][axis])+delta[axis]);request.radii[n]=static_cast<float>(static_cast<long double>(request.radii[n])*.8999999761581421f);}
                    if(request.contact_count){if(!request.contacts[0])throw std::runtime_error("Dog step retry lost actual first contact");for(unsigned axis=0;axis!=3;++axis)request.contacts[0]->normal[axis]=0;}
                    for(unsigned axis=0;axis!=3;++axis)request.position[axis]=0;
                    identity=with_exclusions([&]{return advance_standing_collision_motion(request,world,budget);});
                    if(!identity)for(unsigned axis=0;axis!=3;++axis)request.displacement[axis]=static_cast<float>(static_cast<long double>(request.displacement[axis])+delta[axis]);
                }
            }
            restore_budget.finish();
        }else{
            const auto position=dog.base().world_position();Vector start=position;
            start[0]=static_cast<float>(static_cast<long double>(position[0])+movement[0]);start[1]=static_cast<float>(static_cast<long double>(position[1])+1.f);start[2]=static_cast<float>(static_cast<long double>(position[2])+movement[2]);
            anchor=dog.collisions->anchor(category==0?1u:0u);for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<float>(static_cast<long double>(position[axis])-anchor[axis]);
            const float down=static_cast<float>(static_cast<long double>(movement[1])-1.f);
            identity=with_exclusions([&]{return world.human_geometry_ray(start,{{0,down,0,0}},0xffffffffu,0x10000u);});
            if(identity){for(unsigned axis=0;axis!=3;++axis)request.position[axis]=static_cast<float>(static_cast<long double>(identity->position[axis])-offset[axis]);request.contacts[0]=typename CollisionMotion<Hit>::Contact{identity->normal,0,*identity};request.contact_count=1;}
            b.field24c=0;
        }
        supported=dog_contacts_supported(request,world);b.field24c=supported?0u:b.field24c+1u;
        //The native separate normal/byte1 can affect only this slope worker.
        //When its other gates reject, its outputs are independent of BOTH
        //unknown values. Otherwise the exact existing worker fails at the late
        //required normal read, after all preceding solver/counter publications.
        apply_dog_slope_response(dog,request,bool(identity),true,nullptr,anchor,movement,elapsed);
        apply_dog_fall_response(dog,bool(identity),supported,elapsed,request.displacement,world);
        if(identity){
            auto result=request.position;for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(static_cast<long double>(result[axis])+offset[axis]);
            if(movement[0]==0.f&&movement[2]==0.f&&b.field1f4>=0){b.field264+=static_cast<std::uint32_t>(elapsed);const auto current=dog.base().world_position();result[0]=current[0];result[2]=current[2];}else b.field264=0;
            if(!supported)b.field240=5;else{b.field250=b.field254=0;b.field25c=-10000;b.field264=0;}
            b.material1f0=dog_order_signed(world.human_geometry_material(*identity));
            const auto current=dog.base().world_position();if(result[0]!=current[0]||result[1]!=current[1]||result[2]!=current[2]){result[3]=current[3];dog.base().world_position(result);b.flag232=true;}
        }
    }
    auto matrix=frames.actor_world_matrix(dog.base().frame());long double y=matrix[9];
    const auto axis_square=(y*y+static_cast<long double>(matrix[10])*matrix[10])+static_cast<long double>(matrix[8])*matrix[8];const float stored=static_cast<float>(axis_square);
    if(!(std::abs(axis_square-1)<9.99999993922529e-09f)){
        if(!(stored<9.99999993922529e-09f))y/=std::sqrt(static_cast<long double>(stored));
        else if(matrix[8]==0.f&&matrix[10]==0.f)y=y<0?-1:1;
    }
    if(std::abs(y)>.009999999776482582)b.flag232=true;
    if(!actions.death374)align_dog_to_terrain(dog,elapsed,world,frames);
    matrix=frames.actor_world_matrix(dog.base().frame());const Vector start{{matrix[12],matrix[13],matrix[14],matrix[15]}};auto end=start;
    for(unsigned axis=0;axis!=3;++axis)end[axis]=static_cast<float>(static_cast<long double>(start[axis])+movement[axis]);
    world.dog_physics_motion_line(start,end);
}
}
#endif
