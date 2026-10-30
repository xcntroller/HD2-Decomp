#ifndef HD2_HOST_OPENING_TEXT_H
#define HD2_HOST_OPENING_TEXT_H

#include "host/graphics_interface.h"
#include "host/menu_text.h"
#include "host/opening_text_geometry.h"
#include "frontend/opening_subtitles.h"

namespace hd2::host {

// Actual atlas/primitive consumer for00688630's ordinary style8 text owner.
// Reuses the desktop-confirmed device-state bridge; general paragraphs, original
// object ABI and the full shared font/style cache remain separate recovery.
class OpeningText {
public:
    OpeningText(Graphics* graphics, HMODULE module, frontend::OpeningTextStyles& styles)
        : graphics_(graphics), styles_(styles), atlas_(nullptr, nullptr),
          metrics_(decode_font_metrics(read_menu_text_file("MenuText\\SMALL.tbl"))),
          rectangle_(opening_text_rectangle(graphics->methods->width(graphics), graphics->methods->height(graphics))) {
        if (graphics->methods->width(graphics) != 800 || graphics->methods->height(graphics) != 600)
            throw std::logic_error("Opening subtitle rendering requires English800x600");
        object_.inset = 10;
        const auto base = reinterpret_cast<std::uintptr_t>(module);
        unsigned char anchor[]{0xa1, 0, 0, 0, 0};
        const auto address = static_cast<std::uint32_t>(base + 0xd216c);
        std::memcpy(anchor + 1, &address, 4);
        if (std::memcmp(reinterpret_cast<const void*>(base + 0x6b389), anchor, sizeof(anchor)) != 0)
            throw std::runtime_error("Unsupported opening text device-state bridge");
        std::memcpy(&device_, reinterpret_cast<const void*>(base + 0xd216c), sizeof(device_));
        if (!device_) throw std::runtime_error("Missing opening text device");
        EngineFrame* texture = nullptr;
        const auto status = graphics_->methods->load_texture(graphics_, "HD2_MENU_SMALL.TGA", nullptr, 0x402, &texture);
        // Adopt any returned resource before checking the call status.
        if (texture) atlas_ = MenuDriver::Owner(texture,
            scene_method<MenuDriver::Release>(texture, module, 0, 0x73760));
        check_overlay_status(status, "Load opening subtitle atlas");
        if (!atlas_) throw std::runtime_error("Opening subtitle atlas is null");
    }
    void validate(const frontend::SharedText& text) const {
        frontend::TextObject candidate;
        candidate.inset = object_.inset;
        candidate.set(text, 8, 0);
        vertices(candidate);
    }
    void set(frontend::SharedText text, std::uint8_t style, std::int32_t index) {
        object_.set(std::move(text), style, index);
        rendered_ = false;
    }
    void clear() { object_.clear(); rendered_ = false; }
    // EngineVideo owns this pass's Begin/EndScene, IGraph state0 and presentation.
    void draw() const {
        const auto glyphs = vertices(object_);
        if (glyphs.empty()) return;
        TextOverlayState overlay(device_);
        const auto* previous = graphics_->methods->viewport(graphics_);
        if (!previous) throw std::runtime_error("Missing opening text viewport");
        const auto saved = *previous;
        const CursorViewport full{0, 0, 800, 600, 0, 1};
        try {
            check_overlay_status(graphics_->methods->set_viewport(graphics_, &full), "Set opening text viewport");
            if (!graphics_->methods->bind_texture(graphics_, atlas_.get(), 0))
                throw std::runtime_error("Cannot bind opening subtitle atlas");
            check_overlay_status(graphics_->methods->draw_primitive(graphics_, 3,
                static_cast<std::uint32_t>(glyphs.size() / 3), glyphs.data(), 1), "Submit opening subtitle glyphs");
        } catch (...) {
            graphics_->methods->set_viewport(graphics_, &saved);
            throw;
        }
        check_overlay_status(graphics_->methods->set_viewport(graphics_, &saved), "Restore opening text viewport");
        if (!overlay.restore()) throw std::runtime_error("Cannot restore opening text device state");
        if (!rendered_) {
            std::printf("Opening subtitle rendered: vertices=%zu style=8\n", glyphs.size());
            rendered_ = true;
        }
    }
private:
    std::vector<CursorVertex> vertices(const frontend::TextObject& object) const {
        return opening_subtitle_vertices(object, metrics_, rectangle_, styles_.get(8), 800, 600);
    }
    Graphics* graphics_;
    frontend::OpeningTextStyles& styles_;
    MenuDriver::Owner atlas_;
    FontMetrics metrics_;
    OpeningTextRectangle rectangle_;
    frontend::TextObject object_;
    IDirect3DDevice8* device_ = nullptr;
    mutable bool rendered_ = false;
};

} // namespace hd2::host
#endif
