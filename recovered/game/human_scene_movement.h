#ifndef HD2_GAME_HUMAN_SCENE_MOVEMENT_H
#define HD2_GAME_HUMAN_SCENE_MOVEMENT_H

#include "game/human_scene_movement_detour.h"
#include "game/scene_actor_frame.h"

namespace hd2::game {

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
    human_scene_movement::DetourSearch<Actor> detours_;

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
    template<class World>void adjust_path_prefix(Human& human,World& world){ //0056A0F0.
        namespace math=human_scene_movement;
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
        const auto& job=path.front()->words;
        const auto target=job.template get<HumanVector>(0);
        auto* destination=world.movement_table_record(job.template get<std::uint16_t>(0x20));
        detours_.run(value.position20,value.floor_record,target,destination,
            [&](const auto& from,auto& to){return probe_detour(human,from,to,world);},
            [&](auto& node){math::refresh_floor(node.position,node.floor,nullptr,world,5);});
        if(!detours_.selected_count()||math::square(math::difference(detours_.selected(0).position,target))<=math::epsilon)return;
        if(detours_.remaining_distance()<.01f){
            path.front().reset();
            path.erase(path.begin());
        }
        value.field100=static_cast<std::uint32_t>(path.size());
        auto& pool=world.movement_job_pool();
        for(auto n=detours_.selected_count();n;){
            const auto& node=detours_.selected(--n);
            if(!node.floor)break;
            const auto centre=math::record_position(*node.floor);
            auto job=pool.acquire(world.movement_table_index(*node.floor),
                std::array<float,3>{{centre[0],centre[1],centre[2]}},node.floor->flags&0xfb40u,
                node.floor->byte16,math::record_radius(*node.floor));
            job->words.set(0,node.position);
            path.insert(path.begin(),std::move(job));
        }
    }
    template<class World>std::optional<human_scene_movement::DetourObstacle<Actor>> probe_detour(Human& human,
            const human_scene_movement::DetourNode<Actor>& from,human_scene_movement::DetourNode<Actor>& to,World& world){ //0056C230.
        namespace math=human_scene_movement;
        auto destination=to.position;
        auto* floor=to.floor;
        if(!floor){
            floor=from.floor;
            math::refresh_floor(destination,floor,nullptr,world,5);
        }
        if(!floor||!math::constrain_point(destination,floor,world))return std::nullopt;
        to.floor=floor;
        auto direction=math::difference(destination,from.position);
        direction[1]=0;
        if(math::square(direction)<math::epsilon)return std::nullopt;
        float closest=100000000.f;
        math::DetourObstacle<Actor> obstacle;
        math::DetourRoots roots;
        const auto& neighbours=world.movement_neighbour_ids(human);
        for(auto n=neighbours.size();n&&closest>=math::epsilon;){
            auto* peer=world.movement_neighbour(neighbours[--n]);
            if(!peer)continue;
            auto& other=movement(*peer);
            if(other.flag10a&0x37u)continue;
            if(human_script_suspended(*peer)){other.flag10a|=4u;continue;}
            if(to.ignores_human(peer))continue;
            auto delta=math::difference(from.position,other.position20);
            if(std::abs(delta[1])>2.f)continue;
            delta[1]=0;
            if(world.human_movement_posture(*peer)==2){
                const math::ContactVolume volume{{other.side90,other.upA0,other.forwardB0},
                    {{other.radiiD0[0]+.4f,other.radiiD0[1]+.4f,other.radiiD0[2]+.4f}}};
                const auto origin=volume.project(delta),line=volume.project(direction);
                if(const auto hit=math::detour_contact(origin,line,1,closest,roots,true)){
                    const auto sides=math::ellipsoid_detour_sides(volume,other.position20,origin,line,hit->lateral,1.f);
                    obstacle.sides[0]=sides[0];obstacle.sides[1]=sides[1];
                    obstacle.human=peer;obstacle.intersection=hit->intersection;closest=hit->time;
                }
            }else if(const auto hit=math::detour_contact(delta,direction,math::contact_square,closest,roots,false)){
                obstacle.human=peer;obstacle.intersection=hit->intersection;closest=hit->time;
                for(unsigned side=0;side!=2;++side){
                    auto point=other.position20;
                    const auto scale=side?-math::contact_radius:math::contact_radius;
                    for(unsigned i=0;i!=3;++i)point[i]+=hit->lateral[i]*scale;
                    obstacle.sides[side].points[0]=point;
                    obstacle.sides[side].count=1;
                }
            }
        }
        for(auto n=cars_.size();n&&closest>=math::epsilon;){
            const auto& car=cars_[--n];
            if(car.actor==human.field160||car.actor==to.car||!(world.car_movement_speed(*car.actor)<=math::epsilon))continue;
            const auto position=world.movement_model_position(*car.actor);
            auto delta=math::difference(from.position,position);
            const auto target_delta=math::difference(destination,position);
            if((std::abs(delta[1])>2.f&&std::abs(target_delta[1])>2.f)||
               (math::square(delta)>49.f&&math::square(target_delta)>49.f))continue;
            const auto volume=car_volume(car,world);
            delta[1]=0;
            const auto origin=volume.project(delta),line=volume.project(direction);
            if(const auto hit=math::detour_contact(origin,line,1,closest,roots,true)){
                const auto sides=math::ellipsoid_detour_sides(volume,position,origin,line,hit->lateral,1.05f);
                obstacle.sides[0]=sides[0];obstacle.sides[1]=sides[1];
                obstacle.human=nullptr;obstacle.car=car.actor;obstacle.intersection=hit->intersection;closest=hit->time;
            }
        }
        return obstacle;
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
        const auto car_position=world.movement_model_position(*car.actor);
        const auto volume=car_volume(car,world);
        const auto carried=world.car_movement_velocity(*car.actor);
        const auto relative=math::difference(value.velocity30,carried);
        const auto position_delta=math::difference(value.position20,car_position);
        if(const auto correction=math::ellipsoid_correction(volume,position_delta,relative,carried,true,step)) //00568580.
            math::add_correction(value.direction40,*correction);
        if(value.path&&value.path->empty())return; //00568DC0.
        const auto target_position=world.movement_model_position(*car.actor);
        const auto target_volume=car_volume(car,world);
        const auto delta=math::difference(value.target60,target_position);
        if(const auto correction=math::ellipsoid_target(target_volume,delta,position_delta,1.f)){
            for(unsigned i=0;i!=3;++i)value.target60[i]=static_cast<float>(static_cast<long double>((*correction)[i])*1.1f+value.target60[i]);
            if(value.target_record)(void)math::constrain_point(value.target60,value.target_record,world);
        }
    }
    template<class World>static human_scene_movement::ContactVolume car_volume(const CarSample& car,World& world){
        return {world.car_movement_axes(*car.actor),{{car.radii[0]+.4f,car.radii[1]+.4f,car.radii[2]+.4f}}};
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
