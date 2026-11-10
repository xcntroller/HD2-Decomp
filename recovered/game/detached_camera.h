#ifndef HD2_GAME_DETACHED_CAMERA_H
#define HD2_GAME_DETACHED_CAMERA_H

#include "game/human_joint_math.h"
#include "game/human_camera_amount.h"
#include <utility>
#include <vector>

namespace hd2::game {

//00581210, squared distance from point to head/body segment. Deltas stay
//extended until each native FLOAT store; the unused closest-point temporary
//does not replace the endpoint/relative-vector computations.
inline long double camera_segment_square(const HumanVector& point,const HumanVector& first,const HumanVector& last,float& parameter) {
    std::array<long double,3> d{},w{};
    for(unsigned a=0;a!=3;++a){d[a]=static_cast<long double>(last[a])-first[a];w[a]=static_cast<long double>(point[a])-first[a];}
    const auto square=(d[2]*d[2]+d[1]*d[1])+d[0]*d[0];parameter=0;
    if(square<9.99999993922529e-09f)return (w[2]*w[2]+w[1]*w[1])+w[0]*w[0];
    const float x=static_cast<float>(w[0]),y=static_cast<float>(w[1]);
    const float t=static_cast<float>(((w[2]*d[2]+static_cast<long double>(y)*d[1])+static_cast<long double>(x)*d[0])/square);
    std::array<float,3> relative{};
    if(t<=0) {
        for(unsigned a=0;a!=3;++a)relative[a]=static_cast<float>(w[a]);
    }else if(t>=1) {
        parameter=1;for(unsigned a=0;a!=3;++a)relative[a]=static_cast<float>(static_cast<long double>(point[a])-last[a]);
    }else {
        parameter=t;
        for(unsigned a=0;a!=3;++a)relative[a]=static_cast<float>(static_cast<long double>(static_cast<float>(w[a]))-static_cast<float>(d[a]*t));
        //Interior0041CFD0 uses X²+Y²+Z²; endpoints use Z²+Y²+X².
        return (static_cast<long double>(relative[0])*relative[0]+static_cast<long double>(relative[1])*relative[1])+static_cast<long double>(relative[2])*relative[2];
    }
    return (static_cast<long double>(relative[2])*relative[2]+static_cast<long double>(relative[1])*relative[1])+static_cast<long double>(relative[0])*relative[0];
}

//00462650/004626E0's actual kind3 camera and Human-reference vector8C.
//Human actors are borrowed; cached FLOAT amount is native record+4. Movement,
//toggle/input/simulation remain separate bodies, never successful defaults.
template<class Frames,class Human>class DetachedCamera {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().create_game_camera());
    struct Reference {Human* human;float amount;};
    explicit DetachedCamera(Frames& frames):frames_(frames),camera_(frames.create_game_camera()) {
        if(!camera_.get())throw std::runtime_error("Detached camera factory returned null");
    }
    DetachedCamera(const DetachedCamera&)=delete;DetachedCamera& operator=(const DetachedCamera&)=delete;
    Frame frame()const{return camera_.get();}float speed()const{return speed_;}void speed(float v){speed_=v;} //00462750.
    const auto& references()const{return humans_;}
    bool enabled()const{return enabled_;} //00462770, constructor00462650 clears native48.
    void disable_for_loading(){ //00462AE0(false) is a genuine early return while48=false.
        if(enabled_)throw std::runtime_error("Detached camera disable requires its actual active toggle/scene restoration receiver");
    }
    const HumanVector& position()const{return position_;}
    void position(const HumanVector& v){position_=v;} //Native20; actual controller movement supplies this slot.
    float amount(Human& human) { //00463690; actual head marker BEFORE actual Human model.
        if(!human.head_marker||!human.base().frame())throw std::runtime_error("Camera amount requires actual Human head and model");
        const auto head=frames_.actor_world_matrix(human.head_marker);
        const auto body=frames_.actor_world_matrix(human.base().frame());
        const HumanVector first{{head[12],head[13],head[14],head[15]}};
        const HumanVector last{{body[12],static_cast<float>(static_cast<long double>(body[13])+.10000000149011612f),body[14],0}};
        float parameter;const auto square=camera_segment_square(position_,first,last,parameter);
        if(square>2.4000000953674316f)return 0;
        const auto stored=static_cast<float>(square);if(stored<.4000000059604645f)return 1;
        return static_cast<float>(1-(static_cast<long double>(stored)-.4000000059604645f)*.5f);
    }
    template<class Setter>void update(Human& human,Setter&& set_amount) { //00463780.
        const auto value=amount(human);
        for(std::size_t n=humans_.size();n;)if(humans_[--n].human==&human) {
            if(humans_[n].amount!=value) {
                set_amount(human,value);
                //Native tests OLD cached amount AFTER notification. New zero
                //sticks until a subsequent different amount removes the row.
                if(humans_.at(n).amount==0)humans_.erase(humans_.begin()+static_cast<std::ptrdiff_t>(n));
                else humans_[n].amount=value;
            }
            return;
        }
        if(value!=0){humans_.push_back({&human,value});set_amount(human,value);}
    }
    template<class Setter>void remove(Human& human,Setter&& set_amount) { //00463040.
        for(std::size_t n=humans_.size();n;)if(humans_[--n].human==&human) {
            set_amount(human,0.f);
            //Only actor pointer is copied. Keep the removed slot's old amount.
            humans_.at(n).human=humans_.back().human;humans_.pop_back();return;
        }
    }
    template<class Setter>void reset_amounts(bool enabled,Setter&& set_amount) { //00463620, false only.
        if(enabled)return;
        for(auto n=humans_.size();n;)set_amount(*humans_[--n].human,0.f);
        humans_.clear(); //Retain capacity; destruction itself has NO Human callbacks.
    }
    template<class Players,class Setter>void update_players(Players& players,Setter&& set_amount) { //00463890, selected records then roster, both reverse.
        const auto& selected=players.selection().records();
        for(auto n=selected.size();n;)update(*selected.at(--n)->human,set_amount);
        auto n=players.human_roster().size();
        while(n)update(*players.human_roster().at(static_cast<std::int32_t>(--n)),set_amount);
    }
private:
    Frames& frames_;std::vector<Reference> humans_;Owner camera_; //Frame release BEFORE vector destruction, as native.
    HumanVector position_{};float speed_=1;bool enabled_=false; //Position cold guard is not original constructor credit; no active toggle producer.
};
} //namespace hd2::game
#endif
