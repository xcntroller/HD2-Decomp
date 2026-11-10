#ifndef HD2_GAME_EFFECTS_HUMAN_FADES_H
#define HD2_GAME_EFFECTS_HUMAN_FADES_H

#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <vector>

namespace hd2::game {

//GEffects228's borrowed Human queue00489180 and00487240/487BFD..487D7F.
//Typed closures read each actual Human's CURRENT model, never a retained/stale
//copy of its frame. Clear before Scene2 retirement; no Human/frame is owned.
//Independent of GEffects200/210 frame queues and model+50 opacity.
template<class Frames>class EffectsHumanFades {
public:
    using Frame=typename Frames::Frame;
    struct Record{const void* human;std::function<Frame()> model;bool show;float rate,amount;};
    explicit EffectsHumanFades(Frames& frames):frames_(frames){}
    EffectsHumanFades(const EffectsHumanFades&)=delete;
    EffectsHumanFades& operator=(const EffectsHumanFades&)=delete;
    template<class Human>void visibility(Human& human,std::int32_t duration,bool show){ //00489180, no process mode gate.
        const auto model=human.base().frame();if(!model)throw std::runtime_error("Human fade requires its actual model");
        if(!duration){frames_.effects_model_camera_amount(model,show?1.f:0.f);return;} //Does NOT cancel/update a queued record.
        const auto visual=frames_.actor_find(model,"a",1);
        const auto rate=static_cast<float>(1.L/duration); //Signed FILD, including negative durations.
        for(auto n=records_.size();n;)if(auto& record=records_[--n];record.human==&human){
            record.show=show;record.rate=rate;
            record.amount=visual?frames_.effects_human_fade_amount(visual):1.f;return;
        }
        const auto amount=visual?frames_.effects_human_fade_amount(visual):1.f;
        records_.push_back(Record{&human,[&human]{return human.base().frame();},show,rate,amount});
    }
    void update(std::uint32_t elapsed){
        std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
        const float time=static_cast<float>(signed_elapsed); //Native spills FILD elapsed to FLOAT once before traversal.
        for(auto n=records_.size();n;){--n;auto& record=records_[n];
            const auto step=static_cast<long double>(time)*record.rate;
            record.amount=static_cast<float>(record.show?step+record.amount:static_cast<long double>(record.amount)-step);
            const auto model=record.model();if(!model)throw std::runtime_error("Queued Human fade lost its actual model owner");
            if(record.show?record.amount>=1.f:record.amount<=0.f){
                frames_.effects_model_camera_amount(model,record.show?1.f:0.f);
                records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(n)); //ORDERED erase after successful frame call.
            }else frames_.effects_model_camera_amount(model,record.amount); //NaN stays queued on both paths.
        }
    }
    const auto& records()const{return records_;}
    void clear()noexcept{records_.clear();} //Containing source lifetime guard; no invented native cancellation function.
private:
    Frames& frames_;std::vector<Record> records_;
};
} //namespace hd2::game
#endif
