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
    Frames& frames(){return frames_;}
    Handle* create(const Parameters& parameters) {
        if(clearing_)throw std::logic_error("Physics construction during owner cleanup");
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
        auto owner=std::make_unique<Object>(frames_,collisions_,actors_,parameters,*handle,fades_,[this](const Parameters& child,const Frames::Vector& motion){
            auto* created=create(child);(void)contact(reinterpret_cast<std::uintptr_t>(created),1,motion,{});
        });
        owner->initialize();owner->install_feedback();
        handle->index=static_cast<std::int32_t>(index);handle->first_list=parameters.first_list;
        try {owner->publish();}catch(...) {handle->index=-1;throw;}
        auto* object=owner.get();
        if(index==list.size())list.push_back(std::move(owner));else list[index]=std::move(owner);
        if(parameters.type==5)scene_objects_.push_back(object);
        try{(void)object->feedback(0);}catch(...){ //00410094 ECX0 after actual list/EC publication.
            const auto failure=std::current_exception();
            try{release(handle);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        return handle;
    }
    Object* object(const Handle* handle) const {
        if(!handle)return nullptr;
        if(std::none_of(handles_.begin(),handles_.end(),[&](const auto& value){return value.get()==handle;}))
            throw std::logic_error("Physics object lookup requires its actual stable handle allocation");
        if(handle->index<0)return nullptr;
        const auto& list=handle->first_list?first_:second_;
        const auto index=static_cast<std::size_t>(handle->index);
        if(index>=list.size())throw std::runtime_error("Invalid physics object handle");
        if(list[index] && &list[index]->handle()!=handle)throw std::logic_error("Physics handle names a different object");
        return list[index].get();
    }
    bool release(Handle*& handle){ //00410230: leave a NULL slot, invalidate caller/handle BEFORE destruction.
        auto* value=object(handle);if(!value)return false;
        auto& list=handle->first_list?first_:second_;const auto index=static_cast<std::size_t>(handle->index);
        auto owner=std::move(list[index]);
        frontend::CleanupErrors errors;errors.attempt([&]{owner->unpublish_handle(false);});
        handle->index=-1;handle=nullptr;
        for(auto& item:scene_objects_)if(item==value)item=nullptr;
        owner.reset();errors.rethrow();return true;
    }
    template<class Simulation>void tick(bool authority,std::uint32_t elapsed,Simulation& simulation){ //00410100/004101A0, FIRST list only, reverse captured count.
        if(!(mode_.mode_flags()&(authority?1u:2u)))return;
        std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
        const float seconds=static_cast<float>(static_cast<long double>(signed_elapsed)*.0010000000474974513f);
        for(auto n=first_.size();n;){--n;if(!first_[n])continue;
            const auto kind=first_[n]->parameters().type;
            if(authority){if(kind==4||kind==6||(kind==5&&(mode_.mode_flags()&2u)))continue;}
            else if(kind!=4&&kind!=5&&kind!=6)continue;
            const auto keep=first_[n]->advance(seconds,simulation);
            if(n>=first_.size()||!first_[n])throw std::logic_error("Physics update removed its actual active slot before pose publication");
            first_[n]->synchronize_pose(); //Native even when keep==false, before retirement callback/migration.
            if(n>=first_.size()||!first_[n])throw std::logic_error("Physics pose callback removed its native active slot");
            if(!keep)retire_first(n);
        }
    }
    bool contact(std::uintptr_t token,unsigned kind,const Frames::Vector& velocity,const Frames::Vector& point){
        if(!token)return false;
        auto* handle=reinterpret_cast<Handle*>(token);
        //Check the borrower against actual stable allocations BEFORE reading
        //it; foreign/stale frame data is not a native handle to dereference.
        if(std::none_of(handles_.begin(),handles_.end(),[&](const auto& value){return value.get()==handle;}))
            throw std::logic_error("Physics contact has no matching actual handle owner");
        return game::contact_physics_object(first_,second_,handle,[&](Object& value){return value.contact(kind,velocity,point);});
    }
    Object& borrowed_object(void* borrowed){
        for(auto* list:{&first_,&second_})for(auto& owner:*list)if(owner.get()==borrowed&&borrowed)return *owner;
        throw std::logic_error("Placed item Physics90 has no live object in SAME Physics owner");
    }
    bool placed_contact(void* borrowed,const Frames::Vector& motion){return borrowed_object(borrowed).contact(3,motion,{});} //005E2510 calls virtual14 directly, no manager wake/event.
    void hide_placed_replacement(void* borrowed){borrowed_object(borrowed).hide_pickup_replacement();}
    void impulse(Handle* handle,const Frames::Vector& force,const Frames::Vector& point,float scale){ //00410420, wake BEFORE virtual08, then event2.
        if(!object(handle))return;
        (void)game::contact_physics_object(first_,second_,handle,[&](Object& value){
            value.impulse(force,point,scale);(void)value.feedback(2);return true;
        });
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
        if(clearing_)return;
        clearing_=true;
        for(auto* list:{&first_,&second_}) {
            for(auto index=list->size();index!=0;)(*list)[--index].reset();
            list->clear();
        }
        scene_objects_.clear();handles_.clear();clearing_=false;
    }
private:
    void retire_first(std::size_t index){ //00410940: event5 before move; exact1 overrides object158 deletion flag.
        bool destroy=first_.at(index)->delete_when_stopped();
        if(const auto result=first_[index]->feedback(5);result&&*result==1)destroy=true;
        if(index>=first_.size()||!first_[index])throw std::logic_error("Physics stop callback removed its native active slot");
        if(second_.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Physics dormant list exceeds native handle bounds");
        second_.reserve(second_.size()+1);
        auto owner=std::move(first_[index]);
        if(index+1!=first_.size()){
            first_[index]=std::move(first_.back());
            if(first_[index]&&first_[index]->handle().index!=-1)first_[index]->handle().index=static_cast<std::int32_t>(index);
        }
        first_.pop_back();auto* handle=&owner->handle();
        handle->index=static_cast<std::int32_t>(second_.size());handle->first_list=false;
        second_.push_back(std::move(owner));
        if(destroy)(void)release(handle);
    }
    Frames& frames_;Mode& mode_;Collisions& collisions_;RuntimeActors& actors_;
    game::EffectsFrameFades<Frames>* fades_=nullptr;
    std::vector<std::unique_ptr<Handle>> handles_;
    std::vector<std::unique_ptr<Object>> first_,second_;
    std::vector<Object*> scene_objects_;
    bool clearing_=false;
};

} //namespace hd2::host
#endif
