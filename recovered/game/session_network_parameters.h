#ifndef HD2_GAME_SESSION_NETWORK_PARAMETERS_H
#define HD2_GAME_SESSION_NETWORK_PARAMETERS_H
#include <cstdint>
namespace hd2::game {
//SAME process globals009820D8/D4/DC. Original PE .data virtual tail is loader
//zero-filled; later console-variable producers and packet reads mutate them.
//This is data, not successful transport or an input-history implementation.
struct SessionNetworkParameters {
    std::int32_t word_d8=0,word_d4=0,milliseconds_dc=0;
};
}
#endif
