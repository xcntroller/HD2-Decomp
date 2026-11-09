#ifndef HD2_HOST_GAME_PHYSICS_OBJECTS_H
#define HD2_HOST_GAME_PHYSICS_OBJECTS_H

#include "host/game_actor_services.h"
#include "game/physics_object.h"
#include <limits>

namespace hd2::host {

//004106E0 module2,00410A40 activation,00410B00 scene enumeration and
//0040FB00's two owner lists/stable reusable handles. Loading uses actual frame
//metadata, userdata and the SAME collision buckets/runtime queue as other actors.
//Types0..5 have source owners; type6 rejects its unresolved second registration.
//The containing world supplies the actual shared fade-out owner for type4.
template<class Mode,class Collisions,class RuntimeActors>
class GamePhysicsObjects {
public:
    using Frames=GameActorServices;
    using Frame=Frames::Frame;
    using Object=game::PhysicsObject<Frames,Collisions,RuntimeActors>;
    using Parameters=typename Object::Parameters;
    using Handle=game::PhysicsObjectHandle;
    GamePhysicsObjects(Frames& frames,Mode& mode,Collisions& collisions,RuntimeActors& actors)
        : frames_(frames),mode_(mode),collisions_(collisions),actors_(actors) {}
    GamePhysicsObjects(Frames& frames,Mode& mode,Collisions& collisions,RuntimeActors& actors,game::EffectsFrameFades<Frames>& fades)
        :frames_(frames),mode_(mode),collisions_(collisions),actors_(actors),fades_(&fades){}
    ~GamePhysicsObjects(){clear();}
    GamePhysicsObjects(const GamePhysicsObjects&)=delete;
    GamePhysicsObjects& operator=(const GamePhysicsObjects&)=delete;
    Handle* create(const Parameters& parameters) {
        if(!parameters.frame)return nullptr;
        auto found=std::find_if(handles_.begin(),handles_.end(),[](const auto& handle){return handle->index==-1;});
        Handle* handle;
        if(found==handles_.end()) {
            handles_.push_back(std::make_unique<Handle>());handle=handles_.back().get();
        } else handle=found->get();
        auto& list=parameters.first_list?first_:second_;
        auto slot=std::find(list.begin(),list.end(),nullptr);
        const auto index=static_cast<std::size_t>(slot-list.begin());
        if(index>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
            throw std::runtime_error("Physics object list exceeds native handle bounds");
        //Capacity before the constructor publishes a collision/queue pointer.
        if(slot==list.end())list.reserve(list.size()+1);
        if(parameters.type==5)scene_objects_.reserve(scene_objects_.size()+1);
        auto owner=std::make_unique<Object>(frames_,collisions_,actors_,parameters,*handle,fades_);
        owner->initialize();
        handle->index=static_cast<std::int32_t>(index);handle->first_list=parameters.first_list;
        try {owner->publish();}catch(...) {handle->index=-1;throw;}
        auto* object=owner.get();
        if(index==list.size())list.push_back(std::move(owner));else list[index]=std::move(owner);
        if(parameters.type==5)scene_objects_.push_back(object);
        return handle;
    }
    Object* object(const Handle* handle) const {
        if(!handle||handle->index<0)return nullptr;
        const auto& list=handle->first_list?first_:second_;
        const auto index=static_cast<std::size_t>(handle->index);
        if(index>=list.size())throw std::runtime_error("Invalid physics object handle");
        return list[index].get();
    }
    void activate(bool authority) { //00410A40: authority OR single-player presentation.
        if(!authority&&(mode_.mode_flags()&1))return;
        clear();scene_objects_.clear();
        frames_.enumerate_physics_scene([&](Frame frame) {
            auto parameters=game::physics_scene_parameters(frame,frames_.frame_kind(frame),frames_.actor_user_data(frame));
            if(parameters)create(*parameters);
        });
    }
    void clear() noexcept { //00410830, reverse first list, then reverse second, then handles.
        for(auto* list:{&first_,&second_}) {
            for(auto index=list->size();index!=0;)(*list)[--index].reset();
            list->clear();
        }
        scene_objects_.clear();handles_.clear();
    }
private:
    Frames& frames_;Mode& mode_;Collisions& collisions_;RuntimeActors& actors_;
    game::EffectsFrameFades<Frames>* fades_=nullptr;
    std::vector<std::unique_ptr<Handle>> handles_;
    std::vector<std::unique_ptr<Object>> first_,second_;
    std::vector<Object*> scene_objects_;
};

} //namespace hd2::host
#endif
