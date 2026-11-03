#ifndef HD2_HOST_FULLSCREEN_FRAME_H
#define HD2_HOST_FULLSCREEN_FRAME_H

namespace hd2::host {

enum class FullscreenSurface { ready, lost, reset_needed };

// Native frame guard: do not submit text/movie draw calls to a lost exclusive
// device. Engine1006B3E0 owns reset/recovery; no native D3D Reset is substituted.
template<class Platform>
bool fullscreen_frame_ready(Platform& platform) {
    if (!platform.foreground()) {
        platform.wait();
        return false;
    }
    auto state=platform.surface();
    if (state==FullscreenSurface::reset_needed) {
        platform.recover();
        state=platform.surface();
    }
    if (state!=FullscreenSurface::ready) {
        platform.wait();
        return false;
    }
    return true;
}

} // namespace hd2::host
#endif
