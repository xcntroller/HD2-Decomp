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
//before the400-word reset/textures. Actual footprint publication/update/clear
//uses the SAME process records, counts and pool.
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
    std::uint32_t enabled()const{return process_.enabled;}
    template<class Services,class Before>void emit_footprint(std::int32_t human_id,unsigned texture_index,bool left,Frame texture,
            const std::array<float,4>& position,float angle,Frame parent,float alpha,float colour,Services& services,Before&& before){ //0078A4B0/0078AA10.
        //0075D210 has already checked enabled!=0;0078AA10 separately requires==1.
        typename Process::Records pending;
        if(process_.pool.empty())pending.emplace_back();
        else pending.splice(pending.begin(),process_.pool,process_.pool.begin());
        auto& record=pending.front();
        bool counted=false,scene_added=false;
        try{
            if(!record.frame)record.frame=frames_.create_shot_surface();
            if(!record.frame)throw std::runtime_error("Footprint requires actual PROJ visual");
            record.texture_index=texture_index;record.left=left;
            frames_.human_projection_alpha(record.frame.get(),alpha);
            frames_.human_footprint_colour(record.frame.get(),colour);
            before(); //0075D210 hides the Human AFTER creating/colouring the actual PROJ.
            record.human_id=human_id;
            if((services.mode_flags()&2u)&&process_.enabled==1){
                if(human_id<0||static_cast<std::size_t>(human_id)>=process_.per_human.size())throw std::out_of_range("Footprint Human exceeds original400-word table");
                record.time=services.projection_game_time();++process_.per_human[static_cast<std::size_t>(human_id)];counted=true;
                frames_.shot_surface_texture(record.frame.get(),texture);frames_.human_projection_mode(record.frame.get(),0);
                frames_.object_visible(record.frame.get(),true);scene_added=true;frames_.human_footprint_add(record.frame.get());
                auto point=position;point[1]+=.5f;
                frames_.object_position(record.frame.get(),{{point[0],point[1],point[2]}});
                frames_.shot_surface_direction(record.frame.get(),{{0,-1,0,0}},static_cast<float>(static_cast<long double>(angle)+.7853981852531433f));
                frames_.shot_surface_scale(record.frame.get(),.11999999731779099f,.11999999731779099f,.699999988079071f);
                frames_.human_projection_blend(record.frame.get(),5);frames_.human_footprint_range(record.frame.get(),22.f);
                frames_.human_equipment_opacity(record.frame.get(),1.f);frames_.human_equipment_link(record.frame.get(),parent,0);
                frames_.update_scene_object(record.frame.get());frames_.human_footprint_project(record.frame.get());
                frames_.human_projection_mode(record.frame.get(),3);
                process_.primary.splice(process_.primary.end(),pending);return;
            }
        }catch(...){
            const auto failure=std::current_exception();
            if(scene_added)try{frames_.detach_human_projection(record.frame.get());}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            if(counted)--process_.per_human[static_cast<std::size_t>(human_id)];
            process_.pool.splice(process_.pool.begin(),pending);std::rethrow_exception(failure);
        }
        //Native caller has acquired the payload even if the inner==1 gate
        //rejects it. Retain its actual frame in the SAME pool as a source guard.
        process_.pool.splice(process_.pool.begin(),pending);
    }
    void retain_surface_position(const std::array<float,4>& position,std::uint32_t mode){process_.retain_surface_position(position,mode);}
    bool near_surface_position(const std::array<float,4>& position,std::uint32_t mode)const{return process_.near_surface_position(position,mode);}
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
        process_.clear_surface_positions(); //Native last-user clears retained surface positions BEFORE frame lists/400 words.
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
