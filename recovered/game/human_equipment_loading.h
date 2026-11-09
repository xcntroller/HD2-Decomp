#ifndef HD2_GAME_HUMAN_EQUIPMENT_LOADING_H
#define HD2_GAME_HUMAN_EQUIPMENT_LOADING_H

#include "game/human_equipment_activation.h"
#include "game/human_equipment_attachments.h"
#include "game/human_equipment_visibility.h"
#include "game/human_equipment_services.h"
#include "game/human_equipment_transfers.h"

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
        :services_(services),holsters_(frames,animations),attachments_(frames,items,services,catalogue,state_),
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
private:
    Services& services_;State state_;Holsters holsters_;Attachments attachments_;Visibility visibility_;Activation activation_;Transfers transfers_;
};
} //namespace hd2::game
#endif
