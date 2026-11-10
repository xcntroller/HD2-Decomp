#ifndef HD2_GAME_HUMAN_PLAYER_MODE_LOADING_H
#define HD2_GAME_HUMAN_PLAYER_MODE_LOADING_H

#include "game/human_camera_state.h"
#include "game/human_holster_lookup.h"
#include "game/mission_integrity_tables.h"
#include "game/session_activation.h"
#include "frontend/game_configuration.h"
#include "frontend/pre_mission_data.h"
#include <type_traits>

namespace hd2::game {

//005469E0's actual player-state mode transition, including offline0..3.
//Borrow SAME FPV, Items, session clock, process guard, application+0C preference
//and live gameBC FOV. The preference is a separate zero-initialized application
//byte, NOT profile options or FPV58. Missing transport still rejects its reached
//queue producer; this source receiver does not enable world/player delivery.
template<class Items,class FirstPerson,class Humans>class HumanPlayerModeLoading {
public:
    HumanPlayerModeLoading(Items& items,FirstPerson& view,Humans& humans,SessionActivationState& session,MissionIntegrityTables& guard,
            const std::uint8_t& preference,const frontend::GameConfiguration& configuration,const float& game_fov,
            const std::int32_t& local_player)
        :items_(items),view_(view),humans_(humans),session_(session),guard_(guard),preference_(preference),configuration_(configuration),game_fov_(game_fov),local_player_(local_player){}
    template<class Player,class Human>void set_player_mode(Player& state,Human& human,std::int32_t next,std::uint32_t mode){
        auto& words=state.player_words();const auto previous=words.template get<std::int32_t>(0x52c);
        if(previous==next)return;
        words.set(0x52c,next); //Native stores BEFORE local-player gating/callbacks.
        if((mode&7u)!=7u&&!human_camera_is_player(human,mode,local_player_))return;
        switch(next){
        case 0:
            if(previous==1){
                view_.secondary_view(mode,false);
                humans_.inline_control(human).action_command(0x18);
                if((mode&7u)!=7u)throw std::runtime_error("Player mode requires actual queued command2 secondary-view notification");
            }else {
                view_.enable(mode,(mode&7u)==7u||configuration_.rules228[9]!=0?preference_!=0:true); //Actual24C.
                view_.set_zoom(mode,false,game_fov_);
            }
            words.set(0x530,std::uint32_t{0});return;
        case 1:{
            const auto entry=find_player_mode_holster(human,items_,14,session_.words.template get<std::uint32_t>(0x28));
            if(!entry){words.set(0x52c,previous);return;}
            view_.secondary_view(mode,true);
            const auto model=items_.runtime_item(entry).model(); //Actual007D3AA0 evaluation BEFORE inline command.
            humans_.inline_control(human).action_command(0x17,model);
            if((mode&7u)!=7u)throw std::runtime_error("Player mode requires actual queued command2 secondary-view notification");
            return;
        }
        case 2:case 3:{
            if((mode&7u)==7u&&guard_.expected(41)!=guard_.observed(41)){words.set(0x52c,previous);return;} //WORD8CAB82/8CB282; no random draw.
            view_.enable(mode,true);
            const auto entry=state.item();if(!entry)throw std::runtime_error("Player zoom requires its actual B0A8 item");
            const auto& payload=items_.runtime_item(entry).definition().payloads[0];
            if(payload.bytes.size()<0x10)throw std::runtime_error("Player zoom item has no actual payload FOV");
            const auto bits=frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+0xc); //Native payload+10, serialized after vtable.
            float fov;std::memcpy(&fov,&bits,4);view_.set_zoom(mode,true,fov);return;
        }
        default:return; //Native unhandled mode stores52C, no callbacks.
        }
    }
private:
    Items& items_;FirstPerson& view_;Humans& humans_;SessionActivationState& session_;MissionIntegrityTables& guard_;
    const std::uint8_t& preference_;const frontend::GameConfiguration& configuration_;const float& game_fov_;const std::int32_t& local_player_;
};

} //namespace hd2::game
#endif
