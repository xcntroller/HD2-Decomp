#ifndef HD2_GAME_HUMAN_HUD_LOADING_H
#define HD2_GAME_HUMAN_HUD_LOADING_H
#include "game/human_camera_state.h"
#include "game/first_person_human_items.h"
#include "frontend/human_store.h"
#include "frontend/pre_mission_data.h"
#include <cmath>
#include <optional>
#include "frontend/control_startup.h"
#include "frontend/game_configuration.h"
namespace hd2::game {
//005A8380 offline7: native profile24, distinct from profile rank44/stat array.
template<class Human>std::uint32_t offline_human_hud_rank(const Human& human){return human.profile_record?human.profile_record->field24:0u;}
//005A81C0 offline7: user14 else LAST04, unlike005A82C0's short10 fallback.
//Without B8 use the actual model's E8 name rather than the Actor name override.
template<class Frames,class Human>std::string offline_human_hud_name(Frames& frames,const Human& human){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->last:profile->user_name;
    return frames.frame_name(human.base().frame());
}
//005A8320 offline7 uses profile0C FULL name, unlike005A81C0 LAST04 and
//005A82C0 SHORT10. No actor/model borrower survives this string acquisition.
template<class Frames,class Human>std::string offline_human_target_name(Frames& frames,const Human& human){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->full_name:profile->user_name;
    return frames.frame_name(human.base().frame());
}
//Human008111E0+04=005A9980 lies outside the frozen inventory. Publish into
//the EXISTING Actor80 name owner; this helper receives no new function credit.
template<class Frames,class Human,class Localize,class PeerName>const std::string& human_target_caption(Frames& frames,Human& human,std::uint32_t mode,Localize&& localize,PeerName&& peer_name){
    const auto kind=human.properties.field19c;
    if(kind==1||kind==4||kind==5){
        human.base().name((mode&7u)!=7u&&kind==1?peer_name(human):offline_human_target_name(frames,human));
    }else if(human.properties.field1a8!=0xffffffffu)human.base().name(localize(human.properties.field1a8));
    else if(kind==2)human.base().name(localize(0x1bef));
    else if(kind==3)human.base().name(localize(0x1bfd));
    return human.base().name();
}
template<class Human,class Locks>std::uint32_t network_human_hud_rank(const Human& human,
        frontend::ControlStartupState& peers,Locks& locks){ //005A8380, copy ALL nine words under418, return copied54 AFTER unlock.
    std::array<std::uint32_t,9> values;
    locks.enter_control_lock(0);
    try{const auto& peer=peers.records.at(static_cast<std::uint32_t>(human.base().id10()));
        for(unsigned n=0;n!=9;++n)values[n]=peer.word(0x40+4*n);
    }catch(...){locks.leave_control_lock(0);throw;}
    locks.leave_control_lock(0);return values[5];
}
template<class Human>std::string network_human_hud_short_name(const Human& human,const frontend::ControlStartupState& peers){ //005A82C0.
    if(human.properties.field19c==1){const auto id=static_cast<std::uint32_t>(human.base().id10());return id<peers.records.size()?peers.records[id].name():std::string{};}
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->short_name:profile->user_name;
    return human.base().name();
}
template<class Human>std::uint32_t network_human_hud_side(const Human& human,const frontend::GameConfiguration& configuration,
        const frontend::ControlStartupState& peers){ //005A8820(false), SAME configuration predicate/peer64 as damage.
    if(configuration.remote_ai_enabled()){
        const auto kind=human.properties.field19c;if(kind==2||kind==5)return 0;
        return kind==1||kind==4?1u:2u;
    }
    const auto id=static_cast<std::uint32_t>(human.base().id10());return id<peers.records.size()?peers.records[id].word(0x64):2u;
}
template<class Human,class Items,class Factory,class Text>void bind_network_hud_human_name(Human& human,Items& items,
        Factory& factory,Text& text,std::uint32_t& cached_rank){ //004B3890 reached non7 body, SAME cache/TextState.
    cached_rank=factory.hud_network_human_rank(human);
    bool first=false;
    if(factory.hud_style224()==0){
        auto* storage=human.base().storage();auto* holder=storage?storage->holder(4):nullptr;
        if(!holder)throw std::logic_error("Network HUD rank requires actual Human clothing holder");
        const auto entry=holder->entry();first=entry&&items.runtime_item(entry).definition().fields38[3]==0;
    }else first=factory.hud_network_human_side(human)==1;
    const auto rank=factory.hud_localized((first?0x27a6u:0x27b0u)+cached_rank);
    const auto name=factory.hud_network_human_short_name(human);
    text.text.set(std::make_shared<const std::string>((rank?*rank:std::string{})+" "+name),0x3a,0);
}
template<class Human>std::int32_t human_hud_first_mode(const Human& human){ //005A88B0.
    const auto* state=human.loading_state;
    return !state||state->words().template get<std::uint32_t>(0x48)!=3?-1:state->hud_first_mode();
}
template<class Human>std::int32_t human_hud_second_mode(const Human& human){ //005A88D0.
    const auto* state=human.loading_state;
    return !state||state->words().template get<std::uint32_t>(0x48)!=3?-1:state->hud_second_mode();
}
inline std::int32_t human_hud_signed(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
//0069E070: selected game71/78 Actor, including00475D50 pending reserved
//identities; this is independent of the local peer and current HUD soldier.
template<class Session,class Frames>std::uint32_t hud_selected_objective_identity(const Session& session,Frames& frames){
    if(!session.words.template get<std::uint8_t>(0x71))return 0xffffffffu;
    const auto id=session.words.template get<std::int32_t>(0x78);
    return id!=-1&&frames.scene_actor_command_identity(id)?static_cast<std::uint32_t>(id):0xffffffffu;
}
inline std::int32_t human_hud_truncate(long double value){
    if(!std::isfinite(value)||value< -0x1p63L||value>=0x1p63L)return 0; //Native FTOL indefinite low DWORD.
    return human_hud_signed(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
template<class Human>std::int32_t human_hud_health_percent(const Human& human){ //005A5FF0.
    const auto current=human_hud_signed(human.properties.stats_ec[0]);if(!current)return 0;
    const auto base=human_hud_signed(human.properties.stats_11c[0]);
    auto percent=static_cast<long double>(current)*100.f/base;
    if(percent>0&&percent<1)percent=1;if(percent>100)percent=100;
    return human_hud_truncate(percent);
}
template<class Human>std::int32_t human_hud_stamina_percent(const Human& human){ //005A62A0.
    if(!human.properties.stats_ec[0]||human.activation.field14c==0.f)return 0;
    return human_hud_truncate(human.activation.field14c/(static_cast<long double>(human_hud_signed(human.properties.stats_ec[2]))*.01f));
}
struct HumanHudProgress {std::int32_t current,maximum,caption;};
//005EC840's real kind37 Actor fields. The actual zone owner supplies this
//in-call read view; UI never constructs a zone, captures it or advances EC.
struct HudZoneProgressData {std::uint32_t state98,caption_a0,duration_a8,elapsed_ec;};
inline HumanHudProgress hud_zone_progress(const HudZoneProgressData& zone){
    const auto maximum=zone.duration_a8*1000u;
    const auto offset=zone.state98==2?zone.duration_a8*500u:0u;
    return {human_hud_signed(maximum-zone.elapsed_ec-offset),human_hud_signed(maximum),human_hud_signed(zone.caption_a0)};
}
//005A9920→00475D50, then005A9830 resolves AGAIN and005EC840 rechecks F1.
template<class Human,class Queries>std::optional<HumanHudProgress> human_remote_hud_progress(Human& human,Queries& queries){
    auto* zone=queries.zone_actor(human_hud_signed(human.activation.field210));
    if(!zone||queries.zone_kind(*zone)!=37||!queries.zone_active(*zone))return std::nullopt;
    zone=queries.zone_actor(human_hud_signed(human.activation.field210));
    if(!zone)throw std::logic_error("HUD zone progress lost its actual Actor between original queries");
    if(!queries.zone_active(*zone))return std::nullopt;
    return hud_zone_progress(queries.zone_data(*zone));
}
template<class Human,class Queries>std::optional<HumanHudProgress> human_hud_progress(Human& human,std::uint32_t mode,Queries& queries){ //005A9830.
    queries.validate_human(human);
    if(human.field160)switch(queries.attachment_kind(human)){
    case 4:case 20:case 21:case 34:
        if(queries.attachment_has_control(human))return queries.attachment_progress(human);
        break;
    default:break;
    }
    if(human.activation.field17c)return HumanHudProgress{human_hud_signed(human.activation.field17c),human_hud_signed(human.activation.field180),human_hud_signed(human.activation.field184)};
    if((mode&7u)==7u)return std::nullopt;
    return queries.remote_progress(human); //Actual005A9920/00475D50/005EC840 producer is required outside offline7.
}
//005A8160/005AC410 ->0075E610. Read the SAME DD0 retained bone0/1
//(native810/81C), not the geometry owner's separate feet. This samples pose;
//it neither advances Human simulation nor caches borrowed model/bone frames.
template<class Human,class Frames,class Bones>bool human_hud_movement_side(Human& human,Frames& frames,const Bones& bones){
    if(!human.base().frame()||!bones[0]||!bones[1])throw std::logic_error("HUD movement sample requires actual model and DD0 bones0/1");
    std::array<float,3> predicted{};
    for(unsigned n=0;n!=3;++n)predicted[n]=static_cast<float>(static_cast<long double>(human.activation.vector1b0[n])*2);
    const auto model=frames.actor_world_matrix(human.base().frame());
    for(unsigned n=0;n!=3;++n)predicted[n]=static_cast<float>(static_cast<long double>(predicted[n])+model[12+n]);
    const auto first=frames.actor_world_matrix(bones[0]);const auto second=frames.actor_world_matrix(bones[1]);
    const auto ax=static_cast<long double>(predicted[0])-first[12],ay=static_cast<long double>(predicted[1])-first[13],az=static_cast<long double>(predicted[2])-first[14];
    const auto bx=static_cast<long double>(predicted[0])-second[12],by=static_cast<long double>(predicted[1])-second[13];
    const float bz=static_cast<float>(static_cast<long double>(predicted[2])-second[14]);
    return (static_cast<long double>(bz)*bz+by*by)+bx*bx>=(az*az+ay*ay)+ax*ax;
}
//005A8100. The B0 metric is a read-only publication, not a new aim producer.
//All seven non-Player native tables route EC to genuine zero00524DA0;
//Player00811848 uses00542B10 and SAME Human FC skill (stats_ec[4]).
template<class Human,class Items,class Players>long double human_hud_weapon_spread(Human& human,Items& items,Players& players){
    (void)players.inline_control(human); //Validate actual per-Human lifetime before holder access.
    FirstPersonHumanItems<Items> holders(items);const auto entry=holders.weapon(human);if(!entry)return 0;
    const auto& definition=items.runtime_item(entry).definition();if(definition.kind()==6)return 0;
    const auto& payload=definition.payloads[0];float weapon=0;
    if(payload.type==3||payload.type==4){
        const auto offset=payload.type==3?0x40u:0x3cu; //Native44/40; serialization omits vtable DWORD.
        if(payload.bytes.size()<offset+4)throw std::runtime_error("HUD weapon spread requires actual payload scalar");
        weapon=frontend::pre_mission_detail::Bytes::real(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+offset));
    }else if(payload.type!=1&&payload.type!=2)throw std::runtime_error("HUD weapon spread requires its original payload virtual10 receiver");
    long double state_spread=0;const auto& state=players.state(human).words();
    if(state.template get<std::uint32_t>(0x48)==1){
        const auto flags=state.template get<std::uint32_t>(0x50);
        state_spread=(flags&4u)?.25f:(flags&2u)?.1f:.05f;
        if(flags&0xf8u){if(flags&0x1000u)state_spread+=.1f;else if(flags&0x2000u)state_spread+=.5f;else if(flags&0x4000u)state_spread+=.75f;}
        const auto bits=human.properties.stats_ec[4];std::int32_t skill;std::memcpy(&skill,&bits,4);
        state_spread+=(100.L-skill)*.01f;
    }
    const float contribution=static_cast<float>(static_cast<long double>(weapon)*.01f);
    return state_spread+contribution;
}
} //namespace hd2::game
#endif
