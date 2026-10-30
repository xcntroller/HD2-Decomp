#ifndef HD2_FRONTEND_APPLICATION_DATA_H
#define HD2_FRONTEND_APPLICATION_DATA_H

#include "frontend/game_data.h"
#include "frontend/human_store.h"
#include "frontend/profile_directories.h"

namespace hd2::frontend {

// Actual 006B65D0 preparation order, before profile scan/selection and movies.
// GameData is the original application +04 owner: the +E8 flag belongs to it.
// Both loaders run even if the first fails. Existing current profile/map state
// supplies HumanStore's optional overlay path; do not scan/select early here.
template<class Files, class Graphics>
bool prepare_application_data(GameData& game, HumanStore<Graphics>& humans,
                              const ProfileDirectories& profiles, Files& files,
                              const LocalizedTextCache& text, bool wide) {
    const auto game_result = game.load(files);
    game.field_e8 = 1;
    const auto human_result = humans.load_with_profile_directory(files, "tables\\HumanStore.def", text,
                                          [&] { return profiles.current_directory(wide); }, wide);
    return static_cast<unsigned>(game_result) & static_cast<unsigned>(human_result) & 1u;
}

}
#endif
