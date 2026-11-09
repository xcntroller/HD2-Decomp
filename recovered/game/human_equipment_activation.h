#ifndef HD2_GAME_HUMAN_EQUIPMENT_ACTIVATION_H
#define HD2_GAME_HUMAN_EQUIPMENT_ACTIVATION_H

#include "game/human_equipment_state.h"
#include "game/human_equipment_action.h"
#include "game/human_holsters.h"
#include <optional>

namespace hd2::game {

//0078CA60 and0078DE30, including offline action retirement. The Human owns initialized
//0078CE20 holsters,230 state/attachments, actual item module and DD0/6950.
//All references outlive this activation owner. No world readiness is supplied.
template<class Frames,class Items,class Control,class Holsters,class Attachments>
class HumanEquipmentActivation {
public:
    using Frame=typename Frames::Frame;
    using LightOwner=decltype(std::declval<Frames&>().create_human_equipment_light());
    HumanEquipmentActivation(Frames& frames,Items& items,Control& control,Holsters& holsters,
            Attachments& attachments,HumanEquipmentState<Frame>& state)
        :frames_(frames),items_(items),control_(control),holsters_(holsters),attachments_(attachments),state_(state) {}
    ~HumanEquipmentActivation(){deactivate();}
    HumanEquipmentActivation(const HumanEquipmentActivation&)=delete;
    HumanEquipmentActivation& operator=(const HumanEquipmentActivation&)=delete;
    template<class Human>void activate(Human& human) {
        if(started_)throw std::logic_error("Human equipment activation repeated while live");
        const auto model=human.base().frame();auto* storage=human.base().storage();
        if(!model||!storage||storage->single())throw std::runtime_error("Human equipment requires actual Human model/storage");
        started_=true;
        try {
            state_.reset();light_.emplace(frames_.create_human_equipment_light());
            if(!*light_)throw std::runtime_error("Cannot create required Human equipment light");
            frames_.human_equipment_name(light_->get(),"m_SpecFrameLight");state_.bind_markers(frames_,model);
            const auto entry=[&](unsigned index) {
                auto* holder=storage->holder(index);
                if(!holder)throw std::runtime_error("Human equipment holder is absent");
                return holder->entry();
            };
            if(const auto handle=entry(3)) {
                auto& item=items_.runtime_item(handle);select(item,3);
                control_.reset_equipment_command();
                control_.set_equipment_command(item.definition().equipment_command(),false);
                storage->holder(3)->field18(control_.game_word28()+1000u); //007D68B0.
                if(human_item_holster_group(item.definition())==0)
                    holsters_.show_items(item.definition(),1,control_.equipment_opacity());
            }
            for(const auto binding:{std::pair<unsigned,unsigned>{2,2},{5,1}})if(const auto handle=entry(binding.first)) {
                auto& item=items_.runtime_item(handle);select(item,binding.second);attachments_.attach(binding.second);
            }
            if(const auto handle=entry(0)) {
                auto& item=items_.runtime_item(handle);
                state_.select(item.model(),0,static_cast<std::uint32_t>(item.identity),-10000,-1);attachments_.attach(0);
            }
            auto* stacks=storage->slots(1);
            if(!stacks)throw std::runtime_error("Human holster storage is absent");
            for(const auto& stack:stacks->slots())if(!stack.entries.empty()) {
                if(stack.entries.size()>0x7fffffffu)throw std::length_error("Human holster stack exceeds original signed range");
                const auto& definition=items_.runtime_item(stack.entries.front()).definition();
                holsters_.show_items(definition,static_cast<std::int32_t>(stack.entries.size()),control_.equipment_opacity());
            }
        }catch(...){deactivate();throw;}
    }
    void deactivate() noexcept {
        if(!started_)return;
        attempt([&]{toggle_human_equipment_action(frames_,state_,light(),control_.mode_flags(),false);});
        light_.reset();
        attempt([&]{control_.set_equipment_weapon(Frame{},-1);});
        attachments_.clear_parts();
        attempt([&]{retire_human_equipment_action_model(frames_,state_,control_.mode_flags());});
        holsters_.clear();started_=false;
    }
    Frame light() const{return light_?light_->get():Frame{};}
private:
    template<class Item>void select(Item& item,unsigned kind) {
        state_.select(item.model(),kind,static_cast<std::uint32_t>(item.identity),-10000,
            static_cast<std::int32_t>(item.definition().identifier));
    }
    template<class Work>void attempt(Work&& work) noexcept {
        try{work();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    Frames& frames_;Items& items_;Control& control_;Holsters& holsters_;Attachments& attachments_;
    HumanEquipmentState<Frame>& state_;std::optional<LightOwner> light_;bool started_=false;
};
} //namespace hd2::game
#endif
