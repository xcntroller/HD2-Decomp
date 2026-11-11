#ifndef HD2_GAME_COLLISION_MOTION_H
#define HD2_GAME_COLLISION_MOTION_H

#include "game/actor_activation_math.h"
#include <limits>
#include <optional>

namespace hd2::game {

//0071BAF0/0071C7D0 use ten spheres and ten contact slots in the native
//request. Identities borrow the SAME tree/descriptors as the supplied queries.
template<class Hit>struct CollisionMotion {
    using Vector=std::array<float,4>;
    std::size_t count=0;
    std::array<Vector,10> centers{};
    std::array<float,10> radii{};
    Vector displacement{},position{};
    std::uint32_t excluded_layers=0,excluded_flags=0;
    struct Contact {Vector normal;std::size_t sphere;Hit identity;};
    std::array<std::optional<Contact>,10> contacts{};
    std::size_t contact_count=0;

    void append(const Hit& hit,std::size_t sphere){
        if(contact_count==contacts.size())throw std::runtime_error("Collision motion exceeds original contact storage");
        contacts[contact_count++]=Contact{hit.normal,sphere,hit};
    }
    void validate()const{
        if(!count||count>centers.size())throw std::runtime_error("Collision motion requires one to ten actual spheres");
        if(contact_count>contacts.size())throw std::runtime_error("Collision motion contact count exceeds original storage");
    }
};

namespace collision_motion_math {
using Vector=std::array<float,4>;
constexpr float epsilon=9.99999993922529e-09f;
inline float square(const Vector& value){
    return static_cast<float>((static_cast<long double>(value[2])*value[2]+
        static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0]);
}
inline Vector add(const Vector& first,const Vector& second){
    auto result=first;
    for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(static_cast<long double>(first[axis])+second[axis]);
    return result;
}
inline Vector subtract(const Vector& first,const Vector& second){
    auto result=first;
    for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(static_cast<long double>(first[axis])-second[axis]);
    return result;
}
inline Vector scale(const Vector& value,long double factor){
    auto result=value;
    for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(value[axis]*factor);
    return result;
}
inline long double extended_dot(const Vector& first,const Vector& second){
    return (static_cast<long double>(first[0])*second[0]+
        static_cast<long double>(first[1])*second[1])+static_cast<long double>(first[2])*second[2];
}
inline float dot(const Vector& first,const Vector& second){return static_cast<float>(extended_dot(first,second));}
inline float length(const Vector& value){
    const float stored=square(value);
    if(stored<epsilon)return 0;
    return std::abs(static_cast<long double>(stored)-1)<epsilon?1.f:static_cast<float>(std::sqrt(static_cast<long double>(stored)));
}
inline Vector normalized(Vector value){
    const float stored=square(value);
    if(std::abs(static_cast<long double>(stored)-1)>=epsilon){
        if(stored>=epsilon)value=scale(value,1.L/std::sqrt(static_cast<long double>(stored)));
        else{
            const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[axis]=value[axis]<0?-1.f:1.f;
        }
    }
    return value;
}
inline Vector plane_remainder(const Vector& start,const Vector& travel,const Vector& point,const Vector& normal){
    const float plane=-dot(point,normal),denominator=dot(normal,normal);
    const auto end=add(start,travel);
    if(std::abs(denominator)<epsilon)return subtract(end,point);
    const float distance=static_cast<float>(-(extended_dot(normal,end)+plane)/denominator);
    return subtract(add(end,scale(normal,distance)),point);
}
template<class Hit>bool same_identity(const Hit& first,const Hit& second){
    if(first.dynamic_identity||second.dynamic_identity)return first.dynamic_identity==second.dynamic_identity;
    if(!first.static_identity||!second.static_identity)return false;
    return first.static_identity->kind==second.static_identity->kind&&first.static_identity->index==second.static_identity->index;
}
inline std::size_t iterations(float value){
    if(!std::isfinite(value)||value<0||value>10)throw std::runtime_error("Collision motion iteration budget exceeds original storage");
    return static_cast<std::size_t>(value); //Original __ftol, bounded actual process producer.
}
} //namespace collision_motion_math

//0071BAF0. Services supplies actual sweep/ray and descriptor metadata. The
//return retains the last native identity even when the final sweep misses.
template<class Hit,class Services>
std::optional<Hit> advance_standing_collision_motion(CollisionMotion<Hit>& request,Services& services,float budget){
    using namespace collision_motion_math;
    request.validate();
    const auto maximum=iterations(budget);
    auto centers=request.centers;
    for(std::size_t index=0;index<request.count;++index)centers[index][1]+=.3f;
    Vector travel{{request.displacement[0],0,request.displacement[2],request.displacement[3]}};
    float lift=.3f;
    bool lifted=true;
    std::optional<Hit> retained,previous_human;
    const auto lower=[&]{
        for(std::size_t index=0;index<request.count;++index){centers[index]=request.centers[index];centers[index][1]+=.001f;}
        lift=.001f;lifted=false;
    };
    if(square(travel)<1.e-16f){centers[0]=request.centers[0];centers[0][1]+=.001f;lift=.001f;}
    else{
        const auto range=request.radii[0]+.75f;
        const auto inverse=1.L/length(travel);
        const auto probe=scale(scale(travel,inverse),range);
        if(services.human_geometry_ray(centers[0],scale(probe,1.1f),0,request.excluded_flags))lower();
        int selected=static_cast<int>(request.count);
        while(selected>=0&&request.contact_count<maximum){
            float nearest=std::numeric_limits<float>::max();selected=-1;
            std::array<std::optional<Hit>,10> hits{};
            for(std::size_t index=0;index<request.count;++index){
                hits[index]=services.human_geometry_sweep().sweep(centers[index],travel,request.radii[index],request.excluded_layers,request.excluded_flags);
                if(hits[index]&&hits[index]->distance_squared<nearest){nearest=hits[index]->distance_squared;selected=static_cast<int>(index);retained=hits[index];}
            }
            if(retained&&(services.human_geometry_flags(*retained)&0x40u)&&
                    (services.human_geometry_mask(*retained)&0x100u)&&
                    (services.human_geometry_human_mode(*retained)&1u)){
                if(previous_human&&!same_identity(*previous_human,*retained)){
                    request.position=centers[0];return retained;
                }
                previous_human=retained;
            }
            if(lifted&&(selected>0||(retained&&(services.human_geometry_flags(*retained)&0x40u)&&
                    (services.human_geometry_mask(*retained)&0x505300u)))){lower();continue;}
            auto displacement=travel;
            if(selected>=0){
                auto hit=*hits[static_cast<std::size_t>(selected)];
                displacement=subtract(hit.position,centers[static_cast<std::size_t>(selected)]);
                if(-.3f<hit.normal[1]&&hit.normal[1]<.5f){
                    hit.normal[1]=0;hit.normal=normalized(hit.normal);
                    if(lifted){lower();continue;}
                }
                travel=add(plane_remainder(centers[static_cast<std::size_t>(selected)],travel,hit.position,hit.normal),scale(hit.normal,.0005f));
                request.append(hit,static_cast<std::size_t>(selected));
            }
            if(square(displacement)>epsilon)
                for(std::size_t index=0;index<request.count;++index)centers[index]=add(centers[index],displacement);
        }
    }
    const float vertical=request.displacement[1]-lift;
    std::size_t index=0;
    Vector offset{};
    if(vertical>=0){index=request.count-1;offset=subtract(centers[0],centers[index]);}
    const auto hit=services.human_geometry_sweep().sweep(centers[index],{{0,vertical,0,0}},request.radii[index]-.001f,
        request.excluded_layers|0x100u,request.excluded_flags);
    if(hit){
        request.append(*hit,0);auto point=hit->position;point[1]+=.001f;
        request.position=add(point,offset);return hit;
    }
    if(retained){request.position=centers[0];request.position[1]+=vertical;}
    return retained;
}

//0071C7D0. The prone/floor solver retains its horizontal restart and three
//vertical slide attempts, followed by the original raised fallback sweep.
template<class Hit,class Services>
std::optional<Hit> advance_prone_collision_motion(CollisionMotion<Hit>& request,Services& services,float budget){
    using namespace collision_motion_math;
    request.validate();
    auto maximum=iterations(budget);
    auto centers=request.centers;
    for(std::size_t index=0;index<request.count;++index)centers[index][1]+=.1f;
    Vector travel{{request.displacement[0],0,request.displacement[2],request.displacement[3]}};
    const auto direction=normalized(travel);
    bool raised=true,human_contact=false;
    std::optional<Hit> retained;
    if(square(travel)>=1.e-16f){
        int selected=static_cast<int>(request.count);
        while(selected>=0&&request.contact_count<maximum){
            float nearest=std::numeric_limits<float>::max();selected=-1;
            std::array<std::optional<Hit>,10> hits{};
            for(std::size_t index=0;index<request.count;++index){
                hits[index]=services.human_geometry_sweep().sweep(centers[index],travel,request.radii[index],request.excluded_layers,request.excluded_flags);
                if(!hits[index])continue;
                human_contact=human_contact||((services.human_geometry_flags(*hits[index])&0x40u)&&(services.human_geometry_mask(*hits[index])&0x100u));
                if(hits[index]->distance_squared<nearest){nearest=hits[index]->distance_squared;selected=static_cast<int>(index);retained=hits[index];}
            }
            if(raised&&human_contact){
                for(std::size_t index=0;index<request.count;++index){centers[index]=request.centers[index];centers[index][1]+=.001f;}
                raised=false;retained.reset();request.contact_count=0;maximum=iterations(budget-1.f);selected=static_cast<int>(request.count);continue;
            }
            auto displacement=travel;
            if(selected>=0){
                auto hit=*hits[static_cast<std::size_t>(selected)];
                displacement=subtract(hit.position,centers[static_cast<std::size_t>(selected)]);
                if(-.3f<hit.normal[1]&&hit.normal[1]<.6f){hit.normal[1]=0;hit.normal=normalized(hit.normal);}
                travel=add(plane_remainder(centers[static_cast<std::size_t>(selected)],travel,hit.position,hit.normal),scale(hit.normal,.0005f));
                request.append(hit,static_cast<std::size_t>(selected));
            }
            if(square(displacement)>epsilon)
                for(std::size_t index=0;index<request.count;++index)centers[index]=add(centers[index],displacement);
        }
    }
    const float vertical=request.displacement[1]-(raised?.1f:.001f);
    travel={{0,vertical,0,0}};
    std::array<std::optional<Hit>,10> hits{};
    int last_selected=-1;
    for(unsigned attempt=0;attempt!=3;++attempt){
        float nearest=std::numeric_limits<float>::max();int selected=-1;unsigned obstacles=0;
        Vector side{},side_point{};
        for(std::size_t reverse=request.count;reverse!=0;){
            const auto index=--reverse;
            const auto observed=services.human_geometry_sweep().sweep(centers[index],travel,request.radii[index],request.excluded_layers,request.excluded_flags);
            if(!observed)continue;
            hits[index]=observed;
            const auto& hit=*observed;
            if(hit.distance_squared<nearest){nearest=hit.distance_squared;selected=static_cast<int>(index);last_selected=selected;retained=hit;}
            const auto flat=normalized({{hit.normal[0],0,hit.normal[2],hit.normal[3]}});
            if(-.3f<hit.normal[1]&&hit.normal[1]<.6f){
                const float alignment=static_cast<float>((static_cast<long double>(direction[2])*flat[2]+
                    static_cast<long double>(flat[0])*direction[0])+static_cast<long double>(direction[1])*flat[1]);
                if(-.707f<alignment&&alignment<.707f)++obstacles;
                ++obstacles;side=flat;side_point=hit.position;
            }
        }
        if(obstacles==1){
            const auto back=scale(scale(side,-1.L),static_cast<long double>(request.radii[0])+.3f);
            side_point[1]+=.3f;
            if(services.human_geometry_ray(side_point,back,request.excluded_layers,request.excluded_flags))obstacles=2;
        }
        if(selected<0||(obstacles<2&&(!(services.human_geometry_flags(*retained)&0x40u)||
                !(services.human_geometry_mask(*retained)&0x100u))))break;
        const auto index=static_cast<std::size_t>(selected);
        const auto& hit=*hits[index];
        const auto displacement=subtract(hit.position,centers[index]);
        const auto remainder=plane_remainder(centers[index],travel,hit.position,hit.normal);
        const auto flat=normalized({{hit.normal[0],0,hit.normal[2],hit.normal[3]}});
        travel=add(remainder,scale(scale(flat,length(remainder)),.001f));
        request.append(hit,index);
        request.position=subtract(add(centers[0],hit.position),centers[index]);
        for(std::size_t sphere=0;sphere<request.count;++sphere)centers[sphere]=add(centers[sphere],displacement);
    }
    if(last_selected>=0){
        const auto index=static_cast<std::size_t>(last_selected);
        if(!hits[index])throw std::runtime_error("Collision motion lost its retained final sphere hit");
        request.append(*retained,0);
        request.position=subtract(add(centers[0],hits[index]->position),centers[index]);
        return retained;
    }
    float nearest=std::numeric_limits<float>::max();int selected=-1;
    for(std::size_t reverse=request.count;reverse!=0;){
        const auto index=--reverse;
        auto origin=centers[index];origin[1]+=1;
        hits[index]=services.human_geometry_sweep().sweep(origin,{{0,vertical-1.f,0,0}},request.radii[index],request.excluded_layers,request.excluded_flags);
        if(hits[index]&&hits[index]->distance_squared<nearest){nearest=hits[index]->distance_squared;selected=static_cast<int>(index);retained=hits[index];}
    }
    if(selected>=0){
        const auto index=static_cast<std::size_t>(selected);
        request.append(*retained,0);request.position=subtract(add(centers[0],hits[index]->position),centers[index]);return retained;
    }
    if(retained)request.position=add(centers[0],{{0,vertical,0,0}});
    return retained;
}

} //namespace hd2::game
#endif
