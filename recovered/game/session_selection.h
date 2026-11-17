#ifndef HD2_GAME_SESSION_SELECTION_H
#define HD2_GAME_SESSION_SELECTION_H
#include "frontend/pre_mission_handoff.h"
#include "game/session_owner.h"
namespace hd2::game {

//Read-only bridge for006989B0/006B8EC0. Preparation's actual route owns the
//selected cursor; the accepted request owns its root/mode metadata. Borrow both
//and the SAME loaded GameData rather than using StartupData's independently
//loaded, stale E8 default or introducing another selected mission authority.
class SessionSelection {
public:
    SessionSelection(frontend::GameData& data,const frontend::PreMissionRoute& route,
                     const frontend::PreMissionGameRequest& request)
        :field_e8(request.root),data_(data),route_(route),request_(request){(void)selected_mission_ordinal(field_e8);}
    SessionSelection(frontend::GameData& preparation_data,const frontend::PreMissionRoute& route,
                     frontend::GameData& application_data,const MissionRequest& request)
        :field_e8(request.network?application_data.field_e8:request.preparation.root),
         data_(request.network?application_data:preparation_data),route_(route),request_(request.preparation),accepted_(&request){
        if(request.network&&(&request.network->data!=&application_data||&request.network->route!=&route))
            throw std::logic_error("Selection requires SAME actual application mission/mode owners");
        (void)selected_mission_ordinal(field_e8);
    }
    std::int32_t mission_count(std::uint32_t root)const{return data_.mission_count(root);}
    std::uint32_t selected_mission_ordinal(std::uint32_t root)const{
        if(accepted_&&accepted_->network){
            validate_mission_request(*accepted_);
            return data_.selected_mission_ordinal(root,route_.standalone());
        }
        const auto location=route_.location();const auto* mission=route_.mission();
        if(root!=field_e8||root!=route_.root_index()||request_.mode!=route_.game_mode()||
           request_.standalone!=route_.standalone()||!location||!mission||!location->mission||
           (!route_.standalone()&&location->mission!=mission)||
           location->mission->text24!=mission->text24||location->mission->word8!=mission->word8||
           mission->text24!=request_.mission_resource||mission->word8!=request_.mission_caption)
            throw std::logic_error("Activation requires SAME actual selected preparation mission and request");
        const auto actual=frontend::find_pre_mission_location(data_,root,location->flat_ordinal);
        if(!actual||actual->mission!=location->mission)throw std::logic_error("Selection must borrow SAME actual loaded GameData");
        return location->flat_ordinal;
    }
    const std::uint32_t& field_e8;
    const MissionRequest* accepted_request()const noexcept{return accepted_;}
private:
    frontend::GameData& data_;const frontend::PreMissionRoute& route_;const frontend::PreMissionGameRequest& request_;
    const MissionRequest* accepted_=nullptr;
};
} //namespace hd2::game
#endif
