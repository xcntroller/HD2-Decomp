#ifndef HD2_FRONTEND_MENU_CLICK_SOUND_H
#define HD2_FRONTEND_MENU_CLICK_SOUND_H

namespace hd2::frontend {

// Client 0066b110. Front-end +3c may be null. Query sound +78 with zero,
// and only when false, re-read +3c and invoke sound +30 with one. The
// second lookup matters if the query changes the front-end's sound binding.
template<class Frontend>
void accepted_menu_click(Frontend& frontend) {
    const auto sound = frontend.click_sound();
    if (sound != nullptr && !frontend.sound_active(sound, false)) {
        frontend.enable_sound(frontend.click_sound(), true);
    }
}

}

#endif
