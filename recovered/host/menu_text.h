#ifndef HD2_HOST_MENU_TEXT_H
#define HD2_HOST_MENU_TEXT_H

#include "host/menu_cursor.h"
#include "host/menu_text_geometry.h"

#include <d3d8.h>
#include <fstream>
#include <vector>

namespace hd2::host {

inline void check_overlay_status(HRESULT status, const char* operation) {
    if (FAILED(status)) {
        char message[160];
        std::snprintf(message, sizeof(message), "%s failed: 0x%08x",
                      operation, static_cast<unsigned>(status));
        throw std::runtime_error(message);
    }
}

class TextOverlayState {
public:
    explicit TextOverlayState(IDirect3DDevice8* device) : device_(device) {
        for (auto& state : render_) {
            check_overlay_status(device_->GetRenderState(state.kind, &state.saved),
                                 "Get text render state");
        }
        for (auto& state : stages_) {
            check_overlay_status(device_->GetTextureStageState(state.stage, state.kind,
                                                               &state.saved),
                                 "Get text texture-stage state");
        }
        check_overlay_status(device_->GetPixelShader(&pixel_shader_), "Get text pixel shader");
        check_overlay_status(device_->GetTexture(0, &texture_), "Get text stage-zero texture");
        saved_ = true;
        HRESULT status = device_->SetPixelShader(0);
        for (const auto& state : render_) {
            if (SUCCEEDED(status)) status = device_->SetRenderState(state.kind, state.value);
        }
        for (const auto& state : stages_) {
            if (SUCCEEDED(status)) {
                status = device_->SetTextureStageState(state.stage, state.kind, state.value);
            }
        }
        if (FAILED(status)) {
            if (!restore()) std::fputs("Failed to roll back bitmap text state\n", stderr);
            check_overlay_status(status, "Configure bitmap text overlay");
        }
    }

    ~TextOverlayState() {
        if (saved_ && !restore()) {
            std::fputs("Text overlay state restoration failed during cleanup\n", stderr);
        }
    }
    TextOverlayState(const TextOverlayState&) = delete;
    TextOverlayState& operator=(const TextOverlayState&) = delete;

    bool restore() {
        bool success = true;
        for (const auto& state : render_) {
            if (FAILED(device_->SetRenderState(state.kind, state.saved))) success = false;
        }
        for (const auto& state : stages_) {
            if (FAILED(device_->SetTextureStageState(state.stage, state.kind, state.saved))) {
                success = false;
            }
        }
        if (FAILED(device_->SetTexture(0, texture_))) success = false;
        if (FAILED(device_->SetPixelShader(pixel_shader_))) success = false;
        if (texture_ != nullptr) texture_->Release();
        texture_ = nullptr;
        saved_ = false;
        return success;
    }

private:
    struct RenderState { D3DRENDERSTATETYPE kind; DWORD value; DWORD saved = 0; };
    struct StageState { DWORD stage; D3DTEXTURESTAGESTATETYPE kind; DWORD value; DWORD saved = 0; };
    std::array<RenderState, 16> render_{{
        {D3DRS_ZENABLE, FALSE}, {D3DRS_ZWRITEENABLE, FALSE},
        {D3DRS_ALPHABLENDENABLE, TRUE}, {D3DRS_SRCBLEND, D3DBLEND_SRCALPHA},
        {D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA}, {D3DRS_ALPHATESTENABLE, FALSE},
        {D3DRS_CULLMODE, D3DCULL_NONE}, {D3DRS_LIGHTING, FALSE},
        {D3DRS_FOGENABLE, FALSE}, {D3DRS_SPECULARENABLE, FALSE},
        {D3DRS_BLENDOP, D3DBLENDOP_ADD}, {D3DRS_COLORWRITEENABLE, 15},
        {D3DRS_STENCILENABLE, FALSE}, {D3DRS_CLIPPLANEENABLE, 0},
        {D3DRS_FILLMODE, D3DFILL_SOLID}, {D3DRS_SHADEMODE, D3DSHADE_GOURAUD},
    }};
    std::array<StageState, 15> stages_{{
        {0, D3DTSS_COLOROP, D3DTOP_MODULATE}, {0, D3DTSS_COLORARG1, D3DTA_TEXTURE},
        {0, D3DTSS_COLORARG2, D3DTA_DIFFUSE}, {0, D3DTSS_ALPHAOP, D3DTOP_MODULATE},
        {0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE}, {0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE},
        {0, D3DTSS_TEXCOORDINDEX, 0}, {0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE},
        {0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP}, {0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP},
        {0, D3DTSS_MINFILTER, D3DTEXF_LINEAR}, {0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR},
        {0, D3DTSS_MIPFILTER, D3DTEXF_NONE}, {0, D3DTSS_RESULTARG, D3DTA_CURRENT},
        {1, D3DTSS_COLOROP, D3DTOP_DISABLE},
    }};
    IDirect3DDevice8* device_;
    IDirect3DBaseTexture8* texture_ = nullptr;
    DWORD pixel_shader_ = 0;
    bool saved_ = false;
};

inline std::vector<std::uint8_t> read_menu_text_file(const char* name) {
    std::ifstream input(name, std::ios::binary | std::ios::ate);
    if (!input) throw std::runtime_error(std::string("Cannot open menu text resource: ") + name);
    const auto size = input.tellg();
    if (size < 0 || size > 65536) {
        throw std::runtime_error(std::string("Invalid menu text resource size: ") + name);
    }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), size)) {
        throw std::runtime_error(std::string("Cannot read menu text resource: ") + name);
    }
    return bytes;
}

template<class Graphics>
class MenuText {
public:
    MenuText(Graphics* graphics, HMODULE module, const MenuScene& scene)
        : graphics_(graphics), big_(nullptr, nullptr), small_(nullptr, nullptr),
          big_metrics_(decode_font_metrics(read_menu_text_file("MenuText\\BIG.tbl"))),
          small_metrics_(decode_font_metrics(read_menu_text_file("MenuText\\SMALL.tbl"))),
          labels_(decode_menu_labels(read_menu_text_file("MenuText\\labels.bin"))) {
        if (labels_.size() != menu_label_bindings.size()) {
            throw std::runtime_error("Static menu requires exactly 13 staged labels");
        }
        if (graphics_->methods->width(graphics_) != 800 ||
                graphics_->methods->height(graphics_) != 600) {
            throw std::runtime_error("The static Set2 menu-text slice requires 800x600");
        }
        const auto base = reinterpret_cast<std::uintptr_t>(module);
        // Borrow the device loaded by the verified IGraph::BeginScene instruction.
        unsigned char device_anchor[] = {0xa1, 0, 0, 0, 0};
        const auto device_va = static_cast<std::uint32_t>(base + 0xd216c);
        std::memcpy(device_anchor + 1, &device_va, sizeof(device_va));
        if (std::memcmp(reinterpret_cast<const void*>(base + 0x6b389),
                        device_anchor, sizeof(device_anchor)) != 0) {
            throw std::runtime_error("Unsupported engine Direct3D-device bridge");
        }
        std::memcpy(&device_, reinterpret_cast<const void*>(base + 0xd216c), sizeof(device_));
        if (device_ == nullptr) throw std::runtime_error("Engine Direct3D device is null");
        big_ = load_atlas("HD2_MENU_BIG.TGA", module);
        small_ = load_atlas("HD2_MENU_SMALL.TGA", module);
        for (std::size_t i = 0; i < menu_label_bindings.size(); ++i) {
            const auto& binding = menu_label_bindings[i];
            if (labels_.find(binding.id) == labels_.end()) {
                throw std::runtime_error("Missing staged original menu label");
            }
            rectangles_[i] = menu_rectangle(
                scene.label_bounds(binding.frame, binding.visual, module), 800, 600);
            std::printf("Menu label id=%u frame=%s bounds=(%.1f,%.1f)-(%.1f,%.1f)\n",
                        static_cast<unsigned>(binding.id), binding.frame,
                        rectangles_[i].left, rectangles_[i].top,
                        rectangles_[i].right, rectangles_[i].bottom);
        }
        std::puts("Original bitmap text ready: 13 labels; only Exit is enabled in this host");
    }

    bool over_exit(const std::optional<CursorPoint>& point) const {
        return point && rectangles_[10].contains(*point);
    }

    void draw(bool exit_hover) const {
        std::array<std::vector<CursorVertex>, 13> glyphs;
        for (std::size_t i = 0; i < menu_label_bindings.size(); ++i) {
            const auto& binding = menu_label_bindings[i];
            std::uint32_t color = binding.button && binding.id != 2112 ? 0x60000000 : 0xff000000;
            if (binding.id == 2112 && exit_hover) color = 0xffffffff;
            glyphs[i] = menu_label_vertices(labels_.at(binding.id),
                binding.small ? small_metrics_ : big_metrics_, rectangles_[i],
                binding.height, 600, binding.id == 2101, color);
        }
        TextOverlayState state(device_);
        const CursorViewport viewport{0, 0, 800, 600, 0, 1};
        const auto* old = graphics_->methods->viewport(graphics_);
        if (old == nullptr) throw std::runtime_error("Engine viewport getter returned null");
        const auto old_viewport = *old;
        graphics_->methods->begin_scene(graphics_);
        auto status = graphics_->methods->set_viewport(graphics_, &viewport);
        for (std::size_t i = 0; status >= 0 && i < menu_label_bindings.size(); ++i) {
            const auto& binding = menu_label_bindings[i];
            if (graphics_->methods->bind_texture(
                    graphics_, binding.small ? small_.get() : big_.get(), 0) == 0) {
                status = E_FAIL;
            } else if (!glyphs[i].empty()) {
                status = graphics_->methods->draw_primitive(graphics_, 3,
                    static_cast<std::uint32_t>(glyphs[i].size() / 3), glyphs[i].data(), 1);
            }
        }
        const auto viewport_status = graphics_->methods->set_viewport(graphics_, &old_viewport);
        graphics_->methods->end_scene(graphics_);
        if (!state.restore()) throw std::runtime_error("Cannot restore bitmap text render state");
        check_overlay_status(status, "Submit original menu glyphs");
        check_overlay_status(viewport_status, "Restore original menu viewport");
    }

private:
    MenuDriver::Owner load_atlas(const char* name, HMODULE module) const {
        EngineFrame* texture = nullptr;
        const auto status = graphics_->methods->load_texture(graphics_, name, nullptr, 0x402,
                                                             &texture);
        check_overlay_status(status, "Load original menu font atlas");
        if (texture == nullptr) throw std::runtime_error("Original menu font texture is null");
        return MenuDriver::Owner(texture,
            scene_method<MenuDriver::Release>(texture, module, 0, 0x73760));
    }

    Graphics* graphics_;
    MenuDriver::Owner big_, small_;
    FontMetrics big_metrics_, small_metrics_;
    std::map<std::uint32_t, std::string> labels_;
    std::array<MenuRectangle, 13> rectangles_{};
    IDirect3DDevice8* device_ = nullptr;
};

}

#endif
