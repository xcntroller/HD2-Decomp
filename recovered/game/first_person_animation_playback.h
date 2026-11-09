#ifndef HD2_GAME_FIRST_PERSON_ANIMATION_PLAYBACK_H
#define HD2_GAME_FIRST_PERSON_ANIMATION_PLAYBACK_H
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace hd2::game {
//00492A90 with actual table/model state. Notifications supplies actual004933C0
//sound receiver; no successful default. Resources is the SAME category3 cache.
//This starts tracks;00491440 frame advancement remains a separate consumer.
template<class Frames,class View,class Models,class Resources,class Notifications>
class FirstPersonAnimationPlayback {
public:
    enum class Result {gated,empty_row,no_variant,missing_clip,restarted,played};
    using Frame=typename Frames::Frame;
    FirstPersonAnimationPlayback(Frames& frames,View& view,Models& models,Resources& resources,Notifications& notifications)
        :frames_(frames),view_(view),models_(models),resources_(resources),notifications_(notifications) {}
    Result play(std::uint32_t mode,std::int32_t animation) {
        if(!(mode&2u)||!models_.model())return Result::gated;
        const auto definition=models_.definition();
        if(definition<0||static_cast<std::size_t>(definition)>=View::capacity||animation<0||animation>=13)
            throw std::runtime_error("First-person animation index exceeds native table");
        notifications_.first_person_animation(animation); //Actual004933C0, before model/clip selection.
        auto& state=models_.playback();const auto model=models_.model();
        if(animation>=2&&animation<=5) {
            if(definition==0x26) {
                const auto ammo=frames_.actor_find(model,"ammo",1);
                if(!ammo)throw std::runtime_error("First-person ammunition frame is missing");
                frames_.object_visible(ammo,false);frames_.update_scene_object(ammo);
            }
            if(const auto flash=models_.flash()) {
                models_.flash_time(50.f);frames_.first_person_flash_opacity(flash,1.f);
                const auto scale=static_cast<float>(static_cast<long double>(random())*.4f+.7);
                frames_.first_person_flash_scale(flash,scale);
                const auto direction=frames_.car_forward(flash);
                if(direction[0]==0&&direction[1]==0&&direction[2]==1)
                    frames_.first_person_flash_rotation(flash,static_cast<float>(static_cast<long double>(random())*3.1415927410125732421875f));
                frames_.object_visible(flash,true);frames_.update_scene_object(flash);
            }
            if(state.animation==animation) {
                state.current.weight=0;state.current.speed=5;
                frames_.human_model_time(model,state.current.slot,0);
                frames_.first_person_model_restart(model,state.current.slot);frames_.update_scene_object(model);
                return Result::restarted;
            }
        }
        auto& row=view_.record(static_cast<std::size_t>(definition)).rows[static_cast<std::size_t>(animation)];
        bool available=false;
        for(unsigned n=0;n!=4;++n)if(integer(row.weights[n])>0&&row.text[n]&&!row.text[n]->empty()){available=true;break;}
        if(!available)return Result::empty_row;
        //Weights are cumulative signed thresholds, not probabilities to sum.
        //Native truncates RandFloat()*100 and uses <=, so draw0 can select a
        //zero-weight/empty earlier slot. Preserve this edge case.
        const auto draw=static_cast<std::int32_t>(std::trunc(static_cast<long double>(random())*100.f));
        unsigned variant=0;while(variant!=4&&draw>integer(row.weights[variant]))++variant;
        if(variant==4)return Result::no_variant;
        auto& clip=row.frames[variant];
        if(!clip&&row.text[variant]&&!row.text[variant]->empty())(void)resources_.resolve_animation(*row.text[variant],clip);
        if(!clip)return Result::missing_clip;
        if(!state.current.clip||!frames_.human_model_animation_enabled(model,state.current.slot)) {
            state.current.weight=1;state.current.speed=0;
        }else {
            if(state.fading.size()>0x100000u)throw std::runtime_error("First-person fading tracks exceed bounds");
            state.fading.push_back(state.current);state.current.weight=0;state.current.speed=5;
        }
        state.animation=animation;state.variant=static_cast<std::int32_t>(variant);state.current.clip=clip.get();
        std::uint32_t slot=0;
        for(;slot!=3;++slot) {
            bool free=true;for(const auto& track:state.fading)if(track.slot==slot)free=false;
            if(free)break;
        }
        state.current.slot=slot;
        frames_.human_model_animation(model,clip.get(),slot,state.current.weight,0);frames_.update_scene_object(model);
        return Result::played;
    }
    void advance(std::uint32_t mode,std::int32_t elapsed) { //00491440, blend weights only.
        auto& state=models_.playback();const auto model=models_.model();
        if(!(mode&2u)||!view_.byte3c||!state.current.clip||!model)return;
        if(state.fading.size()>0x100000u)throw std::runtime_error("First-person fading tracks exceed bounds");
        if(state.current.weight==1.f) {
            //Native clears last-to-first and does not issue model24 here.
            while(!state.fading.empty()) {
                frames_.human_model_animation(model,Frame{},state.fading.back().slot,1.f,0);
                state.fading.pop_back();
            }
            return;
        }
        //FILD's exact signed integer is retained for the current lane; FST's
        //rounded float is reloaded for each fading lane. Both comparisons use
        //extended results before the float stores.
        const auto delta=static_cast<float>(elapsed);
        const auto weight=static_cast<long double>(elapsed)*state.current.speed*.001f+state.current.weight;
        state.current.weight=static_cast<float>(weight);
        if(weight>1.f)state.current.weight=1.f;
        frames_.human_model_weight(model,state.current.slot,state.current.weight);
        for(std::size_t i=0;i<state.fading.size();) {
            auto& track=state.fading[i];
            const auto faded=static_cast<long double>(track.weight)-static_cast<long double>(delta)*track.speed*.001f;
            track.weight=static_cast<float>(faded);
            if(faded>0||std::isnan(faded)) { //Native unordered comparison preserves NaN lanes.
                frames_.human_model_weight(model,track.slot,track.weight);++i;
            }else {
                frames_.human_model_animation(model,Frame{},track.slot,1.f,0);
                state.fading.erase(state.fading.begin()+static_cast<std::ptrdiff_t>(i));
            }
        }
        frames_.update_scene_object(model);
    }
private:
    float random() {
        const auto value=frames_.volume_random_unit();
        if(!std::isfinite(value)||value<0||value>1)throw std::runtime_error("Invalid first-person animation random value");
        return value;
    }
    static std::int32_t integer(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    Frames& frames_;View& view_;Models& models_;Resources& resources_;Notifications& notifications_;
};
} //namespace hd2::game
#endif
