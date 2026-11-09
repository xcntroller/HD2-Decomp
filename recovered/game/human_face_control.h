#ifndef HD2_GAME_HUMAN_FACE_CONTROL_H
#define HD2_GAME_HUMAN_FACE_CONTROL_H

#include "game/human_morph_tables.h"
#include "game/human_joint_owners.h"
#include <optional>

namespace hd2::game {

//DD0+610:00780990,007809C0,00780AD0 and00781250. Owns the loading
//morph state; Human/model children/marker are borrowed. Runtime voice7C and
//speech transition producers are separate dependencies, not invented here.
template<class Frames,class Human,class Random>
class HumanFaceControl {
public:
    using Frame=typename Frames::Frame;
    using SoundOwner=decltype(std::declval<Frames&>().create_human_sound());
    HumanFaceControl(Frames& frames,const HumanMorphTables& tables,Random& random)
        :frames_(frames),tables_(tables),random_(random) {state_.set(8,std::int32_t{1});}
    void bind(Human& human) {
        human_=&human;morph_=Frame{};property_=static_cast<std::int32_t>(human.properties.field1a0);
        state_.set(8,property_);
        const auto model=human.base().frame();
        if(!model)return; //Original does not reset remaining state in this branch.
        for(auto frame=frames_.rigid_first_child(model);frame;frame=frames_.rigid_next_sibling(frame))
            if(frames_.visual_type(frame)==0x524d4d53u){morph_=frame;break;}
        marker_=frames_.actor_find(model,"f",0xffff);
        for(auto offset:{0x4cu,0x64u,0x78u})state_.set(offset,std::int32_t{100});
        for(auto offset:{0x14u,0x18u,0x50u})state_.set(offset,std::int32_t{1});
        for(auto offset:{0x1cu,0x20u,0x28u,0x2cu,0x30u,0x34u,0x3cu,0x44u,0x48u,
            0x54u,0x58u,0x5cu,0x60u,0x68u,0x6cu,0x70u,0x74u})state_.set(offset,std::int32_t{0});
        for(auto offset:{0x24u,0x38u,0x80u})state_.set(offset,std::int32_t{-1});
        state_.set(0x40,std::uint8_t{0});state_.set(0x84,-1.f);
        if(morph_){apply(true);state_.set(0x10,std::uint8_t{0});}
    }
    void schedule(bool enabled) { //00781250; return EAX's incidental quotient is unused.
        if(!enabled)state_.set(0x24,std::int32_t{-1});
        else if(state_.template get<std::int32_t>(0x24)==-1)
            state_.set(0x24,static_cast<std::int32_t>(random_()%human_face_delay_range)+human_face_delay_base);
    }
    void equipment_primary_command(std::uint32_t value){state_.set(0x78,value);} //00781300.
    void equipment_secondary_command(std::uint32_t value){state_.set(0x64,value);} //007812F0.
    template<class Pool>void close_sound(Pool& pool) { //00780AB0, typed native7C owner.
        if(sound_){pool.give(std::move(*sound_));sound_.reset();}
    }
    void apply(bool force) {
        if(!morph_)throw std::runtime_error("Human face has no actual morph visual");
        int current=19,previous=19;
        for(unsigned stage=0;stage!=4;++stage) {
            constexpr std::array<unsigned,4> starts{{0x14,0x28,0x54,0x68}};
            const auto offset=starts[stage];
            const auto duration=state_.template get<std::int32_t>(offset+12);
            if(!duration&&!force)continue;
            auto ratio=duration?static_cast<float>(static_cast<long double>(state_.template get<std::int32_t>(offset+8))/duration):1.f;
            if(ratio>1)ratio=1;
            current=select(stage,state_.template get<std::int32_t>(offset),current,false);
            previous=select(stage,state_.template get<std::int32_t>(offset+4),previous,true);
            const auto& first=tables_.record(property_,current);
            const auto& second=tables_.record(property_,previous);
            if(first.loaded!=1||second.loaded!=1)return; //Aborts ALL later groups.
            if(stage==1&&current==previous)continue;
            constexpr std::array<unsigned,4> groups{{1,0,4,3}};
            const auto group=groups[stage];
            const auto count=frames_.human_morph_count(morph_);
            if(!first.metadata[12+group]||!second.metadata[12+group])continue;
            if(count>8)throw std::runtime_error("Human morph visual exceeds its eight-coefficient group");
            const float complement=1.f-ratio;
            for(std::uint32_t index=0;index<count;++index) {
                const auto slot=group*8+index;
                const auto value=static_cast<float>(static_cast<long double>(ratio)*first.coefficients[slot]+
                    static_cast<long double>(complement)*second.coefficients[slot]);
                frames_.human_morph_weight(morph_,group,index,value);
            }
        }
    }
    const auto& state() const {return state_;}
    Frame morph() const {return morph_;}
    Frame marker() const {return marker_;}
private:
    int select(unsigned stage,int command,int fallback,bool previous) const {
        if(stage==0) {
            switch(command){case 0:return 8;case 1:return 7;case 3:return 4;case 4:return 5;case 5:return 6;default:return fallback;}
        }
        if(stage==1) {
            switch(command) {
            case 0:return 1;
            case 1:
                if(state_.template get<std::int32_t>(0x38)<1&&state_.template get<std::int32_t>(0x3c)<1)return 0;
                return previous?2:3-(state_.template get<std::uint8_t>(0x40)!=0);
            case 2:return 4;case 3:return 5;case 4:return 6;default:return fallback;
            }
        }
        return command>=0&&command<=2?9+static_cast<int>(stage==3)+command*2:fallback;
    }
    Frames& frames_;const HumanMorphTables& tables_;Random& random_;
    Human* human_=nullptr;Frame morph_{},marker_{};std::int32_t property_=1;
    std::optional<SoundOwner> sound_; //No runtime speech producer is exposed by loading.
    HumanJointWords<0x88> state_; //Native pointer slots are represented above.
};
} //namespace hd2::game
#endif
