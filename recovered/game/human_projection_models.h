#ifndef HD2_GAME_HUMAN_PROJECTION_MODELS_H
#define HD2_GAME_HUMAN_PROJECTION_MODELS_H

#include <cstdint>
#include <exception>
#include <list>
#include <utility>

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
//pool payloads remain unreconstructed: expose no invented append/producer API.
//This is a real empty cold owner/receiver, not a successful drawing service.
template<class Frames>class HumanProjectionModels {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Record{std::int32_t human_id;Owner frame;};
    explicit HumanProjectionModels(Frames& frames):frames_(frames){}
    ~HumanProjectionModels(){clear();}
    HumanProjectionModels(const HumanProjectionModels&)=delete;
    HumanProjectionModels& operator=(const HumanProjectionModels&)=delete;
    template<class Actors,class Actor>void visibility(Actors& actors,Actor* target,bool hide,std::uint32_t mode){human_projection_visibility(records_,actors,frames_,target,hide,mode);}
    const auto& records()const{return records_;}
    void clear()noexcept{
        while(!records_.empty()){
            auto& record=records_.front();
            try{frames_.detach_human_projection(record.frame.get());}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            record.frame.reset();records_.pop_front();
        }
    }
private:
    Frames& frames_;std::list<Record> records_;
};
} //namespace hd2::game
#endif
