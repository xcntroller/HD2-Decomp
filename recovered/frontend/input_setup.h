#ifndef HD2_FRONTEND_INPUT_SETUP_H
#define HD2_FRONTEND_INPUT_SETUP_H

#include "frontend/localized_text.h"

namespace hd2::frontend {

struct StartupJoystickView {
    void* const* devices;
    std::int32_t count;
};

// 006db620: original keyboard initialization, manager setup and localization
// preload. Setup supplies GetIGraph/+90 and the actual global joystick view.
// Resolve graphics twice and snapshot joystick view BEFORE the second query.
// Both keyboard-init and manager-attachment results are deliberately ignored.
template<class Manager, class Setup, class Language, class Platform, class Files, class Directory>
void setup_graphics_input(Manager& manager, Setup& setup, Language& language,
                           LocalizedTextCache& cache, Platform& platform,
                           Files& files, Directory& directory) {
    setup.initialize_keyboard(setup.graphics(), 0);
    manager.clear_bindings();
    const auto joysticks = setup.joysticks();
    manager.attach(setup.graphics(), joysticks.devices, joysticks.count);
    const auto selected_language = language.get();
    preload_language_text(cache, platform, files, directory, selected_language);
}

}
#endif
