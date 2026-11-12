#ifndef HD2_GAME_PLAYERS_PROCESS_H
#define HD2_GAME_PLAYERS_PROCESS_H

#include "game/human_formation.h"
#include <array>
#include <vector>

namespace hd2::game {

//Actual CRT-owned0089AC30 (00455A20→00456780), distinct from module11's
//embedded control1C. Only known scalar writes and typed loading consumers are
//exposed. Native unwritten fields/W lanes use guards, not constructor credit.
template<class Human>class PlayersProcess {
public:
    struct DelayedSpeech {Human* human;std::int32_t due;}; //Separate process0089AC20 vector, not cleared by00456440.
    PlayersProcess(){reset();}
    PlayersProcess(const PlayersProcess&)=delete;PlayersProcess& operator=(const PlayersProcess&)=delete;
    void reset() { //00456440; does NOT clear five group buffers or delayed speech.
        for(auto offset:{0x3du,0x3fu,0x42u,0x3eu,0x41u,0x40u,0x3cu})words_.set(offset,std::uint8_t{0});
        for(auto offset:{0x38u,0x34u,0x30u,0x98u,0x9cu})words_.set(offset,std::uint32_t{0});
        words_.set(0xa0,std::int32_t{-1});words_.set(0xa4,std::uint32_t{0});
        words_.set(0x120,std::int32_t{3});update_formation_scale();
        words_.set(0x43,std::uint8_t{0});words_.set(0x44,std::uint32_t{5000});
    }
    void update_formation_scale() { //004563F0; signed integer→FLOAT before x87 multiply/add, one final FLOAT store.
        const float count=static_cast<float>(words_.template get<std::int32_t>(0x120));
        const float scale=static_cast<float>(static_cast<long double>(count)*.75f+1.5f);
        for(auto offset:{0x110u,0x114u,0x118u})words_.set(offset,scale);
    }
    HumanVector formation_scale()const{return words_.template get<HumanVector>(0x110);} //Actual0089AD40.
    const auto& groups()const{return groups_;}auto& words(){return words_;}const auto& words()const{return words_;}

    template<class Active,class Control>void first_player_shot(Control& control,std::uint32_t mode){ //00459360, absent frozen.
        auto* selected=control.selected_player(mode);
        for(auto n=control.human_roster().size();n;){
            auto* human=control.human_roster().at(static_cast<std::int32_t>(--n));
            if(!human)throw std::runtime_error("First Player shot lost its actual roster Human");
            if(signed_word(human->properties.stats_ec[0])<=0||human==selected)continue;
            active<Active>(*human).first_player_shot();
        }
    }

    template<class Active,class Control>void offline_player_close(Human& departing,Control& control) { //004593E0, singleton virtual34.
        //Count and entry are reread each iteration; formation callbacks can mutate the roster.
        for(std::size_t n=0;n<control.human_roster().size();++n) {
            auto* human=control.human_roster().at(static_cast<std::int32_t>(n));
            if(!human||human->formation_leader!=&departing||state_kind(*human)!=3)continue;
            auto& state=active<Active>(*human);
            state.disable_follow_loading(state.follow_immediate()); //005377F0(0, actual188).
            clear_human_formation(*human); //005A91A0(0), AFTER follow reset.
        }
    }
    template<class Active,class Frames,class Control,class Speech>
    void tick(std::uint32_t elapsed,std::uint32_t mode,Frames& frames,Control& control,
              std::uint32_t game_time,std::vector<DelayedSpeech>& delayed,Speech& speech) { //004564A0.
        expire_byte(0x30,0x3c,elapsed);expire_byte(0x34,0x40,elapsed);
        if(expire(0x9c,elapsed))words_.set(0x98,std::uint32_t{0});
        for(auto& group:groups_)group.clear(); //0045C860 whole-range erase, retain all five allocations.
        auto* selected=control.selected_player(mode);if(!selected)return;
        for(auto n=control.human_roster().size();n;) {
            auto* human=control.human_roster().at(static_cast<std::int32_t>(--n));
            if(!human)throw std::runtime_error("Players process roster lost its Human borrower");
            if(signed_word(human->properties.stats_ec[0])<=0||human==selected||state_kind(*human)!=3)continue;
            const auto from=frames.actor_world_matrix(selected->base().frame());
            const auto to=frames.actor_world_matrix(human->base().frame());
            const auto x=static_cast<long double>(to[12])-from[12],y=static_cast<long double>(to[13])-from[13],z=static_cast<long double>(to[14])-from[14];
            const float square=static_cast<float>((z*z+y*y)+x*x); //Native x87 order, FLOAT store before comparison0080FB20.
            auto& state=active<Active>(*human);const bool pending=state.has_pending_orders_loading();
            const bool available=!pending&&state.process_available();
            if(square<1600.f||pending)groups_[0].push_back(human); //0045CA80 append, no dedup.
            if(available&&square<1600.f)groups_[1].push_back(human);
            if(pending)groups_[2].push_back(human);
        }
        //Native A0!=-1 calls00456EF0, a verified genuine RET4.
        const auto now=signed_word(game_time+elapsed);
        for(auto n=delayed.size();n;) {
            --n;if(delayed[n].due>now)continue;
            auto* human=delayed[n].human;
            if(!human)throw std::runtime_error("Delayed Players speech lost its actual Human");
            speech.queue_active_ai_speech(active<Active>(*human),0x7fu,1u,300u,0u); //Required005383D0, before erase.
            if(n>=delayed.size())throw std::runtime_error("Players speech callback invalidated its native erase index");
            delayed.erase(delayed.begin()+static_cast<std::ptrdiff_t>(n));
        }
    }
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    static std::uint32_t state_kind(const Human& human) {
        if(!human.loading_state)throw std::runtime_error("Players process requires actual Human B0");
        return human.loading_state->words().template get<std::uint32_t>(0x48);
    }
    template<class Active>static Active& active(Human& human) {
        auto* state=dynamic_cast<Active*>(human.loading_state);
        if(!state)throw std::runtime_error("Players process requires actual active AI state");
        return *state;
    }
    bool expire(unsigned offset,std::uint32_t elapsed) {
        const auto timer=words_.template get<std::uint32_t>(offset);if(!timer)return false;
        const auto next=timer-elapsed;words_.set(offset,next);
        if(signed_word(next)>0)return false;
        words_.set(offset,std::uint32_t{0});return true;
    }
    void expire_byte(unsigned timer,unsigned flag,std::uint32_t elapsed){if(expire(timer,elapsed))words_.set(flag,std::uint8_t{0});}
    HumanJointWords<0x130> words_;std::array<std::vector<Human*>,5> groups_;
};
} //namespace hd2::game
#endif
