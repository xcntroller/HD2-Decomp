#ifndef HD2_NETWORKING_HEADLESS_STARTUP_DATA_H
#define HD2_NETWORKING_HEADLESS_STARTUP_DATA_H

#include "frontend/game_data.h"
#include "networking/headless_human_store.h"
#include <utility>

namespace hd2::networking {

//006F9550's actual preparation order, after the containing application has
// created its owners. File/text/profile-directory inputs must be genuine; this
// does not construct them or acknowledge controller/world/session readiness.
template<class Files, class ProfileDirectory>
bool prepare_headless_startup_data(frontend::GameData& game,HeadlessHumanStore& humans,
        Files& files,const frontend::LocalizedTextCache& text,
        ProfileDirectory&& directory,bool wide) {
    const auto game_loaded = game.load(files);
    game.field_e8 = 1;
    const auto humans_loaded = humans.load(files,text,std::forward<ProfileDirectory>(directory),wide);
    return static_cast<unsigned>(game_loaded) & static_cast<unsigned>(humans_loaded) & 1u;
}

} // namespace hd2::networking
#endif
