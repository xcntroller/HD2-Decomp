#ifndef HD2_GAME_HUMAN_STATE_BASE_LOADING_H
#define HD2_GAME_HUMAN_STATE_BASE_LOADING_H

#include "game/human_joint_owners.h"
#include "game/human_loading_state_view.h"
#include "game/mission_navigation.h"
#include "game/human_state_frame_timers.h"
#include "game/human_frame_inputs.h"
#include "game/human_frame_commands.h"
#include <functional>
#include <optional>

namespace hd2::game {
template<class Backend>class SceneActor;

//00582FF0/005832F0/00583690/00584830 B0 base, including offline mode7. The containing
//derived state owns its +1C/+20 receivers; this base is NOT a successful AI,
//player or script state. Input records and navigation are actual shared owners.
//Native44/40/68/74/78/7C/A0/A4/A8 pointers stay typed, outside DWORD storage.
template<class Frames,class Human,class Inputs>
class HumanStateBaseLoading : public HumanLoadingStateView<Human> {
public:
    using ActionTarget=SceneActor<Frames>;
    using PlayerHandover=typename HumanLoadingStateView<Human>::PlayerHandover;
    using Entry=std::decay_t<decltype(std::declval<Human&>().base().storage()->holder(3)->entry())>;
    using Command=decltype(std::declval<Human&>().command.get());
    using Input=typename Inputs::Record;
    HumanStateBaseLoading(Frames& frames,Inputs& inputs,std::uint32_t kind)
        :frames_(frames),inputs_(inputs) { //00582FF0, including byte-sized writes.
        for(auto offset:{0x10u,0x14u,0x20u,0x24u,0x30u,0x34u,0x50u,0xbcu})
            words_.set(offset,std::uint32_t{0});
        for(auto offset:{0x18u,0x28u,0x38u})words_.set(offset,1.f);
        words_.set(0x48,kind);
        for(auto offset:{0x80u,0x90u})words_.set(offset,static_cast<std::uint8_t>(kind));
        for(auto offset:{0xacu,0xadu,0xb4u})words_.set(offset,std::uint8_t{0});
    }
    HumanStateBaseLoading(const HumanStateBaseLoading&)=delete;
    ~HumanStateBaseLoading()override=default; //Derived owners; native+20 remains an explicit separate call.
    HumanStateBaseLoading& operator=(const HumanStateBaseLoading&)=delete;
    //Typed source handover views, not a native ABI or new function credit.
    HumanVector current_direction()const override{return words_.template get<HumanVector>(0x20);} //005830F0.
    HumanVector target_direction()const override{return words_.template get<HumanVector>(0x10);} //00584B50.
    void set_target_direction(const HumanVector& value){words_.set(0x10,value);} //00583120, all four words; absent frozen.
    std::uint32_t derived_loading_word(unsigned)const override{throw std::runtime_error("Human state has no recovered derived handover word");}
    PlayerHandover player_handover()const override{throw std::runtime_error("Human state has no recovered player handover");}
    void sight_enemy_acquired(Human&)override{} //00583170, genuine RET4 base receiver.
    void sight_enemy_lost(Human&)override{} //00583180.
    void sight_enemy_killed(Human&)override{} //00583160, genuine RET4 base/Remote, absent frozen.
    void sight_dead_acquired(Human&)override{} //00583190.
    void bind_special_music_reference(std::function<void(bool)> receiver){
        if(human_)throw std::logic_error("Cannot replace a bound Human state's music receiver");
        special_music_reference_=std::move(receiver); //Actual0069E920 on SAME game music owner.
    }
    void bind(Human& human,const HumanStateBaseLoading* previous,MissionNavigation* navigation,std::uint32_t mode) {
        if(human_||input_)throw std::logic_error("Human B0 base is already bound");
        human_=&human;command_=human.command.get();
        refresh_item();
        const auto model=human.base().frame();
        if(!model)throw std::runtime_error("Human B0 base requires an actual model");
        frames_.update_scene_object(model);words_.set(0x58,std::uint32_t{0});words_.set(0xb8,std::uint32_t{0});
        navigation_=navigation;
        refresh_navigation_point();
        words_.set(0x5c,std::uint32_t{0x4000});
        if(previous) {
            for(auto offset:{0x50u,0x5cu,0x30u,0x34u,0x38u,0x3cu,0x10u,0x14u,0x18u,0x1cu,
                0x20u,0x24u,0x28u,0x2cu,0xb0u,0x60u,0x64u,0x6cu})
                words_.set(offset,previous->words_.template get<std::uint32_t>(offset));
            action_target_=previous->action_target_; //005832F0 copies native68 identity, not a truncated DWORD.
            words_.set(0x50,words_.template get<std::uint32_t>(0x50)&0x3fffffffu);
            for(auto offset:{0xacu,0xadu})words_.set(offset,previous->words_.template get<std::uint8_t>(offset));
        }else {
            words_.set(0x50,std::uint32_t{0x2104});words_.set(0x60,std::int32_t{-1});words_.set(0x6c,std::int32_t{-1});
            words_.set(0xad,std::uint8_t{0});words_.set(0xac,std::uint8_t{0});words_.set(0xb0,std::uint32_t{10000000});
            const auto matrix=frames_.actor_world_matrix(model);
            const auto direction=normalize({{matrix[8],matrix[9],matrix[10],matrix[11]}});
            words_.set(0x10,direction);words_.set(0x20,direction);
        }
        words_.set(0x54,std::uint32_t{0});first_frame_inputs_=second_frame_inputs_=nullptr;
        words_.set(0x4c,words_.template get<std::uint32_t>(0x50)&7u);words_.set(0xb5,std::uint8_t{0});
        if((mode&7u)==7u){
            input_=nullptr; //Offline7 deliberately has NO network input record.
            const auto kind=words_.template get<std::uint32_t>(0x48);
            if((kind==4||kind==8)&&positive_health(human)){
                require_music_reference();special_music_reference_(true);words_.set(0xb5,std::uint8_t{1});
            }
        }else{
            input_=inputs_.find(human.base().id10(),true);
            if(!input_)throw std::runtime_error("Human B0 has no actual registered input record");
        }
    }
    //Native derived+20 decides whether this is called. AI/player teardown does
    //not implicitly release the shared input; the registry owns its lifetime.
    void close(std::uint32_t mode) { //00583690; derived states decide whether to call it.
        if((mode&7u)!=7u&&input_){inputs_.remove(input_);input_=nullptr;}
        retire_special_music_reference();
        human_=nullptr;
    }
    void retire_special_music_reference(){ //005836E0/00583690, byte reset follows the actual music call.
        if(words_.template get<std::uint8_t>(0xb5)){
            require_music_reference();special_music_reference_(false);words_.set(0xb5,std::uint8_t{0});
        }
    }
    void refresh_item() { //00584830.
        if(!human_)throw std::logic_error("Human B0 item binding has no Human");
        auto* storage=human_->base().storage();
        if(!storage||storage->single()||!storage->holder(3))throw std::runtime_error("Human B0 item binding has no hand holder");
        item_=storage->holder(3)->entry();
    }
    static HumanVector normalize(HumanVector value) { //005832F0's own Y²+Z²+X² order.
        constexpr float epsilon=9.99999993922529e-09f;
        const auto length=(static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2])+
            static_cast<long double>(value[0])*value[0];
        const float stored=static_cast<float>(length);
        if(!std::isfinite(length)||!std::isfinite(stored))throw std::runtime_error("Non-finite Human state direction");
        if(std::abs(length-1)>=epsilon) {
            if(stored>=epsilon) {
                const auto scale=1/std::sqrt(static_cast<long double>(stored));
                for(unsigned i=0;i!=3;++i)value[i]=static_cast<float>(value[i]*scale);
            }else {const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    std::uint32_t first_person_state()const { //005A80F0→00587530, SAME Human B0 byteAD.
        return words_.template get<std::uint8_t>(0xad)?2u:1u;
    }
    HumanJointWords<0xc0>& words()override{return words_;}const HumanJointWords<0xc0>& words()const override{return words_;}
    virtual void publish_frame_command_flags(std::uint32_t command){ //00584B60.
        auto flags=words_.template get<std::uint32_t>(0x50);
        if(!command){words_.set(0x50,flags&0xfcffff07u);return;}
        constexpr std::uint32_t task=0x1e0000u;
        if(command&task){
            const auto requested=command&task;
            flags=(flags&~task)|(requested==task?0u:requested);
        }
        for(auto group:{0x3000000u,0xf8u,0x1f000u,7u})
            if(command&group)flags=(flags&~group)|(command&group);
        words_.set(0x50,flags);
    }
    using FrameCommand=HumanPacketCommand;
    void queue_damage_command(std::uint32_t kind,std::uint32_t hit_kind,std::int32_t health,Human* source,
            const HumanVector& point,const HumanVector& direction){ //0053DB90/00574560, SAME SECOND90.
        if(!human_)throw std::logic_error("Human damage command requires actual B0 Human");
        auto relative=point;const auto origin=human_->base().world_position();
        for(unsigned axis=0;axis!=3;++axis)relative[axis]-=origin[axis];
        FrameCommand command;command.kind=1;auto& f=command.fields;f.set(8,kind);f.set(0xc,hit_kind);
        f.set(0x10,health);f.set(0x14,source?source->base().id10():-1);f.set(0x20,relative);f.set(0x30,direction);
        queue_frame_command(std::move(command),true);
    }
    const auto& first_frame_commands()const{return first_;}
    const auto& second_frame_commands()const{return second_;}
    void queue_frame_command(FrameCommand command,bool second){(second?second_:first_).push_back(std::move(command));}
    void queue_drop_command(std::int32_t actor,std::int32_t item,const HumanVector& direction,float speed){ //00588EF0.
        FrameCommand command;command.kind=10;auto& f=command.fields;f.set(8,item);f.set(0x10,direction);f.set(0x20,speed);f.set(0x24,actor);
        queue_frame_command(std::move(command),true);
    }
    void queue_score_rank(std::uint32_t rank){FrameCommand command;command.kind=16;command.fields.set(8,rank);queue_frame_command(std::move(command),true);} //005470B0.
    bool queue_inventory_move(std::int32_t item,std::int32_t actor,std::int32_t from,std::int32_t to,std::int32_t index){ //0053E610/00592320.
        FrameCommand command;command.kind=7;command.fields.set(8,item);command.fields.set(0xc,actor);
        command.fields.set(0x10,from);command.fields.set(0x14,to);command.fields.set(0x18,index);
        queue_frame_command(std::move(command),false);return true;
    }
    bool queue_inventory_swap(std::int32_t current,std::int32_t current_slot,std::int32_t actor,std::int32_t selected,std::int32_t selected_slot){ //0053E7F0/00592500.
        FrameCommand command;command.kind=8;command.fields.set(8,current);command.fields.set(0xc,current_slot);
        command.fields.set(0x10,actor);command.fields.set(0x14,selected);command.fields.set(0x18,selected_slot);
        queue_frame_command(std::move(command),false);return true;
    }
    bool project_ground_position(HumanVector&)override{return false;} //00583140, genuine AL0/RET4, absent frozen.
    bool frame_actor_contact(std::uint32_t,std::int32_t,std::uint32_t)override{return true;} //00583230, genuine AL1/RET28, absent frozen.
    template<class Services>void bind_frame_command_packets(Services& services){
        frame_packets_=[&services](std::int32_t identity,bool second,const std::vector<FrameCommand>& commands){
            services.publish_human_frame_commands(identity,second,commands);
        };
    }
    void process_first_frame_commands(){process_frame_commands(false);} //00577B30(0), actual B0+84.
    void process_second_frame_commands(){process_frame_commands(true);} //00577B30(1), actual B0+94.
    Human* human()const{return human_;}Command command()const{return command_;}Entry item()const{return item_;}
    void clear_cached_item()noexcept{item_=Entry{};} //005839D0/005783D0 clears A8 AFTER command08, not the hand holder.
    ActionTarget* action_target()const{return action_target_;}
    void action_target(ActionTarget* actor){action_target_=actor;}
    HumanFrameInputBuffer* first_frame_inputs()const{return first_frame_inputs_;}
    HumanFrameInputBuffer* second_frame_inputs()const{return second_frame_inputs_;}
    void bind_frame_inputs(HumanFrameInputBuffer& first,HumanFrameInputBuffer& second){first_frame_inputs_=&first;second_frame_inputs_=&second;}
    template<class Queries>void advance_frame_timers(std::uint32_t elapsed,Queries& queries){advance_human_base_frame_timers(*this,elapsed,queries);} //00588010.
    Input* input()const{return input_;}MissionNavigation* navigation()const{return navigation_;}
    MissionTableRecord* point()const{return point_;}
    void refresh_navigation_point() {
        if(navigation_) {
            if(!human_)throw std::logic_error("Human navigation refresh has no bound actor");
            const auto matrix=frames_.actor_world_matrix(human_->base().frame());
            point_=navigation_->record(navigation_->nearest_volume({{matrix[12],matrix[13],matrix[14]}},5,-1));
        }
    }
protected:
    Frames& state_frames(){return frames_;}
    void queue_ladder_contact(std::int32_t identity){FrameCommand command;command.kind=6;command.actor_identity=identity;first_.push_back(std::move(command));} //00544B40, actual first80 command vector.

    void navigation_point(MissionTableRecord* point){point_=point;} //Actual A4 borrower.
    void clear_human_reference()noexcept{human_=nullptr;} //Derived+20 may clear44 without removing shared7C.
private:
    void process_frame_commands(bool second){
        auto& commands=second?second_:first_;
        if(commands.empty())return;
        if(!human_||!frame_packets_)throw std::logic_error("Human queued command has no actual packet owner");
        frame_packets_(human_->base().id10(),second,commands);
        commands.clear(); //00588C60/00588CC0, only after all actual stream finishes.
    }
    static bool positive_health(const Human& human){
        const auto word=human.properties.stats_ec[0];std::int32_t value;std::memcpy(&value,&word,4);return value>0;
    }
    void require_music_reference()const{if(!special_music_reference_)throw std::runtime_error("Human state4/8 requires actual game special-music reference owner");}
    Frames& frames_;Inputs& inputs_;HumanJointWords<0xc0> words_;
    std::vector<FrameCommand> first_,second_; //Native84/94, typed actual kind6 producer and shared packet serializer.
    Human* human_=nullptr;Command command_=nullptr;Entry item_{};Input* input_=nullptr;
    ActionTarget* action_target_=nullptr; //Borrowed native68. Initial no-previous bind leaves it unwritten in original; cold guard only.
    HumanFrameInputBuffer* first_frame_inputs_=nullptr;HumanFrameInputBuffer* second_frame_inputs_=nullptr; //Actual74/78 borrowers.
    MissionNavigation* navigation_=nullptr;MissionTableRecord* point_=nullptr;
    std::function<void(bool)> special_music_reference_;
    std::function<void(std::int32_t,bool,const std::vector<FrameCommand>&)> frame_packets_;
};
} //namespace hd2::game
#endif
