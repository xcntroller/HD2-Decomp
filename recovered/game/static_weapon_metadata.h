#ifndef HD2_GAME_STATIC_WEAPON_METADATA_H
#define HD2_GAME_STATIC_WEAPON_METADATA_H
#include <string>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
namespace hd2::game {
//004C6570: lowercase BOTH strings, first substring then atoi after key length.
//No token boundaries, separator skip or case-sensitive material semantics.
inline std::int32_t static_weapon_metadata(std::string text,std::string key) {
    if(text.size()>=256||key.size()>=256)throw std::runtime_error("StaticWeapon metadata exceeds native buffer");
    for(auto* value:{&text,&key})for(auto& c:*value)if(c>='A'&&c<='Z')c=static_cast<char>(c+('a'-'A'));
    const auto at=text.find(key);if(at==std::string::npos)return -1;
    const auto value=std::strtoll(text.c_str()+at+key.size(),nullptr,10);
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("StaticWeapon metadata leaves native atoi bounds");
    return static_cast<std::int32_t>(value);
}
}
#endif
