#ifndef HD2_GAME_FOWL_BIRD_RANDOM_H
#define HD2_GAME_FOWL_BIRD_RANDOM_H
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace hd2::game {
inline std::int32_t fowl_signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
//00475190 consumes the SAME CRT stream, returning x87 extended, not a new RNG.
template<class Services>std::int32_t fowl_random_word(Services& services) {
    const auto value=services.fowl_random_word();
    if(value<0||value>32767)throw std::runtime_error("Fowl requires its actual fifteen-bit CRT random stream");
    return value;
}
template<class Services>long double fowl_random(Services& services,float range) {
    if(!(range>0))return 0;
    return static_cast<long double>(fowl_random_word(services))*range*3.0517578125e-05f;
}

} //namespace hd2::game
#endif
