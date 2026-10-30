#ifndef HD2_FRONTEND_OPENING_RENDER_H
#define HD2_FRONTEND_OPENING_RENDER_H

namespace hd2::frontend {

//00688F50: distinct movie/text scenes under one saved IGraph state0. The
// controller presents afterward; preview callers may request presentation here.
template<class Renderer, class TextPass>
void render_opening_frame(Renderer& renderer, bool has_text, TextPass&& text_pass, bool present) {
    const auto saved = renderer.graphics_state();
    renderer.clear();
    if (renderer.has_movie()) {
        renderer.begin();
        renderer.draw_movie();
        renderer.end();
    }
    if (has_text) {
        renderer.graphics_state(1);
        renderer.begin();
        try { text_pass(); }
        catch (...) {
            renderer.end();
            renderer.graphics_state(saved);
            throw;
        }
        renderer.end();
    }
    renderer.graphics_state(saved);
    if (present) renderer.present();
}

} // namespace hd2::frontend
#endif
