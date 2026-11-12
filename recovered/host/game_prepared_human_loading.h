#ifndef HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#define HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#include "host/game_world_loading.h"
#include "game/prepared_human_loading.h"
namespace hd2::host {
using GamePreparedHumanLoading=game::PreparedHumanLoading<world_loading::Dependencies>;
} //namespace hd2::host
#endif
