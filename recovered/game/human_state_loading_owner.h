#ifndef HD2_GAME_HUMAN_STATE_LOADING_OWNER_H
#define HD2_GAME_HUMAN_STATE_LOADING_OWNER_H

#include <memory>
#include <cstdint>
#include <stdexcept>
#include <exception>
#include <type_traits>
#include <utility>

namespace hd2::game {

//005A3F20 ownership/publication and005A3E60 selection. Factory must construct
//the actual derived source, bind its real dependencies, dispatch its own+20,
//publish the SAME borrowed B0 and report cleanup failure. No fallback states.
//The containing Human/world outlive this owner; explicit retire precedes inline98.
template<class Human,class Factory>class HumanStateLoadingOwner {
public:
    using Base=typename Factory::Base;
    HumanStateLoadingOwner(Human& human,Factory& factory):human_(human),factory_(factory) {
        static_assert(noexcept(factory_.publish_human_state(human_,static_cast<Base*>(nullptr))),
            "Human B0 publication must be a nonthrowing borrower update");
    }
    ~HumanStateLoadingOwner(){try{retire();}catch(...){factory_.publish_human_state(human_,nullptr);factory_.record_state_cleanup_failure(std::current_exception());}}
    HumanStateLoadingOwner(const HumanStateLoadingOwner&)=delete;
    HumanStateLoadingOwner& operator=(const HumanStateLoadingOwner&)=delete;
    Base* get()const{return current_.get();}bool ready()const{return current_&&!failed_&&!binding_;}
    void change(std::uint32_t kind) {
        if(failed_)throw std::logic_error("Human B0 must retire a failed state before another bind");
        if(binding_)throw std::logic_error("Human B0 loading cannot replace its in-progress handover");
        if(current_&&current_->words().template get<std::uint32_t>(0x48)==kind)return;
        switch(kind){case 1:case 3:case 4:case 5:case 6:case 7:case 8:break;
            default:throw std::runtime_error("Human B0 kind has no native factory case");}
        auto next=factory_.make_human_state(kind);
        if(!next||next->words().template get<std::uint32_t>(0x48)!=kind)
            throw std::runtime_error("Human B0 factory did not create its actual requested state");
        previous_=std::move(current_);current_=std::move(next);binding_=true;
        factory_.publish_human_state(human_,current_.get()); //NEW B0 visible to native005A80D0 during bind.
        try {
            factory_.bind_human_state(*current_,human_,previous_.get());
            if(previous_){factory_.close_human_state(*previous_);previous_.reset();}
            binding_=false;
        }catch(...){binding_=false;failed_=true;throw;} //Retain both owned lifetimes for explicit failure cleanup.
    }
    template<class Inline>void select_player(bool player,std::uint32_t mode,std::int32_t local_player,Inline& control) { //005A3E60.
        if((mode&7u)!=7u) {
            const bool local=human_.base().id10()==local_player;
            change(local||(mode&1u)?1u:7u);control.enable_animation_control(local);return;
        }
        if(!player){change(3);control.enable_animation_control(false);return;}
        if(!ready())throw std::runtime_error("Offline player selection requires its actual previous state");
        if(current_->words().template get<std::uint32_t>(0x48)==3)factory_.reset_active_ai_orders(*current_); //virtual128(0).
        change(1);factory_.player_select_human(human_); //00461410→0045DA60, before005AC070.
        control.enable_animation_control(true);
    }
    template<class Inline>void select_initial(bool has_script_context,std::uint32_t mode,std::int32_t local_player,Inline& control) { //0059F2C0's B0 selection ONLY.
        if(has_script_context){change(5);return;}
        const auto kind=human_.properties.field19c;
        if(kind==1){select_player(false,mode,local_player,control);return;}
        if(kind<2||kind>5)throw std::runtime_error("Human kind has no native initial B0 selection");
        const bool remote=(mode&7u)!=7u&&!(mode&1u)&&factory_.human_remote_ai_enabled(); //006B66B0, exact short circuit.
        change(remote?7u:kind==2?4u:6u);
    }
    void retire() {
        //0059FA60 closes before deleting and clears the published pointer last.
        if(current_){factory_.close_human_state(*current_);current_.reset();}
        factory_.publish_human_state(human_,nullptr);
        if(previous_){factory_.close_human_state(*previous_);previous_.reset();}
        failed_=false;
    }
private:
    Human& human_;Factory& factory_;std::unique_ptr<Base> current_,previous_;bool failed_=false,binding_=false;
};
} //namespace hd2::game
#endif
