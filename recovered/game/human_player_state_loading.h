#ifndef HD2_GAME_HUMAN_PLAYER_STATE_LOADING_H
#define HD2_GAME_HUMAN_PLAYER_STATE_LOADING_H

#include "game/human_input_loading_manager.h"
#include "game/human_input_callbacks.h"
#include "game/human_state_base_loading.h"
#include "game/human_formation.h"

namespace hd2::game {

//005A3F20 case1 and ordinary0053B9A0/0053BC50 loading state. The actual
//first-person/controlled-actor receivers remain mandatory Presentation
//dependencies below; no implementation of those dependencies or successful
//application activation is supplied here. State3 handover requires its actual
//camera and saved-state getters; offline7 itself is a required supported mode.
template<class Frames,class Human,class Inputs>
class HumanPlayerStateLoading : public HumanStateBaseLoading<Frames,Human,Inputs> {
public:
    using Base=HumanStateBaseLoading<Frames,Human,Inputs>;
    HumanPlayerStateLoading(Frames& frames,Inputs& inputs,HumanInputCallbacks& callbacks)
        :Base(frames,inputs,1),inputs_(inputs),callbacks_(callbacks) {
        state_.set(0x4e4,std::uint32_t{3}); //Inline4E0 type3 command, no runtime producer exposed.
    }
    ~HumanPlayerStateLoading(){detach_callbacks();}
    template<class Services>void bind_sight_services(Services& services) {
        if(bound_)throw std::logic_error("Cannot replace a bound player's Sight receivers");
        sight_known_=[&services](Human& actor){return services.player_knows_enemy(actor);};
        sight_add_=[&services](Human& actor){services.player_add_enemy(&actor);};
        sight_remove_=[&services](Human& actor){services.player_remove_enemy(&actor);};
        sight_speech_=[this,&services](unsigned code){services.queue_player_ai_speech(*this,code,5000);}; //Required00542060.
    }
    template<class Inline,class Camera,class Presentation>
    void bind(Human& human,const Base* previous,MissionNavigation* navigation,std::uint32_t mode,
              Inline& control,Camera& camera,Presentation& presentation) {
        Base::bind(human,previous,navigation,mode);
        bound_mode_=mode;
        try {
            for(auto base:{0xc4u,0x1c4u,0x2c4u,0x3c4u})
                for(unsigned n=0;n!=64;++n)state_.set(base+n*4,std::uint32_t{0});
            for(auto offset:{0x4d0u,0x4ccu,0x4c8u,0x4c4u,0x540u,0x528u})state_.set(offset,std::uint32_t{0});
            state_.set(0xc0,std::uint8_t{0});state_.set(0x548,std::uint32_t{12});
            if(previous&&previous->words().template get<std::uint32_t>(0x48)==3) {
                auto direction=this->words().template get<HumanVector>(0x10);
                (void)camera.query_direction(3,direction); //00465B40 virtual68; native ignores false.
                this->words().set(0x10,direction);this->words().set(0x20,direction);
                const auto transfer=previous->player_handover();
                state_.set(0x528,transfer.flags);state_.set(0xc0,std::uint8_t(transfer.posture));state_.set(0x548,transfer.action);
            }else camera.set_direction(this->words().template get<HumanVector>(0x10)); //00465EA0.
            control.look_command(0,nullptr); //005A45F0→005ABD90 command5/mode0→005ABF60.
            if((mode&7u)!=7u){
                auto* input=required_input();auto* history=input->history(1);
                if(!history)throw std::runtime_error("Player Human input has no required history1");
                input->first_receiver=&callbacks_;input->first_owner=this;
                input->second_receiver=&callbacks_;input->auxiliary_receiver=&callbacks_;input->second_owner=this;
                input->word38=7;input->flag40=false;
                if(mode&1u)inputs_.player_authority_mode();else input->word3c=0;
                //0053BB99..0053BBC5 copies all four actor30 words ONLY outside7.
                history->position_value.seed(human.base().world_position());
            }
            for(auto offset:{0x524u,0x550u,0x538u,0x4d4u})state_.set(offset,std::uint8_t{0});
            for(auto offset:{0x530u,0x520u,0x53cu})state_.set(offset,std::uint32_t{0});
            state_.set(0x525,std::uint8_t{1});this->words().set(0x70,std::uint8_t{0});
            state_.set(0x4d8,std::uint32_t{3500});control.clear_control_flags(6);
            state_.set(0x52c,std::int32_t{-1});
            presentation.set_player_mode(*this,human,0,mode); //Actual005469E0, including required first-person consumers.
            clear_human_formation(human); //Actual005A91A0(0), SAME typed2C8/2CC owners.
            bound_=true;
        }catch(...){detach_callbacks();throw;}
    }
    template<class Presentation>
    void close(std::uint32_t mode,Presentation& presentation) {
        auto* human=this->human();if(!human)throw std::logic_error("Player state close has no bound Human");
        presentation.leave_player_controlled_car(*human,mode); //Actual native160/type4/164 driver gate→004D9280(0,0).
        if((mode&7u)==7u){
            presentation.offline_player_close(*human); //004563E0 getter→actual0089AC30 virtual34/004593E0.
        }else{
            auto* input=required_input();
            //0053BC50 unconditionally resets owners, unlike AI destruction's guard.
            input->first_receiver=&callbacks_;input->first_owner=nullptr;
            input->second_receiver=&callbacks_;input->auxiliary_receiver=&callbacks_;input->second_owner=nullptr;
            input->word38=0;
        }
        presentation.set_player_mode(*this,*human,0,mode); //Actual005469E0, even when its mode is already zero.
        this->clear_human_reference();bound_=false; //Do not call base close or remove shared input.
    }
    auto& player_words(){return state_;}const auto& player_words()const{return state_;}
    std::uint32_t derived_loading_word(unsigned offset)const override {
        if(offset!=0x528&&offset!=0x548)throw std::runtime_error("Unrecovered player handover word");
        return state_.template get<std::uint32_t>(offset);
    }
    bool bound()const{return bound_;}
    bool selection_suppressed()const{return this->words().template get<std::uint8_t>(0x70)!=0;} //005A4240 virtual64.
    void sight_enemy_acquired(Human& actor)override { //005421C0; no AI event producer.
        if((bound_mode_&7u)!=7u)return;
        require_sight_services();
        if(!sight_known_(actor)) {
            auto* human=this->human();if(!human)throw std::logic_error("Player Sight has no bound Human");
            const auto a=actor.base().world_position(),b=human->base().world_position();
            const float x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];
            const auto square=(static_cast<long double>(x)*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z;
            sight_speech_(square>=400.f?0x5cu:0x5eu); //008101E0, before Players insertion.
        }
        sight_add_(actor);
    }
    void sight_enemy_lost(Human& actor)override { //005422B0; remove BEFORE known query.
        if((bound_mode_&7u)!=7u)return;
        require_sight_services();sight_remove_(actor);if(!sight_known_(actor))sight_speech_(0x60u);
    }
    //Dead00583190 genuinely inherits the base RET4 receiver.
private:
    void require_sight_services()const {
        if(!sight_known_||!sight_add_||!sight_remove_||!sight_speech_)
            throw std::runtime_error("Player Sight requires actual Players/00542060 receivers");
    }
    typename Inputs::Record* required_input()const {
        auto* input=this->input();
        if(!input||!inputs_.contains(input))throw std::runtime_error("Player state input record has been retired");
        return input;
    }
    void detach_callbacks()noexcept {
        auto* input=this->input();if(!input||!inputs_.contains(input))return;
        if(input->first_owner==this){input->first_receiver=nullptr;input->first_owner=nullptr;}
        if(input->second_owner==this){input->second_receiver=nullptr;input->auxiliary_receiver=nullptr;input->second_owner=nullptr;}
    }
    Inputs& inputs_;HumanInputCallbacks& callbacks_;HumanJointWords<0x560> state_;bool bound_=false;
    std::uint32_t bound_mode_=0;
    std::function<bool(Human&)> sight_known_;
    std::function<void(Human&)> sight_add_,sight_remove_;
    std::function<void(unsigned)> sight_speech_;
};
} //namespace hd2::game
#endif
