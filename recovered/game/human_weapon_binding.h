#ifndef HD2_GAME_HUMAN_WEAPON_BINDING_H
#define HD2_GAME_HUMAN_WEAPON_BINDING_H

#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//6950 owner's007B90B0/007B9260. State is the containing owner's SAME movement
//record; model ownership is one retained engine reference, not an item clone.
template<class Frames,class State>
class HumanWeaponBinding {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    HumanWeaponBinding(Frames& frames,State& state):frames_(frames),state_(state) {}
    void clear() noexcept {weapon_.reset();left_=right_=butt_=Frame{};}
    void bind_hands(Frame first,Frame second){first_=first;second_=second;}
    Frame model() const{return weapon_?weapon_->get():Frame{};}
    Frame left() const{return left_;}
    Frame right() const{return right_;}
    Frame butt() const{return butt_;}
    bool stowed()const{return (state_.field62d8&0x80000000u)!=0;}
    void blend(std::int32_t duration){ //007B8D00; signed duration, FLOAT6318, __ftol LOW DWORD.
        if(state_.flag62c8||(state_.field6304&0x38u)||!model())return;
        float factor;std::memcpy(&factor,&state_.field6318,4);
        const auto value=static_cast<long double>(duration)*factor;
        state_.field630c=static_cast<std::uint32_t>(duration);
        state_.field6308=std::isfinite(value)&&value>=-0x1p63L&&value<0x1p63L?static_cast<std::uint32_t>(static_cast<std::int64_t>(value)):0;
        state_.flag62c8=true;state_.field6304=(state_.field6304&0x0fffffc2u)|0x40000001u;
    }
    void stow(bool enabled) { //007B96C0; clearing is completed by the later geometry update.
        if(!model()||stowed()==enabled)return;
        float amount;std::memcpy(&amount,&state_.field631c,4);
        const auto value=static_cast<long double>(amount)*200.f; //0080F6F8.
        std::uint32_t converted=0;
        if(std::isfinite(value)&&value>=-0x1p63L&&value<0x1p63L)
            converted=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
        if(enabled)state_.field62d8|=0x80000000u;
        state_.field6314=converted;state_.field6304|=(enabled?0x04000000u:0x08000000u)|2u;
    }
    void advance_frame(std::uint32_t elapsed){ //007A7470's weapon blend and stow timers.
        auto flags=state_.field6304;
        if(flags&0x40000000u){
            auto next=state_.field6308+elapsed;
            if(human_frame_signed_word(next)>human_frame_signed_word(state_.field630c)){
                next=state_.field630c;state_.field6304&=0xbfffffffu;
            }
            state_.field6308=next;
            const float ratio=static_cast<float>(static_cast<long double>(human_frame_signed_word(next))/human_frame_signed_word(state_.field630c));
            std::memcpy(&state_.field6318,&ratio,4);
        }else if(flags&0x80000000u){
            auto next=state_.field6308-elapsed;
            if(human_frame_signed_word(next)<1){
                next=0;state_.field6304=flags&0x7ffffffau;
                if(model()&&state_.field62cc==4)
                    frames_.car_direction(model(),Vector{{(state_.field62d8&0xffu)?1.f:-1.f,0,0,0}});
            }
            state_.field6308=next;
            const float ratio=static_cast<float>(static_cast<long double>(human_frame_signed_word(next))/human_frame_signed_word(state_.field630c));
            std::memcpy(&state_.field6318,&ratio,4);
        }else if(flags&0x10000000u){
            const auto next=state_.field6310-elapsed;
            state_.field6310=next;
            if(human_frame_signed_word(next)<1){
                state_.field6310=0;
                if(!(flags&0x20000000u))blend(300);
                state_.field6304&=0xcffffffbu;
            }
        }
        flags=state_.field6304;
        bool publish=false;
        if(flags&0x04000000u){
            auto next=state_.field6314+elapsed;
            if(human_frame_signed_word(next)>200){next=200;state_.field6304&=0xfbffffffu;}
            state_.field6314=next;publish=true;
        }else if(flags&0x08000000u){
            auto next=state_.field6314-elapsed;
            publish=true;
            if(human_frame_signed_word(next)<1){
                next=0;state_.field6304&=0xf7ffffffu;state_.field62d8&=0x7fffffffu;
                const auto pose=state_.field62d8&0xffu;
                if(model()&&pose<2){
                    if(!right_)throw std::runtime_error("Weapon stow completion requires its actual RW frame");
                    const auto hand=pose?first_:second_;
                    if(!hand)throw std::runtime_error("Weapon stow completion requires its actual hand frame");
                    frames_.car_direction(model(),Vector{{pose?1.f:-1.f,0,0,0}});
                    const auto source=frames_.actor_world_matrix(right_),weapon=frames_.actor_world_matrix(model());
                    Vector delta{};
                    for(unsigned axis=0;axis!=3;++axis)delta[axis]=weapon[12+axis]-source[12+axis];
                    const auto z=frames_.actor_world_matrix(hand),y=frames_.actor_world_matrix(hand),x=frames_.actor_world_matrix(hand);
                    frames_.object_position(model(),{{dot(delta,x,0),dot(delta,y,4),dot(delta,z,8)}});
                }
            }
            state_.field6314=next;
        }
        if(publish){
            const float ratio=static_cast<float>(static_cast<long double>(human_frame_signed_word(state_.field6314))*.005f);
            std::memcpy(&state_.field631c,&ratio,4);
        }
    }
    void select(Frame frame,std::int32_t kind) { //007B90B0.
        if(model()==frame)return; //Same model leaves even the old kind untouched.
        clear();
        if(!frame) {
            state_.field6304&=0x03fffffcu;state_.field62d8&=0x7fffffffu;state_.flag62c8=false;return;
        }
        weapon_.emplace(frames_.retain_actor_frame(frame));
        left_=frames_.actor_find(frame,"LW",0xffff);right_=frames_.actor_find(frame,"RW",0xffff);
        butt_=frames_.actor_find(frame,"BUTT",0xffff);
        if(left_&&right_)state_.field6304|=2;
        state_.field62dc=kind;
        switch(kind) {
        case 2:state_.field62cc=4;break;
        case 3:state_.field62cc=3;break;
        case 4:case 12:state_.field62cc=5;break;
        case 5:state_.field62cc=6;break;
        case 6:state_.field62cc=7;break;
        default:state_.field62cc=0;break;
        }
        pose(1);
    }
    void pose(unsigned command) { //007B9260:0 second hand,1 first hand,2 toggle low-byte0/1.
        const auto weapon=model();if(!weapon)return;
        if(!right_&&!left_) {state_.field6304=0;state_.flag62c8=false;return;}
        if(command==2) {
            const auto previous=state_.field62d8&0xff;
            command=previous==0?1:previous==1?0:3;
        }
        if(command>1){state_.field6304|=2;return;}
        const auto parent=command?first_:second_;
        if(!parent)throw std::runtime_error("Human weapon pose requires its actual hand marker");
        state_.field62d8=command?(state_.field62d8&0xfffffff1u)|1u:state_.field62d8&0x7ffffff0u;
        frames_.human_equipment_link(weapon,parent,0);
        if(!command||!(state_.field62d8&0x80000000u)) {
            frames_.car_direction(weapon,Vector{{command?1.f:-1.f,0,0,0}});
            frames_.update_scene_object(weapon);
        }
        auto source=right_;
        if(!source&&!command)source=left_;
        Vector delta{};
        if(source) {
            const auto source_world=frames_.actor_world_matrix(source);
            const auto weapon_world=frames_.actor_world_matrix(weapon);
            for(unsigned axis=0;axis!=3;++axis)
                delta[axis]=static_cast<float>(static_cast<long double>(weapon_world[12+axis])-source_world[12+axis]);
        }
        //Native rereads the hand world matrix for each row and uses its
        //transpose rotation, with no translation or scale inversion.
        const auto z=frames_.actor_world_matrix(parent),y=frames_.actor_world_matrix(parent),x=frames_.actor_world_matrix(parent);
        std::array<float,3> local{};
        local[0]=dot(delta,x,0);local[1]=dot(delta,y,4);local[2]=dot(delta,z,8);
        frames_.object_position(weapon,local);state_.field6304|=2;
    }
private:
    template<class Matrix>static float dot(const Vector& delta,const Matrix& matrix,unsigned start) {
        return static_cast<float>((static_cast<long double>(delta[1])*matrix[start+1]+
            static_cast<long double>(delta[2])*matrix[start+2])+static_cast<long double>(delta[0])*matrix[start]);
    }
    Frames& frames_;State& state_;std::optional<Owner> weapon_;
    Frame left_{},right_{},butt_{},first_{},second_{};
};
} //namespace hd2::game
#endif
