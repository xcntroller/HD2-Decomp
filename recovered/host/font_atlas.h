#ifndef HD2_HOST_FONT_ATLAS_H
#define HD2_HOST_FONT_ATLAS_H

#include "host/menu_scene.h"

namespace hd2::host {

// Shared real engine acquisition used by the accepted menu and game font owner.
// Adopt any returned reference before checking failure, so failed partial loads
// cannot strand a texture. Engine10073760 owns the actual texture Release.
template<class Graphics>
MenuDriver::Owner load_font_atlas(Graphics* graphics, HMODULE module, const char* name) {
    EngineFrame* texture = nullptr;
    const auto status = graphics->methods->load_texture(graphics, name, nullptr, 0x402, &texture);
    MenuDriver::Owner result{nullptr, nullptr};
    if (texture) {
        const auto release = scene_method<MenuDriver::Release>(texture, module, 0, 0x73760);
        result = MenuDriver::Owner(texture, release);
    }
    if (status < 0 || !result)
        throw std::runtime_error(std::string("Cannot load original font atlas: ") + name);
    return result;
}

} // namespace hd2::host
#endif
