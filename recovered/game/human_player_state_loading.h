#ifndef HD2_GAME_HUMAN_PLAYER_STATE_LOADING_H
#define HD2_GAME_HUMAN_PLAYER_STATE_LOADING_H

#include "game/human_input_loading_manager.h"
#include "game/human_input_callbacks.h"
#include "game/human_state_base_loading.h"
#include "game/human_formation.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_ai_frame_output.h"
#include "game/human_frame_stats.h"
#include "frontend/spatial_sound.h"
#include "game/human_player_weapon_control.h"
#include "game/human_player_item_frame.h"

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
    bool frame_actor_contact(std::uint32_t kind,std::int32_t identity,std::uint32_t mode)override{ //00544B40, absent frozen.
        if(kind==8u&&(mode&2u))this->queue_ladder_contact(identity);
        return true;
    }
    HumanSightMetricFamily sight_metric_family()const override{return HumanSightMetricFamily::player;} //B0+38=00542320,+34 remains RET12.
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
            for(auto& input:local_inputs_)input.bytes.fill(0); //Actual C4/1C4 typed owners.
            for(auto base:{0x2c4u,0x3c4u})
                for(unsigned n=0;n!=64;++n)state_.set(base+n*4,0u);
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
        if(offset!=0x528&&offset!=0x548&&offset!=0x52c&&offset!=0x4d4)throw std::runtime_error("Unrecovered player handover word");
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
    template<class World,class Queries,class Inline,class Runtime>
    void advance_presentation_frame(std::uint32_t elapsed,World& world,Queries& queries,Inline& owner,Runtime& runtime){ //00540090, absent frozen.
        auto* human=this->human();
        if(!human)throw std::logic_error("Player presentation frame requires its bound Human");
        auto& words=this->words();
        words.set(0xbc,words.template get<std::uint32_t>(0xbc)+1u);
        this->process_second_frame_commands(); //Player FC0053F370(1), BEFORE first80 requests.
        this->process_first_frame_commands(); //0053F370(0).
        auto* control=owner.control();auto* look=owner.look();
        if(!control||!look)throw std::logic_error("Player presentation frame requires actual DD0/E0");
        const bool suppressed=words.template get<std::uint8_t>(0x70)!=0;
        control->frame_collision_disabled(suppressed||!human->activation.flag20c,*look);
        const auto mode=world.mode_flags();
        if(suppressed&&(mode&2u)&&world.human_current_player_id()==human->base().id10())
            runtime.refresh_suppressed_player_camera(*this,*human); //Actual World B8/B4 gates and camera virtual48.
        if((words.template get<std::uint32_t>(0x50)&0x800u)||suppressed)return;
        const auto remaining=state_.template get<std::uint32_t>(0x540);
        if(remaining){
            const auto next=remaining-elapsed;
            state_.set(0x540,human_frame_signed_word(next)<0?0u:next);
        }
        const auto command_delay=state_.template get<std::uint32_t>(0x520);
        if(command_delay){
            const auto next=command_delay-elapsed;
            state_.set(0x520,next);
            if(human_frame_signed_word(next)<=0){
                state_.set(0x520,0u);
                runtime.dispatch_player_inline_command(*this); //0053D5B0, Players virtual0 with actual inline4E0 command.
            }
        }
        if(!(world.mode_flags()&1u))this->advance_frame_timers(elapsed,queries);
        if(world.mode_flags()&2u)advance_local_inputs(elapsed,world,owner,runtime); //0053F9B0.
        if(world.mode_flags()&1u){
            this->process_second_frame_commands(); //0053F520 always invokes FC(1) before its offline return.
            if((world.mode_flags()&7u)!=7u&&world.human_current_player_id()!=human->base().id10())
                runtime.advance_player_remote_inputs(*this,elapsed); //Remaining positive0053F520 body.
        }
    }
    template<class World>void change_breath_hold(bool held,World& world){ //0053CD90.
        auto* human=this->human();
        if(!human)throw std::logic_error("Player breath control requires its actual Human");
        const auto emit=[&](std::int32_t entry){
            frontend::SpatialSoundRequest request; //Actual004374D0 defaults and process SOUND54.
            request.position=world.sight_position(human->head_marker);
            request.group=11;request.entry=entry;request.flags=0x41u;
            (void)world.emit_human_equipment_sound(request);
        };
        if(held&&state_.template get<std::int32_t>(0x52c)==3){
            this->refresh_item();
            const auto entry=this->item();
            if(!entry)throw std::runtime_error("Player breath hold reached the native missing-item dereference");
            const auto& item=world.human_frame_output_item(entry);
            if(item.field5c<=world.game_word28()&&item.field14<=world.game_word28()&&
                    state_.template get<std::int32_t>(0x4d8)==3500){
                state_.set(0x4d4,std::uint8_t{1});
                human->activation.field17c=3500;human->activation.field180=3500;human->activation.field184=0x1a57u;
                emit(31);
            }
            return;
        }
        if(!state_.template get<std::uint8_t>(0x4d4))return;
        state_.set(0x4d4,std::uint8_t{0});
        if(state_.template get<std::int32_t>(0x4d8)<2000)state_.set(0x4d8,2000u);
        emit(32);
        human->activation.field17c=human->activation.field180=human->activation.field184=0;
        //0069BFE0 debug format receiver is a genuine RET.
    }
    template<class World,class Inline,class Runtime>
    void advance_local_inputs(std::uint32_t elapsed,World& world,Inline& control,Runtime& runtime){ //0053F9B0.
        auto* human=this->human();
        if(!human)throw std::logic_error("Player local inputs require their actual Human");
        if((world.mode_flags()&7u)!=7u&&world.human_current_player_id()!=human->base().id10()){
            this->process_first_frame_commands();return; //Native common tail still drains FC(0).
        }
        const auto* first=this->first_frame_inputs();const auto* second=this->second_frame_inputs();
        if(!first||!second){
            local_inputs_[0].bytes.fill(0);local_inputs_[1].bytes.fill(0);
            this->bind_frame_inputs(local_inputs_[0],local_inputs_[1]);
        }else{
            local_inputs_[0]=*first;local_inputs_[1]=*second; //Native preserves the existing74/78 pointers in this branch.
        }
        const auto* attachment=human->field160;
        if(attachment&&(attachment->base().type()==4||attachment->base().type()==0x14||
                attachment->base().type()==0x15||attachment->base().type()==0x22))
            runtime.apply_player_attachment_inputs(*this); //Virtual0=00583BC0, actual Car/mounted control receivers.
        else apply_human_ai_frame_movement(*this,control); //SAME00583C30 body, Player's direct00584B60 flag publication.
        //0058C320 is a genuine RET.
        advance_human_endurance(*human,*this,control,elapsed); //00588710.
        advance_player_weapon_readiness(*this,control,world,runtime,elapsed); //00545B90.
        if(!(world.mode_flags()&1u))advance_player_item_frame(*this,control,world,runtime,elapsed); //005433B0.
        const auto breath=state_.template get<std::uint32_t>(0x4d8);
        if(!state_.template get<std::uint8_t>(0x4d4)){
            if(human_frame_signed_word(breath)<3500){
                const auto next=breath+elapsed;
                state_.set(0x4d8,human_frame_signed_word(next)>3499?3500u:next);
            }
        }else{
            const auto next=breath-elapsed;state_.set(0x4d8,next);
            if(human_frame_signed_word(next)<=0){
                state_.set(0x4d8,0u);change_breath_hold(false,world); //0053CD90(0).
            }
        }
        const auto delay=state_.template get<std::uint32_t>(0x530);
        if(human_frame_signed_word(delay)>0){
            const auto next=delay-elapsed;state_.set(0x530,next);
            if(human_frame_signed_word(next)<=0){
                runtime.notify_player_selected_item(*this); //00492130(1, actual item payload+10), before530 clear.
                state_.set(0x530,0u);
            }
        }
        if((world.mode_flags()&7u)!=7u)
            runtime.advance_player_input_transport(*this,elapsed); //Actual history/position/velocity/0073E5A0 branch.
        local_inputs_[0].bytes.fill(0);local_inputs_[1].bytes.fill(0);
        this->process_first_frame_commands(); //FC(0), AFTER both retained local buffer clears.
    }
    template<class World,class Queries,class Inline,class Runtime>
    void advance_authority_frame(std::uint32_t elapsed,World& world,Queries& queries,Inline& owner,Runtime& runtime){ //00540240.
        auto* human=this->human();
        auto* control=owner.control();
        if(!human||!control)throw std::logic_error("Player authority frame requires actual Human/DD0");
        if(auto* input=this->input())
            input->word38=control->animation_state_word(0x1ac)==0&&control->animation_state_word(0x9c0)==0?1u:7u;
        state_.set(0x525,std::uint8_t{0});
        auto& words=this->words();
        if(words.template get<std::uint32_t>(0x50)&0x800u)return;
        this->advance_frame_timers(elapsed,queries);
        constexpr unsigned source_offsets[]={0x4c4,0x4c8,0x4cc,0x4d0};
        constexpr unsigned input_offsets[]={0x40,0x18,0x1c,0x34};
        if(auto* input=this->second_frame_inputs())
            for(unsigned n=0;n!=4;++n){
                const auto value=state_.template get<float>(source_offsets[n]);
                if(value!=0)input->set(input_offsets[n],value);
            }
        advance_player_item_frame(*this,owner,world,runtime,elapsed); //005433B0.
        if(auto* input=this->second_frame_inputs())
            for(unsigned n=0;n!=4;++n)
                if(state_.template get<float>(source_offsets[n])!=0)input->set(input_offsets[n],0u);
        for(auto offset:source_offsets)state_.set(offset,0u);
        const auto matrix=this->state_frames().actor_world_matrix(human->base().frame());
        HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const float square=static_cast<float>((static_cast<long double>(direction[2])*direction[2]+
            static_cast<long double>(direction[0])*direction[0])+static_cast<long double>(direction[1])*direction[1]);
        normalize_human_ai_shot(direction,square);
        words.set(0x20,direction);
        this->process_second_frame_commands(); //FC(1), AFTER actual model direction publication.
    }
    void receive_packet_inputs(const HumanPacketCommand& command){ //00540DB0 case0, SAME C4/1C4 buffers.
        this->queue_frame_command(command,true); //Allocate before publishing, for source allocation-failure retry.
        local_inputs_=command.inputs;
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
    Inputs& inputs_;HumanInputCallbacks& callbacks_;HumanJointWords<0x560> state_;
    std::array<HumanFrameInputBuffer,2> local_inputs_; //Native C4/1C4, outside scalar/pointer storage.
    bool bound_=false;
    std::uint32_t bound_mode_=0;
    std::function<bool(Human&)> sight_known_;
    std::function<void(Human&)> sight_add_,sight_remove_;
    std::function<void(unsigned)> sight_speech_;
};
} //namespace hd2::game
#endif
