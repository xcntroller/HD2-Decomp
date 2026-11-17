#ifndef HD2_FRONTEND_APPLICATION_FRAME_H
#define HD2_FRONTEND_APPLICATION_FRAME_H

#include <cstdint>

namespace hd2::frontend {

//006DCAE0's full-viewport branch, BEFORE controller render. The later Scene2
//text callback restoring the viewport does not clear old pixels in black bars.
template<class Services>void prepare_application_render(Services& services){
    if(!services.application_full_viewport())return;
    auto viewport=services.application_render_viewport();
    services.query_render_dimensions(); //00647E70, actual display getter side effects.
    viewport.x=viewport.y=0;
    viewport.width=services.graphics_width();viewport.height=services.graphics_height();
    services.query_render_dimensions();
    services.application_render_viewport(viewport); //Preserve the saved near/far planes.
    services.clear_application_render(); //Graph64(0,1.f,3), before any Scene Begin/End.
}

// Normal006DCAE0 frame after keyboard/message pump and recovered frame clock.
// Services retain the actual shared gate and cold control owner. These hooks
// must not be supplied as successful substitutes for unavailable operations.
template<class Controller, class Services>
bool advance_application_frame(Controller& controller, Services& services,
                               std::uint32_t milliseconds) {
    services.sample_frame_input();
    services.tick_driver(milliseconds);
    if (services.application_flags() & 2) {
        prepare_application_render(services);
        controller.render();
    }
    services.control_phase(milliseconds, true);
    const auto fixed_flags = services.application_flags();
    if (((fixed_flags & 1) || fixed_flags == 0x12) && !controller.fixed_update(milliseconds))
        return false;
    services.control_phase(milliseconds, false);
    const auto flags = services.application_flags();
    if (((flags & 2) || ((flags & 7) != 7 && services.application_input_enabled())) &&
            !controller.update(milliseconds)) return false;
    services.control_maintenance(milliseconds);
    if (services.application_flags() & 2) services.present();
    return true;
}

} // namespace hd2::frontend
#endif
