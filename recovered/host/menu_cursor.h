#ifndef HD2_HOST_MENU_CURSOR_H
#define HD2_HOST_MENU_CURSOR_H

#include "host/cursor_geometry.h"
#include "host/menu_scene.h"

namespace hd2::host {

struct CursorViewport {
    std::uint32_t x, y, width, height;
    float min_z, max_z;
};

static_assert(sizeof(CursorViewport) == 24);

template<class Graphics>
class MenuCursor {
public:
    MenuCursor(Graphics* graphics, HMODULE module)
        : graphics_(graphics), texture_(nullptr, nullptr) {
        EngineFrame* texture = nullptr;
        const auto status = graphics_->methods->load_texture(
            graphics_, "x_kursor.tga", nullptr, 0x402, &texture);
        if (status < 0 || texture == nullptr) {
            char message[100];
            std::snprintf(message, sizeof(message), "Original cursor texture load failed: 0x%08x",
                          static_cast<unsigned>(status));
            throw std::runtime_error(message);
        }
        const auto release = scene_method<MenuDriver::Release>(texture, module, 0, 0x73760);
        texture_ = MenuDriver::Owner(texture, release);
        std::puts("Original cursor texture loaded: Maps\\x_kursor.tga");
    }

    std::optional<CursorPoint> position(HWND main_window, const POINT* supplied = nullptr) const {
        HWND window = graphics_->methods->child_window(graphics_);
        if (window == nullptr || !IsWindow(window)) {
            throw std::runtime_error("Cursor requires a valid engine child window");
        }
        if (GetAncestor(GetForegroundWindow(), GA_ROOT) != main_window || IsIconic(main_window)) {
            return std::nullopt;
        }
        POINT point{};
        RECT rectangle{};
        if (supplied != nullptr) point = *supplied;
        if ((supplied == nullptr && !GetCursorPos(&point)) || !ScreenToClient(window, &point) ||
                !GetClientRect(window, &rectangle)) {
            throw std::runtime_error("Cannot obtain cursor position in the engine child window");
        }
        const auto client_width = rectangle.right - rectangle.left;
        const auto client_height = rectangle.bottom - rectangle.top;
        if (client_width == 0 || client_height == 0) {
            return std::nullopt;
        }
        const auto width = graphics_->methods->width(graphics_);
        const auto height = graphics_->methods->height(graphics_);
        return cursor_position(point.x, point.y, client_width, client_height, width, height);
    }

    bool draw(HWND main_window) const {
        const auto point = position(main_window);
        if (!point) {
            return false;
        }
        const auto width = graphics_->methods->width(graphics_);
        const auto height = graphics_->methods->height(graphics_);
        const auto vertices = cursor_quad(*point, width, height);
        const CursorViewport viewport{0, 0, static_cast<std::uint32_t>(width),
                                       static_cast<std::uint32_t>(height), 0, 1};
        graphics_->methods->begin_scene(graphics_);
        const auto viewport_status = graphics_->methods->set_viewport(graphics_, &viewport);
        const auto bound = graphics_->methods->bind_texture(graphics_, texture_.get(), 0);
        const auto draw_status = viewport_status >= 0 && bound != 0 ?
            graphics_->methods->draw_primitive(graphics_, 4, 2, vertices.data(), 1) : -1;
        graphics_->methods->end_scene(graphics_);
        if (viewport_status < 0 || bound == 0 || draw_status < 0) {
            char message[160];
            std::snprintf(message, sizeof(message),
                "Cursor rendering failed: viewport=0x%08x bind=%u draw=0x%08x",
                static_cast<unsigned>(viewport_status), static_cast<unsigned>(bound),
                static_cast<unsigned>(draw_status));
            throw std::runtime_error(message);
        }
        return true;
    }

private:
    Graphics* graphics_;
    MenuDriver::Owner texture_;
};

}

#endif
