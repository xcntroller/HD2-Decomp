#ifndef HD2_GAME_SWITCH_ACTOR_STATE_H
#define HD2_GAME_SWITCH_ACTOR_STATE_H

#include "game/actor_properties.h"
#include <limits>
#include <optional>

namespace hd2::game {

//00613550/006136F0/00613E10 after common00610200 frame initialization. Borrowed
//frames belong to the containing actor's retained model. Runtime notifications
//and spatial sound are mandatory consumers, not optional successful callbacks.
//The actual concrete actor/module/audio composition is still required.
template<class Frames>
class SwitchActorState {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    explicit SwitchActorState(Frames& frames) : frames_(frames) {properties_.fieldd8=0xffffffffu;}
    template<class Runtime> bool initialize(Frame model,Runtime& runtime) {
        if(!model)return false;
        if(model_)throw std::logic_error("Switch initialization repeated");
        model_=model;variants_=0;
        const auto first=frames_.actor_find(model_,"1",0xffff);
        for(;;) {
            if(variants_==0x100000)throw std::runtime_error("Switch variant count exceeds bounds");
            const auto name=std::to_string(variants_+1);
            const auto frame=frames_.actor_find(model_,name.c_str(),0xffff);
            if(!frame)break;
            if(first&&first!=frame)align(frame,frames_.actor_parent(first),frames_.actor_position(first));
            ++variants_;
        }
        apply(0,runtime);return true;
    }
    template<class Runtime> void load(SerializedBytes& bytes,Runtime& runtime) {
        properties_.load(bytes,[&](std::uint32_t value) {apply(value,runtime);});
    }
    template<class Runtime> void apply(std::uint32_t value,Runtime& runtime) {
        properties_.fieldd4=value>=1&&value<=3?value:1;
        if(!model_)return;
        if((runtime.mode_flags()&1)&&runtime.switch_has_context(model_)&&!suppress_notification_)
            runtime.notify_switch_state(model_,properties_.fieldd4); //0041D960, before visual work.
        const auto first=frames_.actor_find(model_,"1",0xffff);
        if(!first)return;
        const auto parent=frames_.actor_parent(first);
        const auto position=frames_.actor_position(first);
        if(!parent)return;
        const auto name=std::to_string(properties_.fieldd4);
        const auto active=frames_.actor_find(model_,name.c_str(),0xffff);
        if(!active)return;
        align(active,parent,position);frames_.update_scene_object(active);
        auto count=frames_.actor_child_count(model_);
        while(count) {
            --count;
            if(frames_.actor_child(model_,count))
                frames_.object_visible(frames_.actor_child(model_,count),false);
        }
        frames_.object_visible(active,true);
        if(properties_.fieldd8<=std::uint32_t(std::numeric_limits<std::int32_t>::max())) {
            const auto world=frames_.actor_world_matrix(model_);Vector position_world{};
            for(std::size_t i=0;i!=4;++i)position_world[i]=world[12+i];
            runtime.play_switch_sound(properties_.fieldd8,position_world); //Sound manager+54.
        }
    }
    std::uint32_t state() const {return properties_.fieldd4;}
    std::uint32_t sound() const {return properties_.fieldd8;}
    std::uint32_t variants() const {return variants_;}
    void suppress_notification(bool value) {suppress_notification_=value;}
    void begin_activation() {initial_state_=properties_.fieldd4;suppress_notification_=false;} //00613920 tail.
    template<class Runtime>void restore_activation(Runtime& runtime) { //00613BD0 authority tail.
        if(!initial_state_)throw std::logic_error("Switch has no activated state to restore");
        apply(*initial_state_,runtime);
    }
private:
    void align(Frame frame,Frame parent,const Vector& position) {
        frames_.link_scene_object(frame,parent);
        frames_.object_position(frame,std::array<float,3>{position[0],position[1],position[2]});
    }
    Frames& frames_;
    Frame model_{};
    SwitchActorProperties properties_;
    std::uint32_t variants_=0;
    bool suppress_notification_=false;
    std::optional<std::uint32_t> initial_state_;
};

} //namespace hd2::game
#endif
