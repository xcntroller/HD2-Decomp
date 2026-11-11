#ifndef HD2_GAME_HUMAN_SCENE_MOVEMENT_H
#define HD2_GAME_HUMAN_SCENE_MOVEMENT_H

#include "game/human_ai_navigation_jobs.h"
#include "game/human_state_frame_timers.h"
#include "game/scene_actor_frame.h"
#include "game/table_storage.h"
#include <algorithm>
#include <optional>

namespace hd2::game {

namespace human_scene_movement {
constexpr float epsilon=9.99999993922529e-09f;
constexpr float contact_radius=.8f; //0080FAD4.
constexpr float contact_square=.64000004529953f; //00810E0C.

inline HumanVector difference(const HumanVector& first,const HumanVector& second){
    return {{first[0]-second[0],first[1]-second[1],first[2]-second[2],first[3]}};
}
inline long double dot(const HumanVector& a,const HumanVector& b){
    return (static_cast<long double>(a[2])*b[2]+static_cast<long double>(a[1])*b[1])+
        static_cast<long double>(a[0])*b[0];
}

inline long double square(const HumanVector& value){
    return (static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+
        static_cast<long double>(value[0])*value[0];
}
inline float fast_inverse_root(float value){
    std::uint32_t bits;
    std::memcpy(&bits,&value,sizeof bits);
    bits=0x5f3759dfu-(bits>>1);
    float estimate;
    std::memcpy(&estimate,&bits,sizeof estimate);
    return static_cast<float>((1.5L-static_cast<long double>(estimate)*estimate*value*.5L)*estimate);
}
inline bool negative_word(float value){
    std::uint32_t bits;
    std::memcpy(&bits,&value,sizeof bits);
    return (bits&0x80000000u)!=0;
}
inline HumanVector record_position(const MissionTableRecord& record){
    HumanVector position{};
    std::memcpy(position.data(),record.prefix.data(),3*sizeof(float));
    return position;
}
inline float record_radius(const MissionTableRecord& record){return record.prefix[0x13]*.0625f;} //0080F740.
template<class World>MissionTableRecord& linked_record(MissionTableRecord& record,unsigned index,World& world){
    if(!record.payload)throw std::runtime_error("Human movement floor requires actual link records");
    const auto* link=static_cast<const unsigned char*>(record.payload)+4*index;
    auto* neighbour=world.movement_table_record(static_cast<std::uint16_t>(link[0]|(link[1]<<8)));
    if(!neighbour)throw std::runtime_error("Human movement floor link has no actual navigation record");
    return *neighbour;
}
inline float volume_gap(const HumanVector& point,const MissionTableRecord& record){
    const auto radius=record_radius(record);
    return static_cast<float>(square(difference(point,record_position(record)))-static_cast<long double>(radius)*radius);
}
inline std::optional<long double> connector_gap(const HumanVector& point,
        const MissionTableRecord& first,const MissionTableRecord& second,bool inclusive_end){
    const auto origin=record_position(first);
    const auto segment=difference(record_position(second),origin);
    auto offset=difference(point,origin);
    const auto span=square(segment);
    if(span<epsilon||(!inclusive_end&&span==epsilon))return std::nullopt;
    const float ratio=static_cast<float>(dot(offset,segment)/span);
    if(negative_word(ratio)||ratio>1||(!inclusive_end&&ratio==1))return std::nullopt;
    for(unsigned i=0;i!=3;++i)offset[i]-=static_cast<float>(static_cast<long double>(ratio)*segment[i]);
    const auto a=static_cast<long double>(record_radius(first));
    const auto b=static_cast<long double>(record_radius(second));
    return square(offset)-(1.L-ratio)*a*a-ratio*b*b;
}
template<class World>void refresh_floor(const HumanVector& position,MissionTableRecord*& floor,
        MissionTableRecord* previous,World& world){ //00569890, at most four original breadth levels.
    if(!floor)return;
    const auto current_gap=volume_gap(position,*floor);
    if(negative_word(current_gap))return;
    if(previous&&previous!=floor){
        const auto previous_gap=volume_gap(position,*previous);
        if(negative_word(previous_gap)){floor=previous;return;}
        for(auto n=previous->flags&0x3fu;n;){
            if(&linked_record(*previous,--n,world)!=floor)continue;
            if(const auto gap=connector_gap(position,*floor,*previous,false);gap&&negative_word(static_cast<float>(*gap))){
                if(previous_gap<=current_gap)floor=previous;
                return;
            }
            break;
        }
    }
    auto& search=world.movement_floor_candidates();
    search.clear();
    search.push_back(floor);
    auto* nearest=floor;
    auto best=current_gap;
    for(unsigned depth=0;depth!=4;++depth){
        auto cursor=search.size();
        search.push_back(nullptr); //Native frontier separator.
        while(cursor){
            auto* centre=search[--cursor];
            if(!centre)break;
            const auto centre_gap=volume_gap(position,*centre);
            for(auto n=centre->flags&0x3fu;n;){
                auto* next=&linked_record(*centre,--n,world);
                if(std::find(search.rbegin(),search.rend(),next)!=search.rend())continue;
                if(next->byte16<3||next->byte16>6)search.push_back(next);
                const auto gap=volume_gap(position,*next);
                if(negative_word(gap)){floor=next;return;}
                if(best<epsilon)continue;
                if(gap<best){nearest=next;best=gap;}
                if(const auto connection=connector_gap(position,*centre,*next,true)){
                    const auto distance=negative_word(static_cast<float>(*connection))?0.L:*connection;
                    if(distance<best){
                        nearest=gap<centre_gap?next:centre;
                        best=static_cast<float>(distance);
                    }
                }
            }
        }
    }
    floor=nearest;
}
inline bool constrain_link(HumanVector& point,const HumanVector& first,float first_radius,
        const HumanVector& second,float second_radius){ //00567DF0.
    const auto horizontal_square=[](const HumanVector& value){
        return static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2];
    };
    const auto from_second=difference(point,second);
    const auto second_square=horizontal_square(from_second);
    const float second_limit=second_radius*second_radius;
    if(second_square<second_limit)return true;
    auto from_first=difference(point,first);
    const auto first_square=horizontal_square(from_first);
    const float first_limit=first_radius*first_radius;
    if(first_square<first_limit)return true;
    from_first[1]=0;
    const auto axis=difference(second,first);
    const auto axis_square=horizontal_square(axis);
    auto offset=from_first;
    auto distance=first_square;
    long double radius=first_radius;
    if(axis_square>epsilon){
        const float projection=static_cast<float>((static_cast<long double>(axis[0])*from_first[0]+
            static_cast<long double>(axis[2])*from_first[2])/axis_square);
        if(!negative_word(projection)){
            if(projection>1){
                offset=from_second;
                offset[1]=0;
                distance=second_square;
                radius=second_radius;
            }else{
                offset[0]-=static_cast<float>(static_cast<long double>(projection)*axis[0]);
                offset[2]-=static_cast<float>(static_cast<long double>(projection)*axis[2]);
                distance=horizontal_square(offset);
                radius=(1.L-projection)*first_radius+static_cast<long double>(projection)*second_radius;
            }
        }
    }
    if(!(distance>radius*radius))return true;
    const auto length=std::sqrt(distance);
    const auto scale=(length-radius)/length;
    for(unsigned i=0;i!=3;++i)point[i]-=static_cast<float>(scale*offset[i]);
    return false;
}
template<class World>bool constrain_point(HumanVector& point,MissionTableRecord*& floor,World& world){ //00568190.
    if(!floor)return false;
    const auto centre=record_position(*floor);
    const auto radius=record_radius(*floor);
    const auto delta=difference(point,centre);
    const auto square=static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2];
    if(square<static_cast<long double>(radius)*radius)return true;
    const auto length=std::sqrt(square);
    const auto scale=(length-radius)/length;
    auto closest=point;
    closest[0]=static_cast<float>(point[0]-scale*delta[0]);
    closest[2]=static_cast<float>(point[2]-scale*delta[2]);
    auto best=human_scene_movement::square(difference(closest,point));
    auto* nearest=floor;
    const auto count=floor->flags&0x3fu;
    for(auto n=count;n;){
        auto* neighbour=&linked_record(*floor,--n,world);
        const auto next=record_position(*neighbour);
        auto candidate=point;
        if(constrain_link(candidate,centre,radius,next,record_radius(*neighbour))){
            if(human_scene_movement::square(difference(next,point))<human_scene_movement::square(difference(centre,point)))floor=neighbour;
            return true;
        }
        const auto candidate_distance=human_scene_movement::square(difference(candidate,point));
        if(candidate_distance<best){
            best=candidate_distance;
            closest=candidate;
            nearest=neighbour;
        }
    }
    point=closest;
    floor=nearest;
    return false;
}
inline void add_correction(HumanVector& current,const HumanVector& adjustment){ //00565540.
    const auto length=square(current);
    if(length<epsilon){current=adjustment;return;}
    const auto projection=(static_cast<long double>(adjustment[1])*current[1]+
        static_cast<long double>(adjustment[2])*current[2]+static_cast<long double>(adjustment[0])*current[0])/length;
    if(negative_word(static_cast<float>(projection))){
        for(unsigned i=0;i!=3;++i)current[i]+=adjustment[i];
    }else if(projection>1){
        const auto factor=projection*length/square(adjustment);
        for(unsigned i=0;i!=3;++i)
            current[i]=static_cast<float>((static_cast<long double>(current[i])+adjustment[i])-factor*adjustment[i]);
    }else{
        for(unsigned i=0;i!=3;++i)
            current[i]=static_cast<float>((adjustment[i]-projection*current[i])+current[i]);
    }
}
struct ContactVolume {
    std::array<HumanVector,3> axes;
    std::array<float,3> radii;
    HumanVector project(const HumanVector& vector,bool scale=true)const{
        HumanVector local{};
        for(unsigned i=0;i!=3;++i)
            local[i]=static_cast<float>(dot(axes[i],vector)/(scale?radii[i]:1.f));
        return local;
    }
    HumanVector unproject(const HumanVector& local)const{
        HumanVector result{};
        for(unsigned i=0;i!=3;++i){
            const auto x=static_cast<long double>(local[0])*radii[0]*axes[0][i];
            const auto y=static_cast<long double>(local[1])*radii[1]*axes[1][i];
            const auto z=static_cast<long double>(local[2])*radii[2]*axes[2][i];
            result[i]=static_cast<float>((x+y)+z);
        }
        return result;
    }
};
inline HumanVector normalize_contact_direction(HumanVector direction){
    const float length=static_cast<float>(square(direction));
    if(std::abs(static_cast<long double>(length)-1)>=epsilon){
        if(length>=epsilon){
            const auto scale=1/std::sqrt(static_cast<long double>(length));
            for(unsigned i=0;i!=3;++i)direction[i]=static_cast<float>(direction[i]*scale);
        }else{
            const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;
            direction[axis]=direction[axis]<0?-1.f:1.f;
        }
    }
    return direction;
}
inline std::optional<HumanVector> ellipsoid_correction(const ContactVolume& volume,
        HumanVector separation,HumanVector velocity,const HumanVector& carried_velocity,bool sweep,float step){
    separation[1]=0;
    auto local=volume.project(separation);
    const float gap=static_cast<float>(square(local)-1);
    if(gap<=epsilon){
        if(!(static_cast<long double>(gap)+1>epsilon))return std::nullopt;
        const auto length=std::sqrt(static_cast<long double>(gap)+1);
        for(unsigned i=0;i!=3;++i)local[i]=static_cast<float>(local[i]*(1-length)/length);
        auto adjustment=volume.unproject(local);
        for(unsigned i=0;i!=3;++i)adjustment[i]+=carried_velocity[i];
        return adjustment;
    }
    if(!sweep)return std::nullopt;
    velocity[1]=0;
    velocity=volume.project(velocity);
    const float speed=static_cast<float>(square(velocity));
    if(speed<epsilon)return std::nullopt;
    const float projection=static_cast<float>(dot(local,velocity));
    const float twice=projection*2;
    if(!negative_word(twice))return std::nullopt;
    const auto ratio=static_cast<long double>(projection)/speed;
    HumanVector closest{};
    for(unsigned i=0;i!=3;++i)closest[i]=static_cast<float>(local[i]-ratio*velocity[i]);
    if(square(closest)>1)return std::nullopt;
    const float discriminant=static_cast<float>(static_cast<long double>(twice)*twice-4*speed*gap);
    if(negative_word(discriminant))return std::nullopt;
    const auto root=std::sqrt(static_cast<long double>(discriminant));
    float time=static_cast<float>((root-twice)/(2.L*speed));
    const float near_time=static_cast<float>((-twice-root)/(2.L*speed));
    if(!negative_word(near_time)&&(negative_word(time)||near_time<time))time=near_time;
    if(negative_word(time)||!(time<=1))return std::nullopt;
    HumanVector offset{};
    for(unsigned i=0;i!=3;++i){
        offset[i]=static_cast<float>(static_cast<long double>(time)*velocity[i]);
        local[i]+=offset[i];
    }
    HumanVector tangent{{velocity[2],0,-velocity[0],0}};
    const auto side=static_cast<float>(dot(local,tangent)/speed);
    const auto scale=static_cast<long double>(fast_inverse_root(speed))*(1-square(offset))*(negative_word(side)?-1:1)*(1-time)*step;
    for(unsigned i=0;i!=3;++i)tangent[i]=static_cast<float>(tangent[i]*scale);
    return volume.unproject(tangent);
}
inline std::optional<HumanVector> ellipsoid_target(const ContactVolume& volume,HumanVector delta,
        const HumanVector& position_delta,float response){
    delta[1]=0;
    auto local=volume.project(delta);
    const float length_squared=static_cast<float>(square(local));
    if(static_cast<long double>(length_squared)-1>=1.e-6f)return std::nullopt;
    if(length_squared<=epsilon)local=normalize_contact_direction(volume.project(position_delta,false));
    else{
        const auto length=std::sqrt(static_cast<long double>(length_squared));
        for(unsigned i=0;i!=3;++i)local[i]=static_cast<float>(local[i]*response*(1-length)/length);
    }
    return volume.unproject(local);
}
template<class Movement>bool has_path(const Movement& movement){
    return movement.path&&!movement.path->empty();
}
template<class Movement>auto& target_job(Movement& movement){
    return *movement.path->at(movement.fielde4);
}
template<class Movement>void advance_path_cursor(Movement& movement,float time){ //005653C0.
    auto& jobs=*movement.path;
    if(jobs.size()>0xffffu)throw std::length_error("Human movement path exceeds the native WORD cursor");
    auto previous=movement.fielde4?jobs.at(movement.fielde4-1)->words.template get<HumanVector>(0):movement.previous70;
    while(static_cast<std::size_t>(movement.fielde4)+1<jobs.size()){
        const auto point=target_job(movement).words.template get<HumanVector>(0);
        HumanVector remaining{},segment{};
        for(unsigned i=0;i!=3;++i){
            remaining[i]=static_cast<float>(point[i]-(static_cast<long double>(time)*movement.velocity30[i]+movement.position20[i]));
            segment[i]=point[i]-previous[i];
        }
        const float dot=static_cast<float>((static_cast<long double>(segment[0])*remaining[0]+
            static_cast<long double>(segment[1])*remaining[1])+static_cast<long double>(segment[2])*remaining[2]);
        if(!negative_word(dot)&&square(remaining)>.0025f)return;
        previous=point;
        ++movement.fielde4;
    }
}
template<class Movement>void apply_lateral_correction(Movement& movement,float step){ //00567C80.
    if(!movement.flag107)return;
    const auto length=square(movement.velocity30);
    if(!(square(movement.direction40)>epsilon&&length>epsilon))return;
    const auto& velocity=movement.velocity30;
    auto& correction=movement.direction40;
    const auto dot=(static_cast<long double>(velocity[2])*correction[2]+
        static_cast<long double>(velocity[1])*correction[1])+static_cast<long double>(velocity[0])*correction[0];
    if(!(dot<-epsilon))return;
    HumanVector perpendicular{},parallel{};
    const auto scale=dot/length;
    for(unsigned i=0;i!=3;++i){
        parallel[i]=static_cast<float>(scale*velocity[i]);
        perpendicular[i]=correction[i]-parallel[i];
    }
    if(square(perpendicular)/square(parallel)<.08f){
        correction[0]=static_cast<float>(static_cast<long double>(velocity[2])*step*2+correction[0]);
        correction[2]=static_cast<float>(static_cast<long double>(correction[2])-static_cast<long double>(velocity[0])*step*2);
    }
}
template<class Movement>void publish_steering(Movement& movement,float time){
    HumanVector displacement{};
    for(unsigned i=0;i!=3;++i)displacement[i]=movement.position20[i]-movement.origin50[i];
    if(square(displacement)<=epsilon){
        movement.steering.fill(0);
        movement.flag10=false;
        return;
    }
    const float scale=1.f/time;
    for(unsigned i=0;i!=3;++i)movement.steering[i]=displacement[i]*scale;
    movement.flag10=true;
}
template<class Movement>void age_path_progress(Movement& movement,std::uint32_t elapsed){ //00569530.
    const auto point=target_job(movement).words.template get<HumanVector>(0);
    HumanVector displacement{};
    for(unsigned i=0;i!=3;++i)displacement[i]=movement.position20[i]-point[i];
    const auto distance=(static_cast<long double>(displacement[1])*displacement[1]+
        static_cast<long double>(displacement[0])*displacement[0])+static_cast<long double>(displacement[2])*displacement[2];
    if(distance<movement.path_distance){
        movement.path_distance=static_cast<float>(distance);
        movement.path_deadline=2000;
        return;
    }
    const auto remaining=static_cast<std::uint16_t>(movement.path_deadline-elapsed);
    movement.path_deadline=remaining&0x8000u?2000:remaining;
}
template<class Movement>void apply_standing_pair(Movement& current,const Movement& other){ //00566130, ordinary radius.
    auto separation=difference(current.position20,other.position20);
    if(std::abs(separation[1])>2.f)return;
    separation[1]=0;
    const auto gap=square(separation)-contact_square;
    if(gap<=epsilon){
        const auto distance=gap+contact_square;
        const auto speed=square(current.velocity30);
        if(!(distance>epsilon&&speed>epsilon))return;
        const auto projection=dot(separation,current.velocity30)/speed;
        HumanVector perpendicular{};
        for(unsigned i=0;i!=3;++i)
            perpendicular[i]=static_cast<float>(separation[i]-projection*current.velocity30[i]);
        const auto perpendicular_square=square(perpendicular);
        const auto& direction=perpendicular_square<=epsilon?separation:perpendicular;
        const auto length=std::sqrt(perpendicular_square<=epsilon?distance:perpendicular_square);
        const auto scale=(perpendicular_square<=epsilon?.55f:.75f)/length*(contact_radius-length);
        auto adjustment=direction;
        for(unsigned i=0;i!=3;++i)adjustment[i]=static_cast<float>(adjustment[i]*scale);
        add_correction(current.direction40,adjustment);
        return;
    }
    if(!other.flag108)return;
    auto relative=difference(current.velocity30,other.velocity30);
    relative[1]=0;
    const float speed=static_cast<float>(square(relative));
    if(speed<epsilon)return;
    const float projection=static_cast<float>(dot(separation,relative));
    const float twice=projection*2;
    if(!negative_word(twice))return;
    HumanVector closest{};
    const auto ratio=static_cast<long double>(projection)/speed;
    for(unsigned i=0;i!=3;++i)closest[i]=static_cast<float>(separation[i]-ratio*relative[i]);
    if(square(closest)>contact_square)return;
    const float discriminant=static_cast<float>(static_cast<long double>(twice)*twice-4*speed*gap);
    if(negative_word(discriminant))return;
    const auto root=std::sqrt(static_cast<long double>(discriminant));
    float time=static_cast<float>((root-twice)/(2.L*speed));
    const float near_time=static_cast<float>((-twice-root)/(2.L*speed));
    if(!negative_word(near_time)&&(negative_word(time)||near_time<time))time=near_time;
    if(negative_word(time)||!(time<=1))return;
    HumanVector offset{};
    for(unsigned i=0;i!=3;++i){
        offset[i]=static_cast<float>(static_cast<long double>(time)*relative[i]);
        separation[i]+=offset[i];
    }
    HumanVector tangent{{relative[2],0,-relative[0],current.direction40[3]}};
    const auto side=static_cast<float>(dot(separation,tangent)/speed);
    const auto scale=static_cast<long double>(fast_inverse_root(speed))*
        (1-square(offset)*1.5625f)*(negative_word(side)?-.4f:.4f)*(1-time)*.125L;
    for(unsigned i=0;i!=3;++i)tangent[i]=static_cast<float>(tangent[i]*scale);
    add_correction(current.direction40,tangent);
}
template<class Movement,class Constrain>
void separate_standing_targets(Movement& current,Movement& other,Constrain constrain){ //00566E50, ordinary radius.
    if(current.path&&current.path->empty())return;
    auto separation=difference(current.target60,other.target60);
    if(std::abs(separation[1])>2.f)return;
    separation[1]=0;
    const auto distance=square(separation);
    if(distance-contact_square>=1.e-6f)return;
    if(distance>epsilon){
        const auto length=std::sqrt(distance);
        for(unsigned i=0;i!=3;++i)separation[i]=static_cast<float>(separation[i]*(.75f/length)*(contact_radius-length));
    }else{
        separation=normalize_contact_direction(difference(current.position20,other.position20));
        for(unsigned i=0;i!=3;++i)separation[i]*=contact_radius;
    }
    for(unsigned i=0;i!=3;++i)current.target60[i]+=separation[i];
    if(current.target_record)constrain(current);
    if(!other.flag107||!has_path(other))return;
    for(unsigned i=0;i!=3;++i)other.target60[i]-=separation[i];
    if(other.target_record)constrain(other);
}
template<class Movement,class Constrain>
void apply_prone_pair(Movement& current,Movement& other,Constrain constrain){ //00566130/00566E50 posture 2 of the peer.
    const ContactVolume volume{{other.side90,other.upA0,other.forwardB0},
        {{other.radiiD0[0]+.4f,other.radiiD0[1]+.4f,other.radiiD0[2]+.4f}}};
    const auto position_delta=difference(current.position20,other.position20);
    if(std::abs(position_delta[1])<=2.f){
        const auto velocity=difference(current.velocity30,other.velocity30);
        if(const auto correction=ellipsoid_correction(volume,position_delta,velocity,{},other.flag108,.125f))
            add_correction(current.direction40,*correction);
    }
    if(current.path&&current.path->empty())return;
    const auto target_delta=difference(current.target60,other.target60);
    if(std::abs(target_delta[1])>2.f)return;
    if(const auto correction=ellipsoid_target(volume,target_delta,position_delta,other.flag107?.75f:1.f)){
        for(unsigned i=0;i!=3;++i)current.target60[i]+=(*correction)[i];
        if(current.target_record)constrain(current);
        if(other.flag107&&has_path(other)){
            for(unsigned i=0;i!=3;++i)other.target60[i]-=(*correction)[i];
            if(other.target_record)constrain(other);
        }
    }
}
} //namespace human_scene_movement

//008AADD0's process scratch. The caller supplies Scene2 active20 and the actual
//world receivers. Pointers remain borrowed throughout the synchronous solve.
template<class Actor,class Human,class Car>class HumanSceneMovementProcess {
public:
    template<class World>void advance(std::uint32_t elapsed,const std::vector<Actor*>& active,World& world){ //005647C0.
        if((world.mode_flags()&7u)!=7u&&!world.human_remote_ai_enabled())return;
        humans_.clear();
        cars_.clear();
        const auto count=active.size();
        for(std::size_t n=0;n<count;++n){
            if(n>=active.size()||!active[n])throw std::logic_error("Scene movement requires the actual active actor");
            auto& actor=*active[n];
            if(actor.base().type()==3)gather_human(require_family<Human>(actor),world);
            else if(actor.base().type()==4){
                auto& car=require_family<Car>(actor);
                cars_.push_back({&car,world.car_movement_radii(car)});
            }
        }
        if(humans_.empty())return;
        const auto seconds=static_cast<long double>(human_frame_signed_word(elapsed))*.0010000000474974513f;
        const float time=static_cast<float>(seconds),step=static_cast<float>(seconds*.125L);
        for(auto n=humans_.size();n;){
            auto*& human=humans_[--n];
            if(prepare_or_exclude(*human,time,world))human=nullptr;
        }
        reverse([&](Human& human){prepare_constraints(human,time,world);});
        for(unsigned substep=0;substep!=8;++substep){
            reverse([&](Human& human){
                advance_position(human,step,world);
                auto& correction=movement(human).direction40;
                correction[0]=correction[1]=correction[2]=0;
            });
            reverse([&](Human& human){apply_constraints(human,step,world);});
            reverse([&](Human& human){
                auto& value=movement(human);
                human_scene_movement::apply_lateral_correction(value,step);
                for(unsigned i=0;i!=3;++i)value.position20[i]+=value.direction40[i];
                if(value.flag107)apply_floor(human,false,world);
            });
        }
        reverse([&](Human& human){
            auto& value=movement(human);
            if(human_scene_movement::has_path(value)){
                auto& job=human_scene_movement::target_job(value).words;
                const auto point=job.template get<HumanVector>(0);
                HumanVector displacement{};
                for(unsigned i=0;i!=3;++i)displacement[i]=value.target60[i]-point[i];
                if(human_scene_movement::square(displacement)>human_scene_movement::epsilon)
                    job.set(0,value.target60);
            }
            human_scene_movement::publish_steering(value,time);
            if(human_scene_movement::has_path(value))human_scene_movement::age_path_progress(value,elapsed);
        });
    }
private:
    struct CarSample{Car* actor;HumanVector radii;};
    std::vector<Human*> humans_;
    std::vector<CarSample> cars_;

    template<class Family>static Family& require_family(Actor& actor){
        auto* result=dynamic_cast<Family*>(&actor);
        if(!result)throw std::logic_error("Scene movement actor has a different actual family");
        return *result;
    }
    static auto& movement(Human& human){
        if(!human.scene_movement||human.scene_movement->owner!=&human)
            throw std::logic_error("Scene movement requires actual Human+2C4");
        return *human.scene_movement;
    }
    static auto& state(Human& human){
        if(!human.loading_state)throw std::logic_error("Scene movement requires actual Human B0");
        return *human.loading_state;
    }
    template<class Visit>void reverse(Visit visit){
        for(auto n=humans_.size();n;)if(auto* human=humans_[--n])visit(*human);
    }
    template<class World>void gather_human(Human& human,World& world){
        auto& value=movement(human);
        const bool action=state(human).words().template get<std::int32_t>(0x60)!=-1; //005A45C0.
        value.flag10a=(human_frame_signed_word(human.properties.stats_ec[0])<1?1u:0u)|
            (world.human_unavailable(human)?2u:0u)|(action?8u:0u)|(human.field160?0x10u:0u)|
            (world.human_movement_animation_blocked(human)?0x20u:0u); //005AC2B0.
        if(value.flag10a&0x3fu)value.flag10=false;
        else humans_.push_back(&human);
    }
    static bool car_near(const HumanVector& human,const HumanVector& position){
        const auto x=static_cast<long double>(human[0])-position[0],z=static_cast<long double>(human[2])-position[2];
        return std::abs(static_cast<long double>(human[1])-position[1])<=2.L&&x*x+z*z<=49.f;
    }
    template<class World>bool prepare_or_exclude(Human& human,float time,World& world){ //00564EF0.
        auto& b0=state(human);
        const auto kind=b0.words().template get<std::uint32_t>(0x48);
        if(!world.human_movement_floor(human)&&kind!=1){
            const auto position=world.movement_model_position(human);
            if(!world.movement_neighbour_ids(human).empty())return false;
            for(auto n=cars_.size();n;)if(car_near(position,world.movement_model_position(*cars_[--n].actor)))return false;
        }
        auto& value=movement(human);
        value.path=b0.scene_navigation_jobs();
        value.fielde4=0;
        sample_motion(human,value,world);
        if(!human_scene_movement::has_path(value)){
            value.velocity30[0]=value.velocity30[1]=value.velocity30[2]=0;
            value.flag104=true;
            value.target60=value.previous80=value.previous70=value.position20;
        }else{
            human_scene_movement::advance_path_cursor(value,time);
            value.target60=human_scene_movement::target_job(value).words.template get<HumanVector>(0);
            for(unsigned i=0;i!=3;++i)value.velocity30[i]=value.target60[i]-value.position20[i];
            value.velocity30[1]=0;
            if(std::isnan(value.previous80[0])||std::isnan(value.previous70[0]))
                value.previous80=value.previous70=value.position20;
            if(value.flag104){
                value.flag104=false;
                value.previous70=value.previous80;
            }
            value.previous80=value.path->front()->words.template get<HumanVector>(0);
        }
        const auto length=human_scene_movement::square(value.velocity30);
        if(length<=human_scene_movement::epsilon){
            value.velocity30[0]=value.velocity30[1]=value.velocity30[2]=0;
            value.steering.fill(0);
            value.flag10=false;
        }else{
            const auto scale=static_cast<long double>(human_scene_movement::fast_inverse_root(static_cast<float>(length)))*value.speedDC;
            for(unsigned i=0;i!=3;++i)value.steering[i]=value.velocity30[i]=static_cast<float>(scale*value.velocity30[i]);
            value.flag10=true;
        }
        if(kind==1||kind==5)value.floor_record=nullptr;
        return true;
    }
    template<class Movement,class World>static void sample_motion(Human& human,Movement& value,World& world){
        value.position20=world.movement_model_position(human);
        value.speedDC=world.human_movement_path_speed(human);
        if(state(human).words().template get<std::uint32_t>(0x50)&0xf8u)value.velocity30=human.activation.vector1b0;
        else value.velocity30[0]=value.velocity30[1]=value.velocity30[2]=0;
    }
    template<class World>void prepare_constraints(Human& human,float time,World& world){ //005656E0.
        auto& value=movement(human);
        auto& b0=state(human);
        const auto kind=b0.words().template get<std::uint32_t>(0x48);
        sample_motion(human,value,world);
        value.target_record=nullptr;
        value.origin50=value.position20;
        value.path=b0.scene_navigation_jobs();
        value.fielde4=0;
        if(!human_scene_movement::has_path(value)){
            value.flag104=true;
            value.field100=0xffffffffu;
            value.target60=value.previous80=value.previous70=value.position20;
            if(kind==1){
                value.floor_record=nullptr;
                for(unsigned i=0;i!=3;++i)value.target60[i]=static_cast<float>(
                    static_cast<long double>(time)*value.velocity30[i]+value.position20[i]);
            }else if(kind==5){
                value.floor_record=nullptr;
                value.velocity30[0]=value.velocity30[1]=value.velocity30[2]=0;
            }else if(human_scene_movement::square(value.velocity30)>human_scene_movement::epsilon){
                if(value.floor_record)world.refresh_movement_floor(human,nullptr);
                else value.floor_record=world.nearest_movement_record(value.position20);
            }
        }else{
            auto* previous_floor=value.floor_record;
            const auto& first=value.path->front()->words;
            value.floor_record=world.movement_table_record(first.template get<std::uint16_t>(0x20));
            if(value.floor_record)world.refresh_movement_floor(human,previous_floor);
            if(std::isnan(value.previous80[0])||std::isnan(value.previous70[0]))
                value.previous80=value.previous70=value.position20;
            if(value.flag104){
                value.flag104=false;
                value.path_deadline=2000;
                value.path_distance=100000000.f;
                value.previous70=value.previous80;
                if(value.field100>=value.path->size())adjust_path_prefix(human,world);
            }
            value.previous80=value.path->front()->words.template get<HumanVector>(0);
            const auto& job=human_scene_movement::target_job(value).words;
            value.target60=job.template get<HumanVector>(0);
            value.target_record=world.movement_table_record(job.template get<std::uint16_t>(0x20));
        }
        value.projected_floor=nullptr;
        const auto posture=world.human_movement_posture(human);
        if(posture==2)world.prepare_movement_prone_axes(human);
        value.flag105=kind!=1;
        value.flag106=kind!=5&&posture!=2&&value.flag105&&b0.scene_movement_enabled();
        value.flag107=value.flag105&&(value.flag106||human_scene_movement::has_path(value));
        value.flag108=(world.mode_flags()&7u)==7u||!world.human_remote_ai_enabled()||kind!=1;
        if(!value.flag106)value.velocity30[0]=value.velocity30[1]=value.velocity30[2]=0;
    }
    template<class World>bool path_point_blocked(Human& human,const HumanVector& position,World& world){ //0056BE90.
        namespace math=human_scene_movement;
        const auto& neighbours=world.movement_neighbour_ids(human);
        for(auto n=neighbours.size();n;){
            auto* actor=world.movement_neighbour(neighbours[--n]);
            if(!actor)continue;
            auto& other=movement(*actor);
            if((other.flag10a&0x37u)||(actor->loading_state&&actor->loading_state->scene_movement_interaction()))continue;
            if(human_script_suspended(*actor)){other.flag10a|=4u;continue;}
            const auto delta=math::difference(position,other.position20);
            if(std::abs(delta[1])>2.f)continue;
            if(world.human_movement_posture(*actor)==2){
                const auto x=math::dot(delta,other.side90)/(other.radiiD0[0]+.4f);
                const auto z=math::dot(delta,other.forwardB0)/(other.radiiD0[2]+.4f);
                if(x*x+z*z<1)return true;
            }
            const auto x=static_cast<long double>(delta[0]),z=static_cast<long double>(delta[2]);
            if(x*x+z*z<math::contact_square)return true;
        }
        for(auto n=cars_.size();n;){
            const auto& car=cars_[--n];
            if(human.field160==car.actor||!(world.car_movement_speed(*car.actor)<=math::epsilon))continue;
            const auto delta=math::difference(position,world.movement_model_position(*car.actor));
            if(std::abs(delta[1])>2.f||math::square(delta)>49.f)continue;
            const auto axes=world.car_movement_axes(*car.actor);
            const auto x=math::dot(delta,axes[0])/(car.radii[0]+.4f),z=math::dot(delta,axes[2])/(car.radii[2]+.4f);
            if(x*x+z*z<1)return true;
        }
        return false;
    }
    template<class World>void adjust_path_prefix(Human& human,World& world){ //0056A0F0 initial job retirement.
        auto& value=movement(human);
        auto& path=*value.path;
        auto start=static_cast<std::size_t>(value.fielde4);
        while(start<path.size()&&path_point_blocked(human,path[start]->words.template get<HumanVector>(0),world))++start;
        if(start==path.size())--start; //The final job remains owned even if all current destinations overlap.
        for(std::size_t n=0;n<start;++n){
            path.front().reset();
            path.erase(path.begin());
        }
        value.fielde4=0;
        value.flag109=false;
        const auto target=path.front()->words.template get<HumanVector>(0);
        const auto x=static_cast<long double>(target[0])-value.position20[0],z=static_cast<long double>(target[2])-value.position20[2];
        if(!(x*x+z*z<human_scene_movement::epsilon))world.replan_movement_path(human); //0056A8E0's nonzero route.
    }
    template<class World>static void advance_position(Human& human,float step,World& world){ //00565F30.
        auto& value=movement(human);
        if(!human_scene_movement::has_path(value)&&value.flag106){
            const float inverse=1.f/step;
            for(unsigned i=0;i!=3;++i)value.velocity30[i]=inverse*value.direction40[i];
        }else{
            if(human_scene_movement::has_path(value)){
                human_scene_movement::advance_path_cursor(value,step);
                const auto& job=human_scene_movement::target_job(value).words;
                value.target60=job.template get<HumanVector>(0);
                value.target_record=world.movement_table_record(job.template get<std::uint16_t>(0x20));
                const float x=value.target60[0]-value.position20[0],z=value.target60[2]-value.position20[2];
                const auto square=static_cast<long double>(x)*x+static_cast<long double>(z)*z;
                if(square>1.e-6f){
                    const auto scale=static_cast<long double>(human_scene_movement::fast_inverse_root(static_cast<float>(square)))*value.speedDC;
                    value.velocity30[0]=static_cast<float>(scale*x);value.velocity30[1]=0;
                    value.velocity30[2]=static_cast<float>(scale*z);
                }
            }
            for(unsigned i=0;i!=3;++i)value.position20[i]=static_cast<float>(
                static_cast<long double>(step)*value.velocity30[i]+value.position20[i]);
        }
        if(world.human_movement_posture(human)==2)value.centreC0=value.position20;
    }
    template<class World>void apply_constraints(Human& human,float step,World& world){
        auto& value=movement(human);
        const auto constrain_target=[&](auto& target){
            (void)human_scene_movement::constrain_point(target.target60,target.target_record,world);
        };
        const auto& neighbours=world.movement_neighbour_ids(human);
        for(auto n=neighbours.size();n;){
            auto* other=world.movement_neighbour(neighbours[--n]);
            if(!other)continue;
            auto& peer=movement(*other);
            if(peer.flag10a&0x3fu)continue;
            if(human_script_suspended(*other))peer.flag10a|=4u;
            else if(value.flag107){
                if(world.human_movement_posture(*other)==2)
                    human_scene_movement::apply_prone_pair(value,peer,constrain_target);
                else{
                    human_scene_movement::apply_standing_pair(value,peer);
                    human_scene_movement::separate_standing_targets(value,peer,constrain_target);
                }
            }
        }
        for(auto n=cars_.size();n;){
            const auto& car=cars_[--n];
            const auto position=world.movement_model_position(*car.actor);
            const auto origin=world.movement_model_position(human);
            if(value.flag107&&car_near(origin,position))
                apply_car_pair(human,car,step,world);
        }
        if(value.flag107)apply_floor(human,true,world);
    }
    template<class World>static void apply_car_pair(Human& human,const CarSample& car,float step,World& world){
        namespace math=human_scene_movement;
        auto& value=movement(human);
        const auto contact_volume=[&]{
            return math::ContactVolume{world.car_movement_axes(*car.actor),
                {{car.radii[0]+.4f,car.radii[1]+.4f,car.radii[2]+.4f}}};
        };
        const auto car_position=world.movement_model_position(*car.actor);
        const auto volume=contact_volume();
        const auto carried=world.car_movement_velocity(*car.actor);
        const auto relative=math::difference(value.velocity30,carried);
        const auto position_delta=math::difference(value.position20,car_position);
        if(const auto correction=math::ellipsoid_correction(volume,position_delta,relative,carried,true,step)) //00568580.
            math::add_correction(value.direction40,*correction);
        if(value.path&&value.path->empty())return; //00568DC0.
        const auto target_position=world.movement_model_position(*car.actor);
        const auto target_volume=contact_volume();
        const auto delta=math::difference(value.target60,target_position);
        if(const auto correction=math::ellipsoid_target(target_volume,delta,position_delta,1.f)){
            for(unsigned i=0;i!=3;++i)value.target60[i]=static_cast<float>(static_cast<long double>((*correction)[i])*1.1f+value.target60[i]);
            if(value.target_record)(void)math::constrain_point(value.target60,value.target_record,world);
        }
    }
    template<class World>static void apply_floor(Human& human,bool accumulate,World& world){ //00567AB0/00567BA0.
        auto& value=movement(human);
        if(!value.floor_record)value.floor_record=world.nearest_movement_record(value.position20);
        if(!value.floor_record)return;
        auto corrected=value.position20;
        auto* projected=value.floor_record;
        const bool inside=human_scene_movement::constrain_point(corrected,projected,world);
        value.projected_floor=projected;
        if(inside)return;
        if(accumulate)human_scene_movement::add_correction(value.direction40,
            human_scene_movement::difference(corrected,value.position20));
        else value.position20=corrected;
    }
};
} //namespace hd2::game
#endif
