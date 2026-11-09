#ifndef HD2_GAME_ACTOR_MATERIAL_H
#define HD2_GAME_ACTOR_MATERIAL_H
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
namespace hd2::game {
//00611090: case-sensitive first substring, atoi after "matid"; absent=-1.
//Unlike Ladder it neither lowercases the text nor requires a numeric suffix.
inline std::int32_t actor_material_id(const std::string& text) {
    const auto at=text.find("matid");if(at==std::string::npos)return -1;
    const auto value=std::strtoll(text.c_str()+at+5,nullptr,10);
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Actor material ID exceeds native atoi bounds");
    return static_cast<std::int32_t>(value);
}
} //namespace hd2::game
#endif
