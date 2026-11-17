#ifndef HD2_GAME_HUMAN_PLAYER_LIVE_INPUT_H
#define HD2_GAME_HUMAN_PLAYER_LIVE_INPUT_H
#include "game/human_ai_shot_direction.h"
#include "game/human_frame_inputs.h"
#include "game/human_state_frame_timers.h"
#include <utility>
namespace hd2::game {
namespace player_live_input {
constexpr float pi=3.1415927410125732f,tau=6.2831854820251465f,limit=1.0471975803375244f;
inline float wrap_stored_yaw(long double angle){
    float stored=static_cast<float>(angle);
    if(angle<0)stored+=tau;else if(stored>tau)stored-=tau;
    return stored;
}
//The two body-reference normalizations retain X/Z in x87 until FPATAN.
inline long double body_yaw(const HumanVector& v){
    long double x=v[0],z=v[2];
    const auto square=(x*x+z*z)+static_cast<long double>(v[1])*v[1];
    if(!(std::abs(square-1.f)<9.99999993922529e-09f)){
        const float stored=static_cast<float>(square);
        if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;z*=inverse;}
        else if(x!=0)x=x<0?-1.f:1.f;else if(z!=0)z=z<0?-1.f:1.f;
    }
    auto angle=std::atan2(x,z);if(angle<0)angle+=tau;else if(angle>tau)angle-=tau;return angle;
}
inline float clamp_yaw(float wanted,const HumanVector& body){
    const auto base=body_yaw(body);auto delta=static_cast<long double>(wanted)-base;
    if(delta>pi)delta-=tau;else if(delta<-pi)delta+=tau;
    if(delta>limit)return static_cast<float>(base+limit);
    if(delta<-limit)return static_cast<float>(base-limit);
    return wanted;
}
inline bool retained_blocked_action(unsigned index){ //0053CD64 jump table, index bias21.
    switch(index){case 21:case 22:case 23:case 24:case 26:case 27:case 28:case 29:case 30:
    case 31:case 35:case 36:case 37:case 38:case 46:case 47:case 48:case 62:return true;default:return false;}
}
}
//0053BF90, called directly by005A3DE0, not virtual state frame0053F9B0.
//Player/World own all state and consumers; only the application input loans are
//published here. A missing positive external receiver fails at its native call.
template<class Player,class World,class Frames>
void process_live_player_input(Player& player,std::uint32_t* first_words,std::uint32_t* second_words,World& world,Frames& frames){
    using namespace player_live_input;
    player.bind_application_inputs(first_words,second_words); //BOTH74/78 stores before either NULL test.
    if(!first_words||!second_words)return;
    //Keep original arguments distinct from fresh native74/78 reloads after
    //callbacks; a callback may publish different Player input borrowers.
    HumanFrameInputBuffer first_argument,second_argument;
    first_argument.borrow(first_words);second_argument.borrow(second_words);
    auto& words=player.words();auto& state=player.player_words();
    const auto human=[&]()->auto&{auto* p=player.human();if(!p)throw std::logic_error("Player input reached native missing Human44");return *p;};
    const auto suppressed=[&]{return words.template get<std::uint8_t>(0x70)!=0;};
    const auto blocked=[&]{return (words.template get<std::uint32_t>(0x50)&0x800u)||suppressed();};
    const auto first=[&]()->auto&{auto* p=player.first_frame_inputs();if(!p)throw std::logic_error("Player input lost actual74 loan");return *p;};
    const auto second=[&]()->auto&{auto* p=player.second_frame_inputs();if(!p)throw std::logic_error("Player input lost actual78 loan");return *p;};
    if(!world.player_camera_input_suppressed()&&((world.mode_flags()&7u)==7u||(world.player_input_view_toggle_allowed()&&!suppressed()))&&second_argument.template get<float>(0x68)!=0.f)
        world.toggle_player_input_view(player);
    if(world.player_camera_input_suppressed()||(words.template get<std::int32_t>(0x60)!=-1&&words.template get<std::int32_t>(0x64)>0)){
        for(unsigned n=64;n;){--n;if(!retained_blocked_action(n)){second_argument.set(n*4,0u);first_argument.set(n*4,0u);}}return;
    }
    float mouse_x=first_argument.template get<float>(0xd0),mouse_y=first_argument.template get<float>(0xd4);
    if(world.player_input_mouse_inverted())mouse_y=-mouse_y;
    const auto forward=[&](auto frame){const auto m=frames.actor_world_matrix(frame);return HumanVector{{m[8],m[9],m[10],m[11]}};};
    const auto set_direction=[&](auto frame){
        auto direction=forward(frame);
        const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2];
        normalize_human_ai_shot(direction,square);words.set(0x10,direction);
    };
    bool attachment_handled=false;
    if(!blocked())if(auto* attachment=human().field160){
        const auto kind=attachment->base().type();
        if(kind==4||kind==20||kind==21||kind==34){
            const auto seat=human().field164;const auto flags=world.player_attachment_control_flags(human());
            if(flags&1u){mouse_x=mouse_y=0;set_direction(human().base().frame());}
            if(flags&0x100u){
                if(world.player_input_mouse_inverted())first_argument.set(0xd4,-first_argument.template get<float>(0xd4));
                world.human_emplacement_frame_inputs(human().field160,seat,first_argument,second_argument);
                if(world.player_input_mouse_inverted())first_argument.set(0xd4,-first_argument.template get<float>(0xd4));
                attachment_handled=true;
            }else if(flags&0x10u){
                const auto frame=world.player_input_attachment_frame(human(),seat);if(frame)set_direction(frame);
                attachment_handled=true;
            }
        }
    }
    const auto car_clamped=[&]{auto* a=human().field160;return a&&a->base().type()==4&&(world.player_attachment_control_flags(human())&0x20u);};
    if(attachment_handled||(mouse_x==0.f&&mouse_y==0.f)){
        const auto direction=words.template get<HumanVector>(0x10);
        const float horizontal=static_cast<float>(std::sqrt(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0]));
        auto yaw=wrap_stored_yaw(std::atan2(static_cast<long double>(direction[0]),static_cast<long double>(direction[2])));
        if(car_clamped()){
            yaw=clamp_yaw(yaw,forward(human().base().frame()));
            words.set(0x10,static_cast<float>(std::sin(static_cast<long double>(yaw))*horizontal));
            words.set(0x18,static_cast<float>(std::cos(static_cast<long double>(yaw))*horizontal));
        }
    }else if(mouse_x!=0.f||mouse_y!=0.f){
        const auto mode=state.template get<std::int32_t>(0x52c);
        const float scale=mode==0||state.template get<std::int32_t>(0x530)>0?.30000001192092896f:mode==2?.10000000149011612f:.05000000074505806f;
        mouse_x*=scale;mouse_y*=scale;
        const auto direction=words.template get<HumanVector>(0x10);
        float horizontal=static_cast<float>(std::sqrt(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0]));
        float yaw=static_cast<float>(std::atan2(static_cast<long double>(direction[0]),static_cast<long double>(direction[2])));
        if(mouse_x!=0.f){
            yaw=static_cast<float>(static_cast<long double>(mouse_x)*pi+yaw);
            if(yaw<0)yaw+=tau;else if(yaw>tau)yaw-=tau;
            const bool car_limit=car_clamped(); //Native Car flags query precedes005AC2B0.
            if(world.player_input_animation_blocked(human())||car_limit)yaw=clamp_yaw(yaw,forward(human().base().frame()));
        }
        if(mouse_y!=0.f){
            const auto extended=std::atan2(static_cast<long double>(direction[1]),static_cast<long double>(horizontal))-static_cast<long double>(mouse_y)*pi;
            float pitch=static_cast<float>(extended);
            if(extended>1.3962634801864624f)pitch=1.3962634801864624f;else if(pitch<-1.3962634801864624f)pitch=-1.3962634801864624f;
            //53CA24: unordered selects the candidate, as do the subsequent
            //masked x87 flags; preserve NaN rather than min/max substitution.
            const float upper=1.5676548480987549f,lower=-1.5676548480987549f;
            const float capped=pitch>upper?upper:pitch;const float selected=lower>capped?lower:capped;
            words.set(0x14,static_cast<float>(std::sin(static_cast<long double>(selected))));
            horizontal=static_cast<float>(std::cos(static_cast<long double>(selected)));
        }
        words.set(0x10,static_cast<float>(std::sin(static_cast<long double>(yaw))*horizontal));
        words.set(0x18,static_cast<float>(std::cos(static_cast<long double>(yaw))*horizontal));
    }
    words.set(0x20,words.template get<HumanVector>(0x10));
    world.player_input_camera_direction(words.template get<HumanVector>(0x10)); //AFTER all four20 stores.
    if(blocked())return; //Fresh flags after camera callback.
    if(second().template get<float>(0x88)!=0.f&&world.player_item_controls_available()&&!state.template get<std::uint8_t>(0x538))
        world.player_input_drop_hand(player); //Actual fresh slot3/cooldown/admission and005A2C50 request, not immediate damage-drop.
    for(const auto pair:{std::pair<unsigned,unsigned>{0x40,0x4c4},{0x18,0x4c8},{0x1c,0x4cc},{0x34,0x4d0}})
        if(second().template get<float>(pair.first)!=0.f)state.set(pair.second,1u);
    if(second().template get<float>(0x4c)!=0.f)player.change_breath_hold(true,world);
    if(state.template get<std::uint8_t>(0x4d4)&&first().template get<float>(0x4c)==0.f)player.change_breath_hold(false,world);
    if(second_argument.template get<float>(0x10)>0.f){ //53CBF5..53CC1F: one step down, not unconditional prone.
        const auto posture=words.template get<std::uint32_t>(0x5c);
        if(posture==0x10000u)words.set(0x5c,0x4000u);
        else if(posture==0x4000u)words.set(0x5c,0x2000u);
        else if(posture==0x2000u)words.set(0x5c,0x1000u);
    }
    else if(second_argument.template get<float>(0x14)!=0.f){
        const auto posture=words.template get<std::uint32_t>(0x5c);
        if(posture==0x1000u)words.set(0x5c,0x2000u);
        else if(posture==0x2000u)words.set(0x5c,0x4000u);
        else if(posture==0x4000u&&(words.template get<std::uint32_t>(0x50)&4u)&&
                static_cast<long double>(human_frame_signed_word(human().properties.stats_ec[2]))*.10000000149011612f<human().activation.field14c)words.set(0x5c,0x10000u);
    }
    if(human().field174&&(words.template get<std::uint32_t>(0x5c)&0x14000u))words.set(0x5c,0x2000u);
    if(world.player_input_animation_blocked(human())||world.human_inline(human()).frame_navigation_floor()){
        const auto posture=words.template get<std::uint32_t>(0x5c);
        if(posture==0x1000u)words.set(0x5c,0x2000u);else if(posture==0x10000u)words.set(0x5c,0x4000u);
    }
}
}
#endif
