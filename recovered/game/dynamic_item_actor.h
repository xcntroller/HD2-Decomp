#ifndef HD2_GAME_DYNAMIC_ITEM_ACTOR_H
#define HD2_GAME_DYNAMIC_ITEM_ACTOR_H

#include "game/scene_actor.h"
#include "game/rigid_body.h"
#include "frontend/cleanup.h"
#include "frontend/pre_mission_data.h"
#include "game/dynamic_item_contact_sound.h"
#include <limits>

namespace hd2::game {

//Loose item runtime type11:005B6740/005B67D0/005B68D0/005B69B0. This
//differs from type35's static queued Item. Actual item module ownership, sound
//and network callbacks are REQUIRED services, never successful empty methods.
template<class Backend,class Activation,class Collisions,class Items>
class DynamicItemActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Entry=typename Backend::Entry;
    using Body=RigidBody<Backend,Collisions>;
    using Vector=typename Backend::Vector;
    using Descriptor=typename Body::Descriptor;
    DynamicItemActor(Backend& frames,Activation& activation,Collisions& collisions,typename Body::Registry& bodies,Items& items)
        : SceneActor<Backend>(frames,11),activation_(activation),items_(items),body_(frames,collisions,bodies) {}
    bool initialize(Frame frame) override { //005B68D0.
        entry_=Entry{};
        if(!frame)return false;
        this->backend_.item_actor_unit_scale(frame); //Native frame+54(1).
        return this->base_.initialize_frame(frame);
    }
    void load(SerializedBytes&) override {} //005B6840 returns0/RET4, ignored by caller.
    void activate(bool authority) { //005B6900.
        this->base_.activate_frame(authority,activation_);flag6a8_=false;
    }
    void bind_item(Entry entry,const Vector& velocity) { //005B69B0.
        if(entry_)throw std::logic_error("Loose item binding repeated");
        entry_=entry;
        if(!entry)return;
        auto& item=items_.runtime_item(entry);
        auto model=item.model();
        if(!model)throw std::runtime_error("Loose item requires its actual model");
        if(model!=this->base_.frame())throw std::runtime_error("Loose item model disagrees with its runtime frame");
        body_.bind(model);
        const auto category=frontend::pre_mission_detail::Bytes::integer(item.definition().category); //007D32A0, not instance family.
        typename Body::Parameters parameters{velocity,item.scalar(),.800000011920928955078125f,
            [this](unsigned event,void* context,Descriptor* contact){return callback(event,context,contact);},this,false,category<=1};
        body_.configure(parameters);body_.reset();body_.actor(this);
        field6a4_=0;field6ac_=-1;
        if(!(activation_.mode_flags()&1)||(activation_.mode_flags()&7)==7||items_.dynamic_item_server_policy()||
           item.definition().kind()==20||item.definition().kind()==21)field6a0_=-1;
        else {
            const auto value=static_cast<long double>(items_.dynamic_item_random_unit())*10000.f+30000.f;
            if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
                throw std::runtime_error("Loose item lifetime exceeds bounds");
            field6a0_=static_cast<std::int32_t>(value);
        }
        field6b0_=0;
        this->base_.name(items_.dynamic_item_caption(item.definition().caption));
        bound_=true;
    }
    void deactivate(bool authority) { //005B6920: close, release frame, delete module-owned entry, common.
        body_.close();
        this->base_.release_runtime_frame();
        const auto entry=std::exchange(entry_,Entry{});
        frontend::CleanupErrors errors;
        if(entry)errors.attempt([&]{items_.remove_runtime_item(entry);}); //007DC8A0, not raw delete.
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation_);});
        errors.rethrow();
    }
    int callback(unsigned event,void* context,Descriptor* contact) { //005B7630, native fastcall boundary.
        if(event==1) {
            if(!contact)throw std::runtime_error("Loose item contact requires an actual descriptor");
            if((contact->flags&0x40)&&(contact->mask&0x10100))return 1;
            if(context&&(activation_.mode_flags()&2))items_.dynamic_item_contact_sound(*this); //005B7570.
            return 2;
        }
        if(event==0&&entry_&&(activation_.mode_flags()&7)!=7&&(activation_.mode_flags()&1))
            items_.dynamic_item_sleep_notification(*this); //005B7C50, actual network producer required.
        return 0;
    }
    template<class Sound>void contact_sound(Sound& sound) { //005B7570, actual item definition38 and world pose.
        emit_dynamic_item_contact(this->backend_,items_,entry_,this->base_.frame(),field6a4_,sound);
    }
    template<class Services>void advance_frame(bool authority,std::uint32_t elapsed,Services& services) {
        if(!authority) { //005B7480: body24 runs even without item90, BEFORE cooldown.
            services.advance_loose_item_body(body_,elapsed);
            if(field6a4_!=0) {
                field6a4_=subtract_time(field6a4_,elapsed);
                if(field6a4_<0)field6a4_=0;
            }
            return;
        }
        if(!entry_)return; //005B7320, native entry90 gate precedes every write.
        require_bound(); //Native safety: do not consume a partially bound source object.
        field6b0_+=elapsed;
        if(!(activation_.mode_flags()&2u))services.advance_loose_item_body(body_,elapsed);
        if(field6ac_!=-1) {
            field6ac_=subtract_time(field6ac_,elapsed);
            if(field6ac_<=0) {
                services.restore_loose_item(*this); //005B7EC0, actual retained-item/packet producer.
                field6ac_=-1; //Publish only AFTER successful receiver.
            }
        }
        if(field6a0_==-1)return;
        field6a0_=subtract_time(field6a0_,elapsed);
        if(field6a0_<=0) {
            services.restore_loose_item_opacity(this->base_.frame()); //GEffects60=00486E90, NOT fade cancellation.
            this->backend_.object_visible(this->base_.frame(),false);
            this->backend_.update_scene_object(this->base_.frame());
            services.send_loose_item_expiration(*this); //00478780(id,true), opcode0/3 bits,00478880.
            field6a0_=-1;
            if(!(activation_.mode_flags()&2u)) {
                services.remove_loose_item(this); //Actual deferred runtime registry. No subsequent actor read.
                return;
            }
        }else if(field6a0_<=500&&!flag6a8_) { //Native FILD signed integer versus FLOAT0080F6F0=500.
            flag6a8_=true; //Failed fade must preserve this publication.
            services.fade_loose_item(this->base_.frame(),field6a0_);
        }
    }
    template<class Packets>void publish_expiration(Packets& packets) const {
        //005B7320 calls00478780 with priority=true over the existing process
        //module12 broadcast stream. No additional packet/transport owner.
        auto& packet=packets.begin(12,0);
        packet.write(0,2);packet.write(static_cast<std::uint32_t>(this->base_.id10()),10);
        //FYL2X(log4/log2)+DOUBLE1 -> __ftol is3, distinct from Mine's2-bit command.
        packet.write(0,3);packet.finish_broadcast(); //00478880→0072BF90.
    }
    Entry entry() const {return entry_;}
    Body& body() {return body_;}
    void thrown_lifetime(std::int32_t value){require_bound();field6ac_=value;} //00478EE0's nonoffline kind20 branch.
    std::int32_t lifetime() const {require_bound();return field6a0_;}
    std::int32_t sound_cooldown() const {require_bound();return field6a4_;}
    std::uint32_t frame_age() const {require_bound();return field6b0_;}
private:
    static std::int32_t subtract_time(std::int32_t value,std::uint32_t elapsed) {
        const auto bits=static_cast<std::uint32_t>(value)-elapsed;
        std::int32_t result;std::memcpy(&result,&bits,4);return result;
    }
    void require_bound() const {if(!bound_)throw std::logic_error("Loose item fields require completed binding");}
    Activation& activation_;Items& items_;Body body_;
    Entry entry_{};
    std::int32_t field6a0_=-1,field6a4_=0,field6ac_=-1;
    std::uint32_t field6b0_=0;
    bool flag6a8_=false;
    bool bound_=false;
};

} //namespace hd2::game
#endif
