#ifndef HD2_GAME_HUMAN_STATE_BASE_LOADING_H
#define HD2_GAME_HUMAN_STATE_BASE_LOADING_H

#include "game/human_joint_owners.h"
#include "game/human_loading_state_view.h"
#include "game/mission_navigation.h"
#include <functional>

namespace hd2::game {

//00582FF0/005832F0/00583690/00584830 B0 base, including offline mode7. The containing
//derived state owns its +1C/+20 receivers; this base is NOT a successful AI,
//player or script state. Input records and navigation are actual shared owners.
//Native44/40/7C/A0/A4/A8 pointers stay typed, outside scalar DWORD storage.
template<class Frames,class Human,class Inputs>
class HumanStateBaseLoading : public HumanLoadingStateView<Human> {
public:
    using PlayerHandover=typename HumanLoadingStateView<Human>::PlayerHandover;
    using Entry=std::decay_t<decltype(std::declval<Human&>().base().storage()->holder(3)->entry())>;
    using Command=decltype(std::declval<Human&>().command.get());
    using Input=typename Inputs::Record;
    HumanStateBaseLoading(Frames& frames,Inputs& inputs,std::uint32_t kind)
        :frames_(frames),inputs_(inputs) { //00582FF0, including byte-sized writes.
        for(auto offset:{0x10u,0x14u,0x20u,0x24u,0x30u,0x34u,0x50u,0x74u,0x78u,0xbcu})
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
    std::uint32_t derived_loading_word(unsigned)const override{throw std::runtime_error("Human state has no recovered derived handover word");}
    PlayerHandover player_handover()const override{throw std::runtime_error("Human state has no recovered player handover");}
    void sight_enemy_acquired(Human&)override{} //00583170, genuine RET4 base receiver.
    void sight_enemy_lost(Human&)override{} //00583180.
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
                0x20u,0x24u,0x28u,0x2cu,0xb0u,0x60u,0x64u,0x68u,0x6cu})
                words_.set(offset,previous->words_.template get<std::uint32_t>(offset));
            words_.set(0x50,words_.template get<std::uint32_t>(0x50)&0x3fffffffu);
            for(auto offset:{0xacu,0xadu})words_.set(offset,previous->words_.template get<std::uint8_t>(offset));
        }else {
            words_.set(0x50,std::uint32_t{0x2104});words_.set(0x60,std::int32_t{-1});words_.set(0x6c,std::int32_t{-1});
            words_.set(0xad,std::uint8_t{0});words_.set(0xac,std::uint8_t{0});words_.set(0xb0,std::uint32_t{10000000});
            const auto matrix=frames_.actor_world_matrix(model);
            const auto direction=normalize({{matrix[8],matrix[9],matrix[10],matrix[11]}});
            words_.set(0x10,direction);words_.set(0x20,direction);
        }
        for(auto offset:{0x54u,0x74u,0x78u})words_.set(offset,std::uint32_t{0});
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
        if(words_.template get<std::uint8_t>(0xb5)){
            require_music_reference();special_music_reference_(false);words_.set(0xb5,std::uint8_t{0});
        }
        human_=nullptr;
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
    Human* human()const{return human_;}Command command()const{return command_;}Entry item()const{return item_;}
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
    void clear_human_reference()noexcept{human_=nullptr;} //Derived+20 may clear44 without removing shared7C.
private:
    static bool positive_health(const Human& human){
        const auto word=human.properties.stats_ec[0];std::int32_t value;std::memcpy(&value,&word,4);return value>0;
    }
    void require_music_reference()const{if(!special_music_reference_)throw std::runtime_error("Human state4/8 requires actual game special-music reference owner");}
    Frames& frames_;Inputs& inputs_;HumanJointWords<0xc0> words_;
    struct Task;std::vector<Task*> first_,second_; //Native84/94, no unresearched producers.
    Human* human_=nullptr;Command command_=nullptr;Entry item_{};Input* input_=nullptr;
    MissionNavigation* navigation_=nullptr;MissionTableRecord* point_=nullptr;
    std::function<void(bool)> special_music_reference_;
};
} //namespace hd2::game
#endif
