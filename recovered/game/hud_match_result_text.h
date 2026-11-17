#ifndef HD2_GAME_HUD_MATCH_RESULT_TEXT_H
#define HD2_GAME_HUD_MATCH_RESULT_TEXT_H
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <memory>
namespace hd2::game {
inline std::string hud_match_result_score_line(const char* prefix,std::uint32_t first,std::uint32_t second,const char* suffix){ //004AD650's exact0083B16C format/argument order.
    const auto signed_word=[](std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;};
    std::array<char,512> buffer{};
    const auto count=std::snprintf(buffer.data(),buffer.size(),"%s     %d  :  %d     %s",prefix?prefix:"",signed_word(second),signed_word(first),suffix?suffix:"");
    if(count<0||static_cast<std::size_t>(count)>=buffer.size())throw std::length_error("HUD result exceeds native formatter buffer");
    return std::string(buffer.data(),static_cast<std::size_t>(count));
}
template<class Factory>std::shared_ptr<const std::string> make_hud_match_result_line(Factory& factory,std::uint32_t first,std::uint32_t second){
    const auto suffix=factory.hud_localized(0xa3d),prefix=factory.hud_localized(0xa3c);
    //Return line is constructed BEFORE prefix/suffix retirement; both release
    //before the caller appends line41, matching004AD650's COW owner ordering.
    return std::make_shared<const std::string>(hud_match_result_score_line(prefix?prefix->c_str():nullptr,first,second,suffix?suffix->c_str():nullptr));
}
}
#endif
