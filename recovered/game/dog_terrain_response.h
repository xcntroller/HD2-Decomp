#ifndef HD2_GAME_DOG_TERRAIN_RESPONSE_H
#define HD2_GAME_DOG_TERRAIN_RESPONSE_H
#include "game/human_motion_velocity.h"
namespace hd2::game {
//007884F0 uses an identity matrix and native FLOAT sin/cos stores for X
//rotation, then the actual engine vector operator*=; no quaternion substitute.
template<class Frames>void rotate_dog_terrain_x(HumanVector& value,float angle,Frames& frames){
    auto matrix=human_identity();const float sine=static_cast<float>(std::sin(static_cast<long double>(angle))),cosine=static_cast<float>(std::cos(static_cast<long double>(angle)));
    matrix[5]=matrix[10]=cosine;matrix[9]=sine;matrix[6]=-sine;
    frames.car_transform_in_place(value,matrix);
}
namespace dog_terrain_math {
inline long double retained_x_normalize(HumanVector& value,long double square){
    long double x=value[0];const float stored=static_cast<float>(square);
    if(!(std::abs(square-1)<9.99999993922529e-09f)){
        if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;value[1]=static_cast<float>(value[1]*inverse);value[2]=static_cast<float>(value[2]*inverse);}
        else if(x!=0)x=x<0?-1:1;
        else if(value[2]!=0)value[2]=value[2]<0?-1.f:1.f;
        else value[1]=value[1]<0?-1.f:1.f;
    }
    return x;
}
inline long double square_xzy(const HumanVector& v){return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[2])*v[2])+static_cast<long double>(v[1])*v[1];}
}
//005C26D0: actual Actor pose snapshot, two downward probes with individual
//raised retries, combined normals, engine AngleTo/Y rotations, native X wrapper,
//direction blending and bounded Y change. Normal/geometry identities stay in
//the same World; the native no-hit X,Y,Y,Z publication is deliberately retained.
template<class Dog,class World,class Frames>
void align_dog_to_terrain(Dog& dog,std::int32_t elapsed,World& world,Frames& frames){
    const float limit=static_cast<float>(static_cast<long double>(elapsed)*.004000000189989805f);
    const auto position=dog.base().world_position(),direction=dog.base().world_direction();
    struct Restore{World& world;bool active=true;~Restore(){if(active)try{world.sight_restore_dog();}catch(...){}}void finish(){world.sight_restore_dog();active=false;}} restore{world};
    world.sight_exclude_dog(&dog);
    auto first=position,second=position;
    first[0]=static_cast<float>(static_cast<long double>(position[0])-static_cast<long double>(direction[0])*.10000000149011612f);
    const float dy=static_cast<float>(static_cast<long double>(direction[1])*.10000000149011612f),dz=static_cast<float>(static_cast<long double>(direction[2])*.10000000149011612f);
    first[1]=static_cast<float>((static_cast<long double>(position[1])+.4000000059604645f)-dy);
    first[2]=static_cast<float>(static_cast<long double>(position[2])-dz);
    for(unsigned n=0;n!=2;++n)second[n]=static_cast<float>(static_cast<long double>(first[n])+static_cast<long double>(direction[n])*.6000000238418579f);
    const float offset_z=static_cast<float>(static_cast<long double>(direction[2])*.6000000238418579f);second[2]=static_cast<float>(static_cast<long double>(first[2])+offset_z);
    auto a=world.human_geometry_ray(first,{{0,-2,0,0}},0xffffffffu,0x10000u);
    if(!a){first[1]=static_cast<float>(static_cast<long double>(first[1])+.4000000059604645f);a=world.human_geometry_ray(first,{{0,-2,0,0}},0xffffffffu,0x10000u);}
    auto b=world.human_geometry_ray(second,{{0,-2,0,0}},0xffffffffu,0x10000u);
    if(!b){second[1]=static_cast<float>(static_cast<long double>(second[1])+.4000000059604645f);b=world.human_geometry_ray(second,{{0,-2,0,0}},0xffffffffu,0x10000u);}
    HumanVector normal{};const bool hit=bool(a)||bool(b);
    if(a&&b){
        normal[0]=static_cast<float>((static_cast<long double>(a->normal[0])+b->normal[0])*.5f);
        normal[1]=static_cast<float>((static_cast<long double>(a->normal[1])+b->normal[1])*.5f);
        const float sum_z=static_cast<float>(static_cast<long double>(a->normal[2])+b->normal[2]);normal[2]=static_cast<float>(static_cast<long double>(sum_z)*.5f);
    }else if(hit){normal=a?a->normal:b->normal;normal[1]=static_cast<float>(static_cast<long double>(normal[1])+1.f);for(unsigned n=0;n!=3;++n)normal[n]=static_cast<float>(static_cast<long double>(normal[n])*.5f);}
    restore.finish();
    if(!hit){
        dog.body.field200=1;auto output=direction;
        if(!(limit<std::abs(static_cast<long double>(output[1]))))output[1]=0;
        else output[1]=static_cast<float>(static_cast<long double>(output[1])+(output[1]>=0?-static_cast<long double>(limit):limit));
        normalize_human_ai_shot(output,dog_terrain_math::square_xzy(output));
        dog.base().world_direction({{output[0],output[1],output[1],output[2]}}); //Raw005C31C3..31D3, including produced W=Z.
        dog.body.flag231=true;return;
    }
    const HumanVector horizontal{{direction[0],0,direction[2],0}};
    const float tilt=static_cast<float>(frames.rigid_vector_angle(horizontal,normal));
    const auto difference=std::abs(human_motion_planar_angle(direction,normal));
    dog.body.field200=static_cast<float>(difference);
    if(difference>1.5707963705062866f)dog.body.field200=static_cast<float>(3.1415927410125732f-difference);
    dog.body.field200=static_cast<float>(1.L-static_cast<long double>(dog.body.field200)*.6366197466850281f);
    const float first_angle=static_cast<float>(-human_motion_planar_angle({{0,0,1,0}},normal));
    frames.car_transform_in_place(normal,frames.human_y_rotation(first_angle));
    rotate_dog_terrain_x(normal,tilt<1.5707963705062866f?-1.5707963705062866f:1.5707963705062866f,frames);
    const float second_angle=static_cast<float>(-human_motion_planar_angle(direction,normal));
    frames.car_transform_in_place(normal,frames.human_y_rotation(second_angle));
    const auto normal_x=dog_terrain_math::retained_x_normalize(normal,(static_cast<long double>(normal[1])*normal[1]+static_cast<long double>(normal[0])*normal[0])+static_cast<long double>(normal[2])*normal[2]);
    const float previous=static_cast<float>(1.L-dog.body.field200);const auto weight=static_cast<long double>(dog.body.field200);
    normal[0]=static_cast<float>(static_cast<long double>(previous)*direction[0]+normal_x*weight);
    const float zero=static_cast<float>(static_cast<long double>(previous)*0.f);
    normal[1]=static_cast<float>(static_cast<long double>(zero)+normal[1]*weight);
    const float old_z=static_cast<float>(static_cast<long double>(previous)*direction[2]),new_z=static_cast<float>(static_cast<long double>(normal[2])*weight);
    normal[2]=static_cast<float>(static_cast<long double>(old_z)+new_z);
    normalize_human_ai_shot(normal,dog_terrain_math::square_xzy(normal));
    if(limit<std::abs(static_cast<long double>(normal[1])-direction[1])){
        const auto new_y=static_cast<long double>(direction[1])+(normal[1]>direction[1]?static_cast<long double>(limit):-static_cast<long double>(limit));
        normal[1]=static_cast<float>(new_y);
        normalize_human_ai_shot(normal,(new_y*normal[1]+static_cast<long double>(normal[0])*normal[0])+static_cast<long double>(normal[2])*normal[2]);
    }
    dog.base().world_direction(normal);dog.body.flag231=true; //Positive-path W has no claimed native value.
}
}
#endif
