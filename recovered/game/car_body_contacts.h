#ifndef HD2_GAME_CAR_BODY_CONTACTS_H
#define HD2_GAME_CAR_BODY_CONTACTS_H

#include "game/car_collision_parts.h"
#include "game/car_contact_callbacks.h"
#include "game/car_definition_tables.h"
#include "game/dynamic_collision_exclusions.h"

namespace hd2::game {

struct CarBodyContactRecord {CarContactVector point{};std::uint32_t surface=0;};
struct CarBodyContactState {
    CarContactVector correction{},other_correction{},maximum_impulse{};
    float maximum_magnitude=0,speed=0;
    bool moving_primitive=false,override_material=false;
    float friction=0,restitution=0;
    std::vector<CarBodyContactRecord> records;
};

//00514D70's BODYCOL traversal/response. The caller supplies CURRENT velocities
//and the native cached COM/inertia matrices (updated by0050DE60 AFTER this
//pass), old390 and old-up, and the SAME descriptor exclusions/callbacks. This
//does not decide the solver's subsequent pose rollback/retry.
template<class Frames,class Queries,class Object,class Callbacks,class Receivers>
bool car_body_contacts(Frames& frames,typename Frames::Frame model,
        const std::vector<std::unique_ptr<typename CarCollisionParts<Frames>::Box>>& roots,
        CarContactState& motion,CarBodyContactState& output,const CarMaterialRegistry& materials,
        Queries& queries,DynamicCollisionExclusions<Object>& exclusions,const std::vector<Object*>& excluded,
        Callbacks& callbacks,Receivers& receivers,const typename Frames::Matrix& before,
        const typename Frames::Matrix& cached_inverse,
        const typename Frames::Vector& old_up,const typename Frames::Vector& up,
        std::uint32_t milliseconds,std::uint32_t layers,std::uint32_t flags,
        std::uint32_t features,bool audible,bool gear){
    using Vector=typename Frames::Vector;
    using Box=typename CarCollisionParts<Frames>::Box;using Point=typename CarCollisionParts<Frames>::Point;
    if(roots.empty())return false;
    (void)milliseconds; //Native forwards this word through recursion without reading it.
    const auto dot=[](const Vector& a,const Vector& b){return (static_cast<long double>(a[0])*b[0]+
        static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    const auto cross=[](const Vector& a,const Vector& b){Vector result{};for(unsigned axis=0;axis!=3;++axis){
        const auto n=(axis+1)%3,l=(axis+2)%3;result[axis]=static_cast<float>(static_cast<long double>(a[n])*b[l]-static_cast<long double>(a[l])*b[n]);}return result;};
    const auto length=[&](const Vector& v){const float squared=static_cast<float>(dot(v,v));
        std::int32_t bits;std::memcpy(&bits,&squared,4);return bits<0x322bcc77?0.f:
            std::abs(static_cast<long double>(squared)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(squared)));};
    const auto normalize=[&](Vector v){const float square=static_cast<float>(dot(v,v));
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Car body contact direction");
        if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f){
            if(square>=9.99999993922529e-09f){const auto scale=1/std::sqrt(static_cast<long double>(square));for(unsigned axis=0;axis!=3;++axis)v[axis]=static_cast<float>(v[axis]*scale);}
            else{const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.f:1.f;}
        }return v;};
    const auto current=frames.car_world_matrix(model),inverse=frames.car_inverse_matrix(current);
    auto priority=normalize(motion.linear410);
    for(unsigned axis=0;axis!=3;++axis)priority[axis]=static_cast<float>(static_cast<long double>(priority[axis])*1000.f+motion.centre1b0[axis]);
    //Runtime2D0 is the cached inverse390, not the newly calculated inverse.
    priority=frames.car_transform(frames.car_transform(priority,cached_inverse),current);
    struct BoxEntry {const Box* box;Vector start{},displacement{};float key;};
    std::vector<BoxEntry> boxes;
    for(const auto& owner:roots){const auto& box=*owner;
        if(box.reference&&!frames.car_frame_visible(box.reference))continue;
        Vector start{},finish{};
        if(!box.reference){start=frames.car_transform(box.position,before);finish=frames.car_transform(box.position,current);}
        else{finish=frames.car_transform(box.position,frames.car_world_matrix(box.reference));start=frames.car_transform(frames.car_transform(finish,inverse),before);}
        Vector delta{},distance{};for(unsigned axis=0;axis!=3;++axis){delta[axis]=finish[axis]-start[axis];distance[axis]=priority[axis]-start[axis];}
        boxes.push_back({&box,start,delta,length(distance)-box.radius});
    }
    //Original inserts equal keys before older entries into a descending array,
    //then pops backwards: ascending keys, original encounter order on ties.
    std::stable_sort(boxes.begin(),boxes.end(),[](const auto& a,const auto& b){return a.key<b.key;});
    auto query=[&](auto&& operation){
        exclusions.many(excluded.data(),excluded.size());
        struct Clear {DynamicCollisionExclusions<Object>& value;~Clear(){value.clear();}}clear{exclusions};
        return operation();
    };
    auto angular=[&](Vector value){frames.car_transform_in_place(value,motion.matrix350);
        for(unsigned axis=0;axis!=3;++axis)value[axis]*=motion.inertia220[axis];
        frames.car_transform_in_place(value,motion.matrix310);return value;};
    auto apply=[&](const Vector& impulse,const Vector& offset){
        const auto change=angular(cross(offset,impulse));const float reciprocal=1.f/motion.mass918;
        for(unsigned axis=0;axis!=3;++axis){motion.linear410[axis]+=impulse[axis]*reciprocal;motion.angular210[axis]-=change[axis];}
    };
    auto maximum=[&](const Vector& impulse){const auto magnitude=length(impulse);
        if(output.maximum_magnitude<magnitude){output.maximum_magnitude=magnitude;for(unsigned axis=0;axis!=3;++axis)output.maximum_impulse[axis]=impulse[axis];}
        return magnitude;};
    auto save_correction=[&](Vector& destination,const Vector& value){if(dot(destination,destination)<dot(value,value))
        for(unsigned axis=0;axis!=3;++axis)destination[axis]=value[axis];};
    bool collided=false;
    for(const auto& entry:boxes){const auto& box=*entry.box;
        const bool broad=query([&]{
            if(!box.type_b)return bool(queries.sweep(entry.start,entry.displacement,box.radius,layers,flags));
            auto bounds=box.bounds;
            if(box.reference){const auto reference=frames.car_world_matrix(box.reference);
                bounds.minimum=frames.car_transform(frames.car_transform(bounds.minimum,reference),inverse);
                bounds.maximum=frames.car_transform(frames.car_transform(bounds.maximum,reference),inverse);}
            return queries.box(bounds,before,current,entry.displacement,old_up,up,layers,flags);
        });
        if(!broad)continue;
        struct PointEntry {const Point* point;Vector start{},displacement{};float key;};
        std::vector<PointEntry> points;points.reserve(box.points.size());
        for(const auto& point:box.points){Vector start{},finish{};
            if(!box.reference){start=frames.car_transform(point.position,before);finish=frames.car_transform(point.position,current);}
            else{finish=frames.car_transform(point.position,frames.car_world_matrix(box.reference));start=frames.car_transform(frames.car_transform(finish,inverse),before);}
            Vector delta{},distance{};for(unsigned axis=0;axis!=3;++axis){delta[axis]=finish[axis]-start[axis];distance[axis]=priority[axis]-start[axis];}
            points.push_back({&point,start,delta,length(distance)-point.radius});
        }
        std::stable_sort(points.begin(),points.end(),[](const auto& a,const auto& b){return a.key<b.key;});
        for(const auto& point:points){
            auto hit=query([&]{return queries.sweep(point.start,point.displacement,point.point->radius,layers,flags);});
            if(!hit)continue;
            auto* contact=&hit->contact;
            if(callbacks.contact6a8)contact=callbacks.contact6a8(*contact);
            if(!contact)continue;
            if(contact!=&hit->contact)throw std::runtime_error("Car script filter replaced its actual collision descriptor");
            if(hit->flags&0x10000u)output.moving_primitive=true;
            const auto actor=(contact->flags&0x40u)?contact->actor:decltype(contact->actor){};
            const auto normal=hit->normal;const auto radius=point.point->radius;
            Vector position{},offset{};
            for(unsigned axis=0;axis!=3;++axis){position[axis]=static_cast<float>(-static_cast<long double>(radius)*normal[axis]+point.start[axis]);offset[axis]=position[axis]-motion.centre1b0[axis];}
            auto velocity=motion.linear410;const auto rotation=cross(motion.angular210,offset);
            for(unsigned axis=0;axis!=3;++axis)velocity[axis]-=rotation[axis];
            if(0<=dot(velocity,normal))continue;
            Vector correction{},hit_surface{},from_hit{};
            for(unsigned axis=0;axis!=3;++axis){hit_surface[axis]=static_cast<float>(static_cast<long double>(hit->position[axis])-static_cast<long double>(normal[axis])*radius);from_hit[axis]=motion.centre1b0[axis]-hit->position[axis];}
            const float plane=static_cast<float>(dot(normal,point.start));
            const float surface_plane=static_cast<float>(dot(hit_surface,normal));
            const auto penetration=static_cast<long double>(radius)-(static_cast<long double>(plane)-surface_plane);
            if(.05000000074505806f<penetration&&0<=dot(from_hit,normal))
                for(unsigned axis=0;axis!=3;++axis)correction[axis]=static_cast<float>(normal[axis]*penetration);
            bool notifications=true,filtered=false;
            if(actor){if(!callbacks.filter68c)throw std::runtime_error("Car body collision has no actual actor filter");filtered=callbacks.filter68c(*contact,notifications);}
            if(actor&&!filtered){
                if(!callbacks.response688||!callbacks.actor680)throw std::runtime_error("Car dynamic body collision has no actual response receivers");
                CarContactResponse other{};if(!callbacks.response688(*contact,position,normal,other))continue;
                Vector relative{},impact_point=hit_surface;
                for(unsigned axis=0;axis!=3;++axis)relative[axis]=velocity[axis]-other.velocity[axis];
                float numerator=static_cast<float>(dot(relative,normal));if(numerator>0)numerator*=.009999999776482582f;
                const auto own=angular(cross(offset,normal));
                const auto own_cross=cross(own,offset),other_cross=cross(other.angular_response,other.offset);
                float denominator=static_cast<float>(dot(own_cross,normal)+dot(other_cross,normal)+1.L/motion.mass918+1.L/other.mass);
                if(std::abs(denominator)<static_cast<float>(9.999999974752427e-7))denominator=9.999999974752427e-7f;
                const float amount=numerator/denominator;Vector impulse{};
                for(unsigned axis=0;axis!=3;++axis)impulse[axis]=normal[axis]*amount;
                callbacks.actor680(*contact,impact_point,impulse,normal,motion.mass918);
                for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=other.coefficient;
                apply(impulse,offset);const auto magnitude=maximum(impulse);
                if(notifications){if(features&0x8000u)receivers.deform(position,normal,radius,output.maximum_magnitude);if(features&0x4000u)receivers.damage(output.maximum_magnitude);}
                if(callbacks.passenger690)callbacks.passenger690(impulse,position,true,nullptr);
                if(audible)receivers.sound(magnitude,gear);
                if(other.enabled)save_correction(output.other_correction,correction);
                collided=true;continue;
            }
            const auto& material=materials.responses().at(materials.indices().at(contact->surface));
            const auto restitution=output.override_material?output.restitution:material.field1c;
            const float numerator=static_cast<float>(-static_cast<long double>(restitution)*dot(velocity,normal));
            const auto own=angular(cross(offset,normal));
            float denominator=static_cast<float>(dot(cross(own,offset),normal)+1.L/motion.mass918);
            if(std::abs(denominator)<static_cast<float>(9.999999974752427e-7))denominator=9.999999974752427e-7f;
            const float amount=numerator/denominator;Vector impulse{};for(unsigned axis=0;axis!=3;++axis)impulse[axis]=normal[axis]*amount;
            apply(impulse,offset);const auto magnitude=maximum(impulse);
            if(notifications){if(features&0x8000u)receivers.deform(position,normal,radius,output.maximum_magnitude);if(features&0x4000u)receivers.damage(output.maximum_magnitude);}
            if(callbacks.passenger690)callbacks.passenger690(impulse,position,false,contact);
            if(audible)receivers.sound(magnitude,gear);
            if(5.f<magnitude/motion.mass918||3.f<output.speed)output.records.push_back({position,contact->surface});
            if(dot(output.correction,output.correction)<dot(correction,correction)){
                bool enabled=true;
                if(actor){CarContactResponse other{};if(!callbacks.response688)throw std::runtime_error("Car correction has no actual actor response");
                    (void)callbacks.response688(*contact,position,normal,other);enabled=other.enabled;}
                if(enabled)save_correction(output.correction,correction);
            }
            if(filtered&&callbacks.human684)callbacks.human684(*contact,position,motion.linear410,normal);
            auto tangent=normalize(cross(cross(normalize(velocity),normal),normal));
            const auto friction=output.override_material?output.friction:material.field18;
            const float friction_amount=length(impulse)*friction;
            for(unsigned axis=0;axis!=3;++axis)tangent[axis]*=friction_amount;
            const auto sine=std::sin(static_cast<long double>(frames.car_vector_angle(velocity,normal)));
            //00517027 retains the sine-scaled impulse in x87 through the
            //linear division and angular cross product; no intermediate FLOAT
            //vector store occurs. Y/Z linear increments spill before addition.
            std::array<long double,3> friction_impulse{};Vector turn{};
            for(unsigned axis=0;axis!=3;++axis)friction_impulse[axis]=tangent[axis]*sine;
            const auto reciprocal=1.L/motion.mass918;
            for(unsigned axis=0;axis!=3;++axis){
                auto change=friction_impulse[axis]*reciprocal;if(axis)change=static_cast<float>(change);
                motion.linear410[axis]=static_cast<float>(change+motion.linear410[axis]);
                const auto n=(axis+1)%3,l=(axis+2)%3;
                turn[axis]=static_cast<float>(friction_impulse[n]*offset[l]-friction_impulse[l]*offset[n]);
            }
            turn=angular(turn);for(unsigned axis=0;axis!=3;++axis)motion.angular210[axis]+=turn[axis];
            collided=true;
        }
        collided=car_body_contacts(frames,model,box.children,motion,output,materials,queries,exclusions,excluded,
            callbacks,receivers,before,cached_inverse,old_up,up,milliseconds,layers,flags,features,audible,gear)||collided;
    }
    return collided;
}
} //namespace hd2::game
#endif
