#ifndef HD2_GAME_HUMAN_PROJECTION_MODELS_H
#define HD2_GAME_HUMAN_PROJECTION_MODELS_H

#include <cstdint>
#include <exception>
#include <list>
#include <utility>
#include "game/human_projection_process.h"

namespace hd2::game {

//0078B180 over actual0098AD3C list order. Record04 is a Human ID, NOT a
//frame/pointer. Resolve twice as native; null target matches every valid Human.
template<class Records,class Actors,class Frames,class Actor>
void human_projection_visibility(const Records& records,Actors& actors,Frames& frames,Actor* target,bool hide,std::uint32_t mode){
    if(!(mode&2u))return;
    for(const auto& record:records)if(actors.scene_human_identity(record.human_id)){
        auto* human=actors.scene_human_identity(record.human_id);
        if(human==target||!target)frames.object_visible(record.frame.get(),!hide);
    }
}

//Known loading lifetime of the SAME shared primary projection list. Original
//CRT0078A2C0 constructs an empty list; last-user0078A5F0 retires its frames
//before the400-word reset/textures. Runtime mark/projected-model creation and
//frame/footprint creation remains unreconstructed: no invented append/producer
//API. Actual update/clear uses the SAME process records, counts and pool.
template<class Frames>class HumanProjectionModels {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    using Process=HumanProjectionProcess<Owner>;
    explicit HumanProjectionModels(Frames& frames,Process& process):frames_(frames),process_(process){
        if(process_.world)throw std::logic_error("Human projection process already belongs to an actual World");
        process_.world=this;
    }
    ~HumanProjectionModels(){clear();process_.world=nullptr;}
    HumanProjectionModels(const HumanProjectionModels&)=delete;
    HumanProjectionModels& operator=(const HumanProjectionModels&)=delete;
    template<class Actors,class Actor>void visibility(Actors& actors,Actor* target,bool hide,std::uint32_t mode){human_projection_visibility(process_.primary,actors,frames_,target,hide,mode);}
    const auto& records()const{return process_.primary;}
    void initialize(std::uint32_t mode){if(mode&2u){process_.enabled=1;process_.maximum_per_human=process_.lifetime_seconds=40;}} //0078A4F0, no timer resets.
    template<class Services>void tick(bool authority,std::uint32_t elapsed,Services& services){
        struct Binding {
            Frames& frames;Services& services;
            std::uint32_t mode_flags()const{return services.mode_flags();}
            std::uint32_t projection_game_time(){return services.projection_game_time();}
            std::uint32_t projection_scene_epoch(){return frames.vertex_scene_epoch();}
            std::int32_t projection_random_word(){return services.projection_random_word();}
            std::uint32_t projection_quality_byte(){return services.projection_quality_byte();}
            std::uint32_t projection_frame_epoch(Frame frame){return frames.vertex_frame_epoch(frame);}
            void detach_projection(Frame frame){frames.detach_human_projection(frame);}
            void projection_mode(Frame frame,unsigned value){frames.human_projection_mode(frame,value);}
            void projection_blend(Frame frame,unsigned value){frames.human_projection_blend(frame,value);}
            float projection_alpha(Frame frame){return frames.human_projection_alpha(frame);}
            void projection_alpha(Frame frame,float alpha){frames.human_projection_alpha(frame,alpha);}
            void update_projection(Frame frame){frames.update_scene_object(frame);}
        } binding{frames_,services};
        tick_item_projection_process(process_,authority,elapsed,binding);
    }
    void clear()noexcept{
        for(auto* records:{&process_.primary,&process_.fading})while(!records->empty()){
            auto& record=records->front();
            try{frames_.detach_human_projection(record.frame.get());}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            record.frame.reset();process_.pool.splice(process_.pool.begin(),*records,records->begin()); //Native last-user releases frame BEFORE pooled payload.
        }
        process_.per_human.fill(0);
    }
private:
    Frames& frames_;Process& process_;
};
} //namespace hd2::game
#endif
