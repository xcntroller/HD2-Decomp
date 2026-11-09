#ifndef HD2_GAME_ANIMATION_ACTOR_COLLISIONS_H
#define HD2_GAME_ANIMATION_ACTOR_COLLISIONS_H

#include "game/rigid_body.h"
#include <cstdio>
#include <cstdlib>

namespace hd2::game {

//Actual animation-actorE0 owned BODYCOLB records. Declaration is shared with
//the actor, keeping the descriptor owner alive until its grid entries retire.
class AnimationActorCollisionLifetime {
public:
    virtual ~AnimationActorCollisionLifetime()=default;
    virtual void rebuild()=0;
    virtual void clear()noexcept=0;
};

//005B3AC0/005B4DE0/005B4850/005B48B0's BODYCOLB loading owner.
//Each descriptor borrows the actual actor/frame; world must retire this owner
//before either actor/model or the SAME dynamic buckets. Non-dummy geometry
//uses the native driver+AC aggregate bounds, unlike rigid-body OBJ_ traversal.
template<class Frames,class Collisions,class Actor>class AnimationActorCollisions final : public AnimationActorCollisionLifetime {
public:
    using Frame=typename Frames::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Matrix=std::array<float,16>;
    using Vector=std::array<float,4>;
    struct alignas(16) Record {
        Descriptor descriptor;Matrix matrix{},inverse{};
        Record(){descriptor.matrix=&matrix;descriptor.inverse=&inverse;}
    };
    AnimationActorCollisions(Frames& frames,Collisions& collisions,Actor& actor):frames_(frames),collisions_(collisions),actor_(actor){}
    ~AnimationActorCollisions()override{clear();}
    const auto& records()const{return records_;}
    void clear()noexcept override { //005B4DC0 removes bucket before aligned record free.
        while(!records_.empty()){collisions_.remove_for_cleanup(records_.back()->descriptor);records_.pop_back();}
    }
    void rebuild()override { //005B3AC0 clears old list BEFORE searching consecutive BODYCOLB00...
        clear();const auto model=actor_.base().frame();if(!model)return;
        for(unsigned n=0;n!=1000000;++n) {
            char name[32];std::snprintf(name,sizeof(name),"BODYCOLB%02u",n);
            const auto frame=frames_.actor_find(model,name,0xffff);if(!frame)return;
            records_.reserve(records_.size()+1); //Publication cannot allocate after registering the borrowed descriptor.
            auto owner=std::make_unique<Record>();auto& descriptor=owner->descriptor;
            descriptor.frame=frame;descriptor.flags=0x40;descriptor.mask=0x400000;descriptor.actor=&actor_;
            update(*owner,true);records_.push_back(std::move(owner));
        }
        throw std::runtime_error("Animation BODYCOLB enumeration exceeds bounded native name sequence");
    }
    void update() {for(auto& record:records_)update(*record,false);}
private:
    void update(Record& record,bool initial) {
        auto& descriptor=record.descriptor;const auto frame=descriptor.frame;frames_.update_scene_object(frame);
        const auto scale=frames_.actor_local_scale(frame);
        const bool dummy=frames_.frame_kind(frame)==6;
        const auto bounds=dummy?frames_.car_dummy_bounds(frame):frames_.car_bounds(frame);
        descriptor.minimum=bounds.minimum;descriptor.maximum=bounds.maximum;
        Vector centre{},half{};float geometry_radius=0;
        if(!dummy&&!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Animation aggregate bounds leave native centre undefined");
        if(!dummy&&bounds.minimum[0]<bounds.maximum[0]) {
            //0040E080→004066C0→00404280 spills subtraction and half-span
            //before adding the minimum. The native radius is unscaled here.
            for(unsigned axis=0;axis!=3;++axis) {
                const float span=static_cast<float>(static_cast<long double>(bounds.maximum[axis])-bounds.minimum[axis]);
                half[axis]=static_cast<float>(static_cast<long double>(span)*.5f);
                centre[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])+half[axis]);
            }
            geometry_radius=static_cast<float>(std::sqrt((static_cast<long double>(half[2])*half[2]+static_cast<long double>(half[1])*half[1])+static_cast<long double>(half[0])*half[0]));
        }
        for(unsigned axis=0;axis!=3;++axis) {
            if(!std::isfinite(scale[axis])||scale[axis]==0)throw std::runtime_error("Animation BODYCOLB scale cannot normalize its native matrix");
            if(dummy) {
                //Y/Z sums spill before the half multiply; X remains in x87.
                const auto sum=static_cast<long double>(bounds.minimum[axis])+bounds.maximum[axis];
                centre[axis]=static_cast<float>((axis?static_cast<long double>(static_cast<float>(sum)):sum)*.5f);
            }
            descriptor.minimum[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])*scale[axis]);
            descriptor.maximum[axis]=static_cast<float>(static_cast<long double>(bounds.maximum[axis])*scale[axis]);
        }
        if(initial) {
            const auto text=frames_.car_user_data(frame);
            if(text.size()>5&&text.compare(0,5,"matid")==0) {
                char* end=nullptr;const auto value=std::strtoll(text.c_str()+5,&end,10);
                if(end!=text.c_str()+5) {
                    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Animation material ID exceeds native scanf integer");
                    descriptor.flags|=static_cast<std::uint32_t>(value)<<24;
                }
            }
        }
        const auto world=frames_.actor_world_matrix(frame);record.matrix=world;
        for(unsigned axis=0;axis!=3;++axis)for(unsigned component=0;component!=3;++component)
            record.matrix[axis*4+component]=static_cast<float>(static_cast<long double>(world[axis*4+component])/scale[axis]);
        record.inverse=frames_.car_inverse_matrix(record.matrix);
        if(dummy) {
            std::array<long double,3> span{};for(unsigned axis=0;axis!=3;++axis)span[axis]=static_cast<long double>(descriptor.maximum[axis])-descriptor.minimum[axis];
            const float square=static_cast<float>((span[2]*span[2]+span[1]*span[1])+span[0]*span[0]);
            std::int32_t bits;std::memcpy(&bits,&square,4);
            descriptor.radius=bits<0x322bcc77?0:std::abs(static_cast<long double>(square)-1.L)<9.99999993922529e-9f?.5f:static_cast<float>(std::sqrt(static_cast<long double>(square))*.5f);
        }else {
            //005B4CBF: parentless local scale, otherwise actual world GetScale.
            const auto world_scale=frames_.car_scale(frame);
            const auto yz=world_scale[1]>world_scale[2]?world_scale[1]:world_scale[2];
            const auto largest=world_scale[0]>yz?world_scale[0]:yz;
            descriptor.radius=static_cast<float>(static_cast<long double>(largest)*geometry_radius);
        }
        descriptor.position=frames_.car_transform(centre,world);
        //Constructor registers before publication in actorE0. Rollback on an
        //insert/move failure must not leave a borrowed descriptor in the grid.
        try {if(initial)collisions_.insert(descriptor);collisions_.move(descriptor);}
        catch(...){if(initial)collisions_.remove_for_cleanup(descriptor);throw;}
    }
    Frames& frames_;Collisions& collisions_;Actor& actor_;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
