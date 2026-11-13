#ifndef HD2_GAME_STEAM_INTEGRITY_CALLS_H
#define HD2_GAME_STEAM_INTEGRITY_CALLS_H

#include "game/integrity_descriptors.h"
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace hd2::game {

template<class Value>struct IntegrityApiArgument {Value value;std::uint32_t word;};

//Pinned Steam image00A8D258. Private0099AEB0 table decoding is not invoked by
//this distribution's retained entry path. Do not apply its retail XOR here.
inline constexpr std::array<IntegrityDescriptor,32> steam_integrity_descriptors{{
    {0x10116203u,0x075ac025u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x11116203u,0x075ac025u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x10110207u,0x0143b042u,0x0004076bu,0xf0f0f0f0u},
    {0x11110207u,0x0143b042u,0x0004076bu,0xf0f0f0f0u},
    {0x10216203u,0x097a5e32u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x11216203u,0x097a5e32u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x10210207u,0x0437e723u,0x000923a1u,0xf0f0f0f0u},
    {0x11210207u,0x0437e723u,0x000923a1u,0xf0f0f0f0u},
    {0x10026203u,0x0320eba0u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x11026203u,0x0320eba0u,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x10020207u,0x056a648fu,0x000702feu,0xf0f0f0f0u},
    {0x11020207u,0x056a648fu,0x000702feu,0xf0f0f0f0u},
    {0x10206203u,0x0a4276bcu,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x11206203u,0x0a4276bcu,0xf0f0f0f0u,0xf0f0f0f0u},
    {0x10200207u,0x01770325u,0x000a1372u,0xf0f0f0f0u},
    {0x11200207u,0x01770325u,0x000a1372u,0xf0f0f0f0u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
    {0xffffff00u,0x00000000u,0x00000000u,0x00000000u},
}};

//Actual Steam initial BSS: key00B3BD70=0, pointer00B3BF78=null. Their only
//known writers belong to dormant0099AEB0. Absence is not a successful validator.
struct SteamIntegrityState {
    std::uint32_t key=0;
    std::unique_ptr<std::array<std::uint32_t,2>> blocked_words;
    bool blocked(std::size_t slot)const noexcept {
        return integrity_descriptor_blocked(key,static_cast<std::uint8_t>(slot),blocked_words.get());
    }
};

enum class SteamValidation {not_requested,skipped_by_distribution};
struct SteamIntegrityResult {
    std::uint32_t actual_api_result=0,returned=0;
    std::size_t descriptor=32;
    SteamValidation validation=SteamValidation::not_requested;
    bool blocked=false;
};
struct SteamIntegritySite {
    std::size_t stage;
    std::uintptr_t caller;
    std::uint32_t marker,second=0;
    bool caller_stores_observation=false;
};

//Original logical DWORD destination token. Never dereference a truncated host
//pointer: scalar C++ storage is bound separately to this matched token.
template<class Tables>
std::uint32_t steam_observation_token(Tables& tables,std::size_t stage){
    return 0x8cae30u+4u*tables.observed_indices().at(stage);
}

template<class Process,class Call>
SteamIntegrityResult dispatch_steam_integrity(Process& process,Call& call,
    SteamIntegritySite site,bool window_prefix,std::uint32_t argument,
    std::uint32_t expected,std::uint32_t token,std::uint32_t* observation){
    IntegrityCallWords words;
    const auto start=window_prefix?1u:0u;
    if(window_prefix)words.arguments[0]=argument;
    words.arguments[start]=site.marker;
    words.arguments[start+1]=site.caller_stores_observation?site.second:token;
    words.arguments[start+2]=expected;
    const auto matched=match_integrity_descriptor(steam_integrity_descriptors,words);
    SteamIntegrityResult result;
    if(!matched)return result;
    result.descriptor=matched.slot;
    result.validation=SteamValidation::skipped_by_distribution; //00997602 ->00997658.
    result.blocked=process.steam_integrity_state().blocked(matched.slot);
    const auto action=static_cast<std::uint8_t>(steam_integrity_descriptors[matched.slot].flags>>8);
    if(action==0x62&&matched.words[1]!=token)
        throw std::logic_error("Steam descriptor lost its actual destination binding");
    if(!call.apply_steam(action,result.blocked,matched.words[2],observation))call.fallback(action);
    return result;
}

template<class Process,class Resolve,class Api>
SteamIntegrityResult steam_integrity_window_call(Process& process,SteamIntegritySite site,
    Resolve resolve,Api api){
    auto call=process.begin_integrity_call(site.caller);
    auto& tables=process.mission_integrity_tables();
    auto* payload=&tables.api_result(site.stage);
    auto* observation=&tables.observed(site.stage);
    const auto expected=tables.expected(site.stage);
    const auto token=steam_observation_token(tables,site.stage);
    const auto argument=resolve();
    call.record_api_argument(payload,argument.word);
    auto result=dispatch_steam_integrity(process,call,site,true,argument.word,expected,token,observation);
    result.actual_api_result=api(argument.value);
    result.returned=call.complete(result.actual_api_result,
        site.caller_stores_observation?nullptr:payload,
        site.caller_stores_observation?observation:nullptr).value;
    return result;
}

//Marker-first calls include no-window probes and IsWindow(marker) probes.
//The callback supplies the exact underlying API arguments, independently of
//the marker words consumed by descriptor matching.
template<class Process,class Api>
SteamIntegrityResult steam_integrity_marker_call(Process& process,SteamIntegritySite site,Api api){
    auto call=process.begin_integrity_call(site.caller);
    auto& tables=process.mission_integrity_tables();
    auto* payload=&tables.api_result(site.stage);
    auto* observation=&tables.observed(site.stage);
    const auto expected=tables.expected(site.stage);
    const auto token=steam_observation_token(tables,site.stage);
    auto result=dispatch_steam_integrity(process,call,site,false,0,expected,token,observation);
    result.actual_api_result=api();
    result.returned=call.complete(result.actual_api_result,
        site.caller_stores_observation?nullptr:payload,
        site.caller_stores_observation?observation:nullptr).value;
    return result;
}

} //namespace hd2::game
#endif
