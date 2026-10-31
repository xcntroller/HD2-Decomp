#ifndef HD2_HOST_MENU_TEXT_H
#define HD2_HOST_MENU_TEXT_H

#include "host/menu_cursor.h"
#include "host/menu_text_geometry.h"
#include "host/menu_text_object.h"
#include "frontend/localized_text.h"

#include <set>

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
    MenuText(Graphics* graphics, HMODULE module, const MenuScene& scene,
             bool confirmation_enabled = false, bool runtime_confirmation_enabled = false,
             const frontend::LocalizedTextCache* localized = nullptr)
        : graphics_(graphics), big_(nullptr, nullptr), small_(nullptr, nullptr),
          big_metrics_(decode_font_metrics(read_menu_text_file("MenuText\\BIG.tbl"))),
          small_metrics_(decode_font_metrics(read_menu_text_file("MenuText\\SMALL.tbl"))),
          labels_(localized ? std::map<std::uint32_t, std::string>{} :
                  decode_menu_labels(read_menu_text_file("MenuText\\labels.bin"))) {
        std::set<std::uint32_t> required;
        const auto require_captions = [&](const frontend::MenuScreenDefinition& definition) {
            for (const auto& control : definition.controls) for (const auto& event : control.initialization) {
                if (event.code == 0x01000015 && event.argument != 40) required.insert(event.word_0c);
            }
        };
        require_captions(frontend::main_menu_definition());
        if (confirmation_enabled) require_captions(frontend::confirmation_definition());
        if (runtime_confirmation_enabled) required.insert(2013);
        if (localized) for (const auto id : required)
            labels_.emplace(id, frontend::lookup_localized_text(*localized, static_cast<std::int32_t>(id)));
        if (labels_.size() != required.size()) throw std::runtime_error("Incorrect staged control caption count");
        for (const auto id : required) if (labels_.find(id) == labels_.end()) {
            throw std::runtime_error("Missing original control caption");
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
        create_objects(scene, module, frontend::main_menu_definition(), false);
        if (confirmation_enabled) create_objects(scene, module, frontend::confirmation_definition(), true);
        std::printf("Original attached text objects ready: %u; captions come from definition events\n",
                    static_cast<unsigned>(objects_.size()));
    }

    MenuTextObject* object_for(std::uint32_t identifier) const { return objects_.at(identifier).get(); }

    // The runtime dialog definition is produced after the queue exists. Its
    // owner must outlive these attached text objects.
    void attach_runtime_definition(const MenuScene& scene, HMODULE module,
                                   const frontend::MenuScreenDefinition& definition) {
        create_objects(scene, module, definition, true);
    }

    bool over_exit(const std::optional<CursorPoint>& point) const {
        if (!point) return false;
        for (const auto& entry : frontend::main_menu_definition().controls) {
            if (entry.role != frontend::MenuControlRole::exit) continue;
            auto* text = object_for(entry.identifier);
            return text->visible() && text->available() && !text->object().disabled &&
                   text->rectangle(false).contains(*point);
        }
        return false;
    }

    int confirmation_hit(const std::optional<CursorPoint>& point) const {
        return confirmation_hit(frontend::confirmation_definition(), point);
    }
    int confirmation_hit(const frontend::MenuScreenDefinition& definition,
                         const std::optional<CursorPoint>& point) const {
        if (!point) return 0;
        for (std::size_t i = 0; i < definition.controls.size(); ++i) {
            const auto& entry = definition.controls[i];
            if (entry.role != frontend::MenuControlRole::yes && entry.role != frontend::MenuControlRole::no &&
                entry.role != frontend::MenuControlRole::ok) continue;
            auto* text = object_for(entry.identifier);
            if (text->visible() && text->available() && !text->object().disabled &&
                text->rectangle(false).contains(*point)) return static_cast<int>(i);
        }
        return 0;
    }

    void draw(bool exit_hover, bool confirmation = false, int confirm_hover = 0, int main_hover = -1) const {
        draw_definition(confirmation ? frontend::confirmation_definition() : frontend::main_menu_definition(),
                        exit_hover, confirmation ? confirm_hover : main_hover);
    }

    void draw_runtime_confirmation(const frontend::MenuScreenDefinition& definition, int hover) const {
        draw_definition(definition, false, hover);
    }

private:
    void draw_definition(const frontend::MenuScreenDefinition& definition,
                         bool exit_hover, int confirm_hover) const {
        std::vector<MenuTextSubmission> submissions;
        for (std::size_t i = 0; i < definition.controls.size(); ++i) {
            const auto& entry = definition.controls[i];
            auto* attached = object_for(entry.identifier);
            const auto& object = attached->object();
            if (!attached->visible()) continue;
            const bool button = entry.kind == frontend::MenuControlKind::button;
            const bool hovered = confirm_hover >= 0 ? static_cast<int>(i) == confirm_hover && button
                                                    : entry.role == frontend::MenuControlRole::exit && exit_hover;
            auto submission = menu_text_submission(object, [&] { return attached->bounds(); },
                button, attached->available(), hovered, big_metrics_, small_metrics_);
            if (submission) submissions.push_back(std::move(*submission));
        }
        TextOverlayState state(device_);
        const CursorViewport viewport{0, 0, 800, 600, 0, 1};
        const auto* old = graphics_->methods->viewport(graphics_);
        if (old == nullptr) throw std::runtime_error("Engine viewport getter returned null");
        const auto old_viewport = *old;
        graphics_->methods->begin_scene(graphics_);
        auto status = graphics_->methods->set_viewport(graphics_, &viewport);
        for (const auto& submission : submissions) {
            if (status < 0) break;
            if (graphics_->methods->bind_texture(graphics_, submission.small ? small_.get() : big_.get(), 0) == 0) {
                status = E_FAIL;
            } else if (!submission.vertices.empty()) {
                status = graphics_->methods->draw_primitive(graphics_, 3,
                    static_cast<std::uint32_t>(submission.vertices.size() / 3), submission.vertices.data(), 1);
            }
        }
        const auto viewport_status = graphics_->methods->set_viewport(graphics_, &old_viewport);
        graphics_->methods->end_scene(graphics_);
        if (!state.restore()) throw std::runtime_error("Cannot restore bitmap text render state");
        check_overlay_status(status, "Submit original menu glyphs");
        check_overlay_status(viewport_status, "Restore original menu viewport");
    }

private:
    void create_objects(const MenuScene& scene, HMODULE module,
                        const frontend::MenuScreenDefinition& definition, bool confirmation) {
        for (const auto& entry : definition.controls) {
            auto object = std::make_unique<MenuTextObject>(scene, module, entry, confirmation,
                [&](std::uint32_t id) { return std::make_shared<const std::string>(labels_.at(id)); });
            if (!objects_.emplace(entry.identifier, std::move(object)).second) {
                throw std::logic_error("Duplicate attached text identifier");
            }
        }
    }

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
    std::map<std::uint32_t, std::unique_ptr<MenuTextObject>> objects_;
    IDirect3DDevice8* device_ = nullptr;
};

}

#endif
