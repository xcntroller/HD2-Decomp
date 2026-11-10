#ifndef HD2_GAME_HUMAN_ANIMATION_WORLD_BINDINGS_H
#define HD2_GAME_HUMAN_ANIMATION_WORLD_BINDINGS_H

#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//00774D80/007759D0's actual model EC lookup and per-Human DD0+260 receiver.
//No callback is routed through a different Human's tracks. Frames and World
//borrow the SAME Scene2 registry, checked composition, Players and FPV owners.
template<class Frames,class World>class HumanAnimationWorldBindings {
public:
    using Frame=typename Frames::Frame;using Actor=typename Frames::Instance;
    using Human=typename Frames::Human;
    HumanAnimationWorldBindings(Frames& frames,World& world,const float& time_scale)
        :frames_(frames),world_(world),time_scale_(time_scale){}
    Actor* animation_frame_actor(Frame frame){
        const auto* data=frame?frames_.find_actor_frame_data(frame):nullptr;
        return data?reinterpret_cast<Actor*>(data->words[0]):nullptr;
    }
    void queue_human_animation_event(Actor& actor,const char* text,std::int32_t time){
        auto& control=world_.human_state_factory(human(actor)).control();
        auto* dd0=control.control();if(!dd0)throw std::runtime_error("Animation marker requires actual Human DD0");
        dd0->tracks().queue_event(text,time);
    }
    unsigned animation_actor_type(Actor& actor)const{return actor.base().type();}
    bool animation_view_actor(Actor& actor){return world_.human_is_view_actor(human(actor));}
    bool animation_view_offset_enabled(){return world_.human_view_offset_enabled();}
    void animation_player_check(Frame model){
        auto* actor=animation_frame_actor(model);
        if(!actor)throw std::runtime_error("Animation player query requires the actual bound Human");
        (void)world_.human_is_player(human(*actor)); //007724E0 queries its own Human8, result unused.
    }
    float animation_time_scale()const{return time_scale_;} //SAME process FLOAT0084E80C.
private:
    static Human& human(Actor& actor){
        auto* result=dynamic_cast<Human*>(&actor);
        if(!result)throw std::runtime_error("Human animation receiver has a different actual actor family");
        return *result;
    }
    Frames& frames_;World& world_;const float& time_scale_;
};

//Actual shared SOUND50/54 consumers, including null-Human speech markers.
template<class Spatial,class Startup,class Dubbing>class HumanAnimationWorldSound {
public:
    HumanAnimationWorldSound(Spatial& spatial,Startup& startup,Dubbing& dubbing)
        :spatial_(spatial),startup_(startup),dubbing_(dubbing){}
    template<class Actor,class Frame>void play_animation_speech(Actor* actor,Frame frame,std::int32_t identity,float near_range,float far_range,bool subtitles){
        (void)startup_.play_game_speech(actor,frame,identity,near_range,far_range,subtitles,dubbing_);
    }
    template<class Request>auto emit_animation_sound(const Request& request){return spatial_.emit(request);}
private:
    Spatial& spatial_;Startup& startup_;Dubbing& dubbing_;
};

} //namespace hd2::game
#endif
