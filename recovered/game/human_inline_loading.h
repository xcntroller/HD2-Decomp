#ifndef HD2_GAME_HUMAN_INLINE_LOADING_H
#define HD2_GAME_HUMAN_INLINE_LOADING_H

#include "game/human_look_control.h"
#include "game/human_camera_amount.h"
#include <memory>

namespace hd2::game {

//005AB330/005ABA90: inline98's actual E0/DD0/230 lifetime. Factory constructs
//the recovered concrete DD0/230 owners with shared world resources; it supplies
//no synthetic successful state. Publish this owner on Human only when the
//containing actor/world activation and its callback lookup are reconstructed.
template<class Human,class Control,class Equipment,class Factory>
class HumanInlineLoading {
public:
    using Look=HumanLookControl<Control>;
    explicit HumanInlineLoading(Factory& factory):factory_(factory) {}
    ~HumanInlineLoading(){clear();}
    HumanInlineLoading(const HumanInlineLoading&)=delete;
    HumanInlineLoading& operator=(const HumanInlineLoading&)=delete;
    void initialize(Human& human,std::uint32_t property) {
        if(human_||look_||control_||equipment_)throw std::logic_error("Human inline98 initialized twice");
        human_=&human;property_=property;
        try {
            look_=std::make_unique<Look>();
            control_=factory_.make_human_control();
            if(!control_)throw std::runtime_error("Cannot allocate required Human DD0");
            equipment_=factory_.make_human_equipment(human,*control_,*look_);
            if(!equipment_)throw std::runtime_error("Cannot allocate required Human230");
            look_->bind(*control_);look_->reset(property_);
            auto& resources=factory_.human_collision_resources();auto& sweep=factory_.human_collision_sweep();
            const auto* tree=factory_.human_collision_tree();
            control_->bind(human,resources,sweep,tree);control_->activate(resources,sweep,tree);
            equipment_->activate(human);look_->command(0,nullptr);flag14_=false;
            const auto mode=factory_.mode_flags();
            //005AA790: native authority bit1 and stance!=1 exempt nonplayer.
            if((mode&7)!=7&&factory_.human_current_player_id()!=human.base().id10()&&
                    !((mode&1u)&&human.properties.field19c!=1))control_->physical().enable(false);
            initialized_=true;
        }catch(...){clear();throw;}
    }
    void clear() noexcept { //005ABA90: deactivate230,DD0; destroyE0,DD0,230.
        if(equipment_)equipment_->deactivate();
        if(control_)control_->deactivate();
        look_.reset();control_.reset();equipment_.reset();human_=nullptr;initialized_=false;
    }
    Human* human()const{return human_;}Look* look()const{return look_.get();}
    Control* control()const{return control_.get();}Equipment* equipment()const{return equipment_.get();}
    bool initialized()const{return initialized_;}
    auto scene_sector()const { //005AC4C0: SAME DD0+CE4/CD0 sector, no scene query.
        if(!control_)throw std::logic_error("Human sector requires actual DD0 owner");
        return control_->animation().sector();
    }
    void apply_camera_amount(float amount) { //00786650 through the SAME DD0/230 owners.
        if(!initialized_||!control_||!equipment_)throw std::logic_error("Human camera amount requires live inline98");
        control_->apply_camera_amount(amount,equipment_->state());
    }
    bool first_person_weapon_suppressed()const { //005AC400 reads SAME inline10/E0+50, not an item flag.
        if(!initialized_||!look_)throw std::logic_error("First-person weapon gate requires live Human E0");
        return (look_->state().template get<std::uint32_t>(0x50)&1u)!=0;
    }
    void enable_animation_control(bool enabled) { //005AC070→007870D0, same owned DD0/230.
        if(!initialized_||!control_||!equipment_)throw std::logic_error("Human animation control requires live inline98");
        control_->enable_animation_control(enabled);
        if(control_->animation_state_word(0xa30)==1)(void)equipment_->state().selected_model(9);
    }
    void look_command(std::uint32_t mode,const HumanVector* target) { //005ABF60, command5 receiver.
        constexpr std::array<std::uint32_t,9> flags{{0,0x86,0x92,6,0x12,0xc2,0xa2,0x85,0x91}};
        if(mode>=flags.size())return;
        if(!initialized_||!look_)throw std::logic_error("Human look command requires live inline98");
        look_->command(flags[mode],mode<7?nullptr:target);
    }
    void add_control_flags(std::uint32_t mask) { //005AC390, actual owned DD0.
        if(!initialized_||!control_)throw std::logic_error("Human control flags require live inline98");
        control_->physical().add_control_flags(mask);
    }
    void clear_control_flags(std::uint32_t mask) { //005AC3E0→0075C770.
        if(!initialized_||!control_||!look_)throw std::logic_error("Human control flags require live inline98");
        control_->physical().clear_control_flags(mask,*look_,factory_.human_collision_sweep());
    }
private:
    Factory& factory_;Human* human_=nullptr;std::uint32_t property_=0;
    std::unique_ptr<Look> look_;std::unique_ptr<Control> control_;std::unique_ptr<Equipment> equipment_;
    bool flag14_=false,initialized_=false;
};
} //namespace hd2::game
#endif
