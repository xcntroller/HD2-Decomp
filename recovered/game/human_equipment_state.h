#ifndef HD2_GAME_HUMAN_EQUIPMENT_STATE_H
#define HD2_GAME_HUMAN_EQUIPMENT_STATE_H

#include "game/human_joint_owners.h"

namespace hd2::game {

template<class Frame>class HumanEquipmentTransferPeer;

//230 owner's0078C980 state reset and00791290 model-selection receiver.
//Pointers are borrowed from the containing equipment/model owners. This record
//must never reset live ownership; the actual activation/cleanup owns that order.
template<class Frame>
class HumanEquipmentState {
public:
    HumanEquipmentState(){reset();}
    void reset() {
        for(auto& frame:models_)frame=Frame{};
        field4_=nullptr;field0c_=nullptr;
        for(auto offset:{0x34u,0x38u,0x3cu})words_.set(offset,std::int32_t{-1});
        for(auto offset:{0x30u,0x40u,0x44u,0x48u,0x50u,0xd0u,0xbcu,0xc0u,0xccu})words_.set(offset,std::uint32_t{0});
        for(auto offset:{0x6cu,0x70u,0x54u,0xd4u})words_.set(offset,std::int32_t{-10000});
        words_.set(0x2c,std::uint8_t{1});words_.set(0x4c,std::uint8_t{1});words_.set(0xb0,std::uint32_t{11});
    }
    void select(Frame model,std::uint32_t kind,std::uint32_t value,std::int32_t identity,std::int32_t definition) {
        if(words_.template get<std::int32_t>(0x54)<0||identity!=-10000)words_.set(0x54,identity);
        switch(kind) {
        case 0:if(model){models_[3]=model;words_.set(0x44,value);}break;
        case 1:if(model){models_[4]=model;words_.set(0x48,value);words_.set(0x38,definition);}break;
        case 2:if(model){models_[2]=model;words_.set(0x40,value);words_.set(0x3c,definition);}break;
        case 3:
            if(model){models_[0]=model;words_.set(0xd4,std::uint32_t{700});words_.set(0x4c,std::uint8_t{1});
                words_.set(0x30,value);words_.set(0x34,definition);}break;
        case 4:words_.set(0x54,std::int32_t{-9});[[fallthrough]];
        case 5:case 6:case 7:case 9:models_[1]=model;break;
        default:break;
        }
    }
    template<class Frames>void bind_markers(Frames& frames,Frame model) { //0078CC90, borrowed model-owned frames.
        volume_b4_=frames.actor_find(model,"HW1",0x20);volume_b8_=frames.actor_find(model,"HW2",0x20);
        if(volume_b4_&&volume_b8_){frames.object_visible(volume_b4_,true);frames.object_visible(volume_b8_,true);}
        markers_[0]=frames.actor_find(model,"d",0xffff);markers_[2]=markers_[0];
        markers_[1]=frames.actor_find(model,"f",0xffff);
        markers_[3]=frames.actor_find(model,"u",0xffff);markers_[4]=frames.actor_find(model,"q",0xffff);
    }
    Frame model(unsigned index) const{return models_.at(index);}
    Frame selected_model(unsigned kind) const { //00791370; no mutation or attachment calls.
        switch(kind) {
        case 0:return models_[3];case 1:return models_[4];case 2:return models_[2];case 3:return models_[0];
        case 4:case 5:case 6:case 7:case 9:return models_[1];default:return Frame{};
        }
    }
    void model(unsigned index,Frame frame){models_.at(index)=frame;}
    auto& words(){return words_;}
    const auto& words() const{return words_;}
    const auto& markers() const{return markers_;}
    Frame first_volume() const{return volume_b4_;}
    Frame second_volume() const{return volume_b8_;}
    HumanEquipmentTransferPeer<Frame>* departing()const{return field4_;}
    HumanEquipmentTransferPeer<Frame>* arriving()const{return field0c_;}
    void departing(HumanEquipmentTransferPeer<Frame>* value){field4_=value;}
    void arriving(HumanEquipmentTransferPeer<Frame>* value){field0c_=value;}
private:
    HumanJointWords<0xd8> words_;
    HumanEquipmentTransferPeer<Frame>* field4_=nullptr; //Actual Human230 peer borrowers, not native DWORDs.
    HumanEquipmentTransferPeer<Frame>* field0c_=nullptr;
    std::array<Frame,5> models_{},markers_{}; //18/1C/20/24/28 and74/78/7C/80/84.
    Frame volume_b4_{},volume_b8_{};
};

//The containing Human's SAME230 owner implements these endpoints. Native4/0C
//borrow another Human, whose actual230 is used at the animation handoff. Keep
//that lifetime explicit instead of putting host pointers into native scalars.
template<class Frame>class HumanEquipmentTransferPeer {
public:
    virtual ~HumanEquipmentTransferPeer()=default;
    virtual HumanEquipmentState<Frame>& transfer_state()=0;
    virtual std::int32_t transfer_life()const=0;
    virtual void remove_inventory_model(unsigned)=0;
    virtual void attach_inventory_model(unsigned)=0;
};
} //namespace hd2::game
#endif
