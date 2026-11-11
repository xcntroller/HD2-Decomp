#ifndef HD2_GAME_HUMAN_EQUIPMENT_LOADING_H
#define HD2_GAME_HUMAN_EQUIPMENT_LOADING_H

#include "game/human_equipment_activation.h"
#include "game/human_equipment_attachments.h"
#include "game/human_equipment_visibility.h"
#include "game/human_equipment_services.h"
#include "game/human_equipment_transfers.h"
#include "game/human_equipment_frame.h"

namespace hd2::game {

//230's ordinary constructor0078CA00, activation0078CA60 and0078DE30/0078CA30
//lifetime. Actual DD0/E0/item/SOUND services remain mandatory; all geometry and
//ownership bodies below are the recovered owners, not callbacks for each stage.
template<class Frames,class Items,class Animations,class Services,class Shadow>
class HumanEquipmentLoading : public HumanEquipmentVisibilityReceiver<typename Frames::Frame> {
public:
    using Frame=typename Frames::Frame;
    using State=HumanEquipmentState<Frame>;
    using Holsters=HumanHolsters<Frames,Animations>;
    using Attachments=HumanEquipmentAttachments<Frames,Items,Services>;
    using Visibility=HumanEquipmentVisibility<Frames,Shadow,Services>;
    using Activation=HumanEquipmentActivation<Frames,Items,Services,Holsters,Attachments>;
    using Transfers=HumanEquipmentTransfers<Frames,Items,Services,Holsters,Attachments>;
    template<class Human>HumanEquipmentLoading(Frames& frames,Items& items,Animations& animations,
            Services& services,Shadow& shadow,const frontend::PreMissionItemCatalogue& catalogue,Human& human)
        :frames_(frames),catalogue_(catalogue),services_(services),holsters_(frames,animations),attachments_(frames,items,services,catalogue,state_),
         visibility_(frames,shadow,services,catalogue,state_),activation_(frames,items,services,holsters_,attachments_,state_),
         transfers_(frames,items,services,holsters_,attachments_,state_) {
        holsters_.initialize(human.base().frame()); //Native constructor loads before DD0 binds/activates.
        services.bind_visibility(*this);
    }
    ~HumanEquipmentLoading(){deactivate();services_.unbind_visibility(*this);} //Remove borrower before members retire.
    HumanEquipmentLoading(const HumanEquipmentLoading&)=delete;
    HumanEquipmentLoading& operator=(const HumanEquipmentLoading&)=delete;
    template<class Human>void activate(Human& human){activation_.activate(human);}
    void deactivate() noexcept {activation_.deactivate();}
    void refresh(Frame model)override{visibility_.refresh(model);}
    auto& state(){return state_;}const auto& state()const{return state_;}
    auto& attachments(){return attachments_;}
    auto& transfers(){return transfers_;}
    void toggle_frame_action(bool enabled){toggle_human_equipment_action(frames_,state_,activation_.light(),services_.mode_flags(),enabled);} //007931B0.
    void set_action_holster_visible(bool visible){holsters_.set_action_holster_visible(visible);} //007918C0, SAME initialized group4.
    void set_frame_weapon_pose(bool ready,bool toggle){ //00791AC0.
        auto& words=state_.words();
        const bool previous=words.template get<std::uint8_t>(0x4c)!=0;
        if(toggle)ready=!previous;
        const auto model=state_.model(0);
        if(ready==previous||!model)return;
        if(previous)transfers_.event(1);
        services_.pose_equipment_weapon(toggle?2u:static_cast<unsigned>(ready));
        if(!previous)frames_.object_position(model,{{0,0,0}});
        frames_.update_scene_object(model);
        if(previous){services_.equipment_primary_face(0);services_.equipment_secondary_face(2);}
        else{services_.equipment_secondary_face(0);services_.equipment_primary_face(2);}
        words.set(0x4c,static_cast<std::uint8_t>(!previous));
    }
    void set_hidden(bool hidden){ //00791C60.
        auto& words=state_.words();
        if(hidden){
            bool incompatible=false;
            const auto identity=words.template get<std::int32_t>(0x30);
            if(identity>=0)if(auto* item=services_.equipment_item(identity)){
                const auto kind=item->definition().kind();
                incompatible=kind<3||(kind>7&&kind!=12);
                if(!incompatible)words.set(0x50,words.template get<std::uint32_t>(0x50)&~1u);
            }
            if(incompatible){
                words.set(0x50,words.template get<std::uint32_t>(0x50)|1u);
                services_.equipment_first_person_request(9);
                if(const auto model=state_.model(0))frames_.object_visible(model,false);
            }else{
                if(words.template get<std::uint32_t>(0x50)&1u)return;
                words.set(0x50,words.template get<std::uint32_t>(0x50)|1u);
                if(services_.equipment_weapon_stowed())transfers_.event(1);
                if(state_.model(0)){
                    services_.equipment_first_person_request(9);
                    attachments_.attach(10);
                }
            }
            services_.set_equipment_weapon(Frame{},-1);
            services_.equipment_transfer_mode(0,0,false);
            services_.force_equipment_command(100,true);
            return;
        }
        if(!(words.template get<std::uint32_t>(0x50)&1u))return;
        words.set(0x50,words.template get<std::uint32_t>(0x50)&~1u);
        if(state_.model(0)){
            attachments_.attach(3);
            if(auto* item=services_.equipment_item(words.template get<std::int32_t>(0x30))){
                const auto command=item->definition().equipment_command();
                services_.force_equipment_command(command,true);
                services_.equipment_transfer_mode(command,3,false);
            }
        }
        if(words.template get<std::uint32_t>(0x50)&2u)transfers_.event(0);
        words.set(0x50,0u);
        if(state_.model(0))services_.equipment_first_person_request(8);
    }
    void advance_frame(std::uint32_t elapsed){advance_human_equipment_frame(frames_,state_,services_,attachments_,transfers_,activation_.light(),catalogue_,elapsed);} //00792CE0.
private:
    Frames& frames_;const frontend::PreMissionItemCatalogue& catalogue_;
    Services& services_;State state_;Holsters holsters_;Attachments attachments_;Visibility visibility_;Activation activation_;Transfers transfers_;
};
} //namespace hd2::game
#endif
