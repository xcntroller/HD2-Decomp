#ifndef HD2_HOST_MENU_SCENE_H
#define HD2_HOST_MENU_SCENE_H

#include <windows.h>

#include "host/frame_name.h"
#include "frontend/pointer_pick.h"
#include "frontend/cursor_frame.h"
#include "frontend/menu_click_sound.h"

#include <cstddef>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::host {

struct EngineFrame {
    const std::uintptr_t* methods;
};

template<class Function>
Function scene_method(EngineFrame* frame, HMODULE module, std::size_t offset,
                      std::uintptr_t rva) {
    if (frame == nullptr || frame->methods[offset / 4] !=
            reinterpret_cast<std::uintptr_t>(module) + rva) {
        char message[100];
        std::snprintf(message, sizeof(message),
                      "Unsupported menu ABI: slot 0x%x, expected RVA 0x%x, actual RVA 0x%x",
                      static_cast<unsigned>(offset), static_cast<unsigned>(rva),
                      frame == nullptr ? 0u : static_cast<unsigned>(frame->methods[offset / 4] -
                          reinterpret_cast<std::uintptr_t>(module)));
        throw std::runtime_error(message);
    }
    return reinterpret_cast<Function>(frame->methods[offset / 4]);
}

class MenuDriver {
public:
    using Factory = EngineFrame* (WINAPI *)(EngineFrame*, std::uint32_t);

    explicit MenuDriver(HMODULE module) : module_(module) {
        const auto entry = GetProcAddress(module, "I3DGetDriver");
        if (reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x68fb0) {
            throw std::runtime_error("Missing or unsupported I3DGetDriver export");
        }
        using GetDriver = EngineFrame* (__cdecl *)();
        GetDriver get_driver = nullptr;
        static_assert(sizeof(entry) == sizeof(get_driver));
        std::memcpy(&get_driver, &entry, sizeof(entry));
        driver_ = get_driver();
        using Initialize = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        const auto initialize = scene_method<Initialize>(driver_, module, 0, 0x152f0);
        close_ = scene_method<Close>(driver_, module, 4, 0x15650);
        factory_ = scene_method<Factory>(driver_, module, 0x54, 0x19e10);
        const auto status = initialize(driver_, 0);
        if (status < 0) {
            close_(driver_);
            throw std::runtime_error("Original 3D driver initialization failed");
        }
        std::puts("3D driver initialized");
    }

    ~MenuDriver() {
        close_(driver_);
        std::puts("3D driver closed");
    }

    MenuDriver(const MenuDriver&) = delete;
    MenuDriver& operator=(const MenuDriver&) = delete;

    using Release = std::int32_t (WINAPI *)(EngineFrame*);
    using Owner = std::unique_ptr<EngineFrame, Release>;

    Owner create(std::uint32_t kind) const {
        EngineFrame* frame = factory_(driver_, kind);
        if (frame == nullptr) {
            throw std::runtime_error("Original engine frame factory returned null");
        }
        const auto release = scene_method<Release>(frame, module_, 0, 0x1c370);
        return Owner(frame, release);
    }

    std::uint32_t render_state(std::uint32_t state) const {
        using GetState = std::uint32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        return scene_method<GetState>(driver_, module_, 0x1c, 0x15c20)(driver_, state);
    }

    std::int32_t set_render_state(std::uint32_t state, std::uint32_t value) const {
        using SetState = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
        return scene_method<SetState>(driver_, module_, 0x18, 0x15770)(driver_, state, value);
    }

    EngineFrame* frame() const { return driver_; }

private:
    using Close = std::int32_t (WINAPI *)(EngineFrame*);
    HMODULE module_;
    EngineFrame* driver_ = nullptr;
    Close close_ = nullptr;
    Factory factory_ = nullptr;
};

class MenuScene {
public:
    explicit MenuScene(HMODULE module, bool confirmation_preview = false,
                       bool confirmation_enabled = false, bool bind_cursor_model = false,
                       bool runtime_confirmation_enabled = false)
        : MenuScene(std::make_unique<MenuDriver>(module), nullptr, module,
                    confirmation_preview, confirmation_enabled, bind_cursor_model,
                    runtime_confirmation_enabled) {}

    MenuScene(MenuDriver& driver, HMODULE module, bool confirmation_enabled,
              bool bind_cursor_model)
        : MenuScene(nullptr, &driver, module, false, confirmation_enabled,
                    bind_cursor_model, false) {}

private:
    MenuScene(std::unique_ptr<MenuDriver> owned, MenuDriver* borrowed, HMODULE module,
              bool confirmation_preview, bool confirmation_enabled,
              bool bind_cursor_model, bool runtime_confirmation_enabled)
        : owned_driver_(std::move(owned)), driver_(borrowed ? *borrowed : *owned_driver_),
          scene_(driver_.create(13)), camera_(driver_.create(3)),
          light_(driver_.create(2)), model_(driver_.create(9)) {
        using Pair = void (WINAPI *)(EngineFrame*, EngineFrame*);
        using Vector = void (WINAPI *)(EngineFrame*, const float*);
        using Direction = void (WINAPI *)(EngineFrame*, const float*, const float*);
        using Float = void (WINAPI *)(EngineFrame*, float);
        using Integer = void (WINAPI *)(EngineFrame*, std::uint32_t);
        using Range = void (WINAPI *)(EngineFrame*, float, float);
        using Projection = void (WINAPI *)(EngineFrame*, std::uint8_t);
        using Open = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t,
                                           void*, void*);
        using Link = std::int32_t (WINAPI *)(EngineFrame*, EngineFrame*, std::uint32_t);
        const auto add = scene_method<Pair>(scene_.get(), module, 0x80, 0x469a0);
        const float position[] = {0.5f, -0.375f, -15.5f};
        const float direction[] = {0, 0, 1};
        scene_method<Direction>(camera_.get(), module, 0x1c, 0x1c400)(
            camera_.get(), direction, nullptr);
        scene_method<Vector>(camera_.get(), module, 8, 0x1c9f0)(camera_.get(), position);
        scene_method<Range>(camera_.get(), module, 0x7c, 0x09ef0)(camera_.get(), 0.2f, 20);
        scene_method<Float>(camera_.get(), module, 0x74, 0x09e30)(camera_.get(), 1.57079637f);
        scene_method<Projection>(camera_.get(), module, 0x88, 0x09ff0)(camera_.get(), 1);
        scene_method<Float>(camera_.get(), module, 0x84, 0x09fb0)(camera_.get(), 2);
        using Update = void (WINAPI *)(EngineFrame*);
        scene_method<Update>(camera_.get(), module, 0x24, 0x1e610)(camera_.get());
        scene_method<Pair>(scene_.get(), module, 0x90, 0x471a0)(scene_.get(), camera_.get());
        add(scene_.get(), camera_.get());

        // The original client also reads this field; no public accessor is known yet.
        EngineFrame* sector = nullptr;
        std::memcpy(&sector, reinterpret_cast<const char*>(scene_.get()) + 0x1f8,
                    sizeof(sector));
        if (sector == nullptr) {
            throw std::runtime_error("Menu scene has no primary sector");
        }
        scene_method<Integer>(light_.get(), module, 0x74, 0x217e0)(light_.get(), 4);
        const float white[] = {1, 1, 1};
        scene_method<Vector>(light_.get(), module, 0x84, 0x21db0)(light_.get(), white);
        scene_method<Float>(light_.get(), module, 0x90, 0x22120)(light_.get(), 0.9f);
        add(scene_.get(), light_.get());
        scene_method<Pair>(sector, module, 0x74, 0x4ceb0)(sector, light_.get());

        const char* model_path = confirmation_preview ? "Models\\confirm" : "Models\\main_menu";
        std::printf("Opening original model: %s.4ds\n", model_path);
        std::fflush(stdout);
        const auto status = scene_method<Open>(model_.get(), module, 0x74, 0x340b0)(
            model_.get(), model_path, 0, nullptr, nullptr);
        if (status < 0) {
            char message[100];
            std::snprintf(message, sizeof(message), "Menu model open failed: 0x%08x",
                          static_cast<unsigned>(status));
            throw std::runtime_error(message);
        }
        const auto linked = scene_method<Link>(model_.get(), module, 0x38, 0x1d680)(
            model_.get(), sector, 0);
        if (linked < 0) {
            throw std::runtime_error("Menu model could not be linked to the primary sector");
        }
        if (confirmation_preview) {
            const float confirmation_position[] = {0.5f, -0.375f, -0.5f};
            scene_method<Vector>(model_.get(), module, 8, 0x1c9f0)(
                model_.get(), confirmation_position);
            std::puts("Confirmation model position: 0.5, -0.375, -0.5");
        }
        scene_method<Update>(model_.get(), module, 0x24, 0x1e610)(model_.get());
        std::uint32_t nodes = 0;
        log_hierarchy(model_.get(), module, 0, nodes);
        if (nodes <= 1) {
            throw std::runtime_error("Menu model opened without any attached child frames");
        }
        std::printf("Menu model hierarchy: %u frames including model root\n",
                    static_cast<unsigned>(nodes));
        if (confirmation_enabled) {
            confirmation_ = driver_.create(9);
            if (scene_method<Open>(confirmation_.get(), module, 0x74, 0x340b0)(
                    confirmation_.get(), "Models\\confirm", 0, nullptr, nullptr) < 0 ||
                scene_method<Link>(confirmation_.get(), module, 0x38, 0x1d680)(
                    confirmation_.get(), sector, 0) < 0) {
                throw std::runtime_error("Cannot open or link original confirmation model");
            }
            const float position[] = {0.5f, -0.375f, -0.5f};
            scene_method<Vector>(confirmation_.get(), module, 8, 0x1c9f0)(
                confirmation_.get(), position);
            scene_method<Update>(confirmation_.get(), module, 0x24, 0x1e610)(
                confirmation_.get());
            set_confirmation_visible(module, false);
        }
        if (runtime_confirmation_enabled) {
            runtime_confirmation_ = driver_.create(9);
            if (scene_method<Open>(runtime_confirmation_.get(), module, 0x74, 0x340b0)(
                    runtime_confirmation_.get(), "Models\\confirm", 0, nullptr, nullptr) < 0 ||
                scene_method<Link>(runtime_confirmation_.get(), module, 0x38, 0x1d680)(
                    runtime_confirmation_.get(), sector, 0) < 0) {
                throw std::runtime_error("Cannot open or link runtime confirmation model");
            }
            const float position[] = {0.5f, -0.375f, -0.5f};
            scene_method<Vector>(runtime_confirmation_.get(), module, 8, 0x1c9f0)(
                runtime_confirmation_.get(), position);
            scene_method<Update>(runtime_confirmation_.get(), module, 0x24, 0x1e610)(
                runtime_confirmation_.get());
            set_screen_visible(module, runtime_confirmation_.get(), false);
        }
        if (bind_cursor_model) {
            cursor_model_ = driver_.create(9);
            const auto open = scene_method<Open>(cursor_model_.get(), module, 0x74, 0x340b0);
            if (open(cursor_model_.get(), "Models\\kurzor_menu", 0, nullptr, nullptr) < 0) {
                throw std::runtime_error("Cannot open original kurzor_menu model");
            }
            // Models override the visual/base name setter with a wrapper that
            // also propagates model name state. Preserve that virtual method.
            using Name = std::int32_t (WINAPI *)(EngineFrame*, const char*);
            if (scene_method<Name>(cursor_model_.get(), module, 0x34, 0x346a0)(
                    cursor_model_.get(), "CURZOR") < 0) {
                throw std::runtime_error("Cannot name original CURZOR model");
            }
            if (scene_method<Link>(cursor_model_.get(), module, 0x38, 0x1d680)(
                    cursor_model_.get(), sector, 0) < 0) {
                throw std::runtime_error("Cannot link original CURZOR to primary sector");
            }
            using Visible = void (WINAPI *)(EngineFrame*, std::uint8_t);
            scene_method<Visible>(cursor_model_.get(), module, 0x30, 0x1cfe0)(
                cursor_model_.get(), 0);
            using Find = EngineFrame* (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
            const auto find = scene_method<Find>(scene_.get(), module, 0x7c, 0x46920);
            cursor_frame_ = find(scene_.get(), "CURZOR", 0xffff);
            if (cursor_frame_ != cursor_model_.get()) {
                throw std::runtime_error("Original CURZOR scene lookup did not resolve owned model");
            }
            const char* name = engine_frame_name(cursor_frame_);
            if (name == nullptr || std::strcmp(name, "CURZOR") != 0) {
                throw std::runtime_error("Original CURZOR frame name failed verification");
            }
            cursor_position_ = scene_method<CursorPosition>(cursor_frame_, module, 8, 0x1c9f0);
            cursor_update_ = scene_method<CursorUpdate>(cursor_frame_, module, 0x24, 0x1e610);
            std::uint32_t cursor_nodes = 0;
            log_hierarchy(cursor_model_.get(), module, 0, cursor_nodes);
            std::printf("Original CURZOR model bound: %u frames; hidden scene model\n",
                        static_cast<unsigned>(cursor_nodes));
        }
        if (bind_cursor_model) {
            // 0066ad10 creates this frame even when sound is unavailable. The
            // host initially leaves it unbuffered; loading follows if audio initialized.
            click_sound_ = driver_.create(4);
            auto* sound = click_sound_.get();
            scene_method<Integer>(sound, module, 0x7c, 0x549a0)(sound, 3);
            using Name = std::int32_t (WINAPI *)(EngineFrame*, const char*);
            if (scene_method<Name>(sound, module, 0x34, 0x1d330)(sound, "sound_click") < 0) {
                throw std::runtime_error("Cannot name original click-sound frame");
            }
            click_sound_enable_ = scene_method<SoundEnable>(sound, module, 0x30, 0x1bb10);
            // This is the SOUND override, not visual frame visibility 1001cfe0.
            using SoundPlay = void (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
            scene_method<SoundPlay>(sound, module, 0xac, 0x54f30);
            click_sound_enable_(sound, 0);
            scene_method<Float>(sound, module, 0x8c, 0x54c40)(sound, 1.0f);
            click_sound_active_ = scene_method<SoundActive>(sound, module, 0x78, 0x548d0);
            add(scene_.get(), sound);
            scene_method<Pair>(sector, module, 0x7c, 0x4dcb0)(sector, sound);
            const char* name = engine_frame_name(sound);
            if (name == nullptr || std::strcmp(name, "sound_click") != 0) {
                throw std::runtime_error("Original click-sound frame name failed verification");
            }
            std::puts("Original sound_click frame bound: type 3, volume 1; no audio buffer loaded");
        }
        render_ = scene_method<Render>(scene_.get(), module, 0x78, 0x46860);
        std::puts("Original model opened; static artwork preview, no menu interaction");
        std::fflush(stdout);
    }

public:
    std::int32_t render() const {
        return render_(scene_.get(), 0);
    }

    void update_input_cursor(float x, float y, float width, float height) const {
        if (cursor_frame_ == nullptr) throw std::logic_error("CURZOR model is not bound");
        struct FrameAdapter {
            const MenuScene& scene;
            void set_position(const std::array<float, 3>& position) {
                scene.cursor_position_(scene.cursor_frame_, position.data());
            }
            void update() { scene.cursor_update_(scene.cursor_frame_); }
        } adapter{*this};
        frontend::update_cursor_frame(adapter, x, y, width, height);
        if (!cursor_update_logged_) {
            const auto position = frontend::cursor_frame_position(x, y, width, height);
            std::printf("Original CURZOR position/update: (%.6f,%.6f,%.1f)\n",
                        position[0], position[1], position[2]);
            cursor_update_logged_ = true;
        }
    }

    std::uint32_t pick_main_menu(HMODULE module, float x, float y,
                                 float width, float height) const {
        return pick_menu_screen(module, x, y, width, height, false);
    }

    std::uint32_t pick_confirmation(HMODULE module, float x, float y,
                                    float width, float height) const {
        return pick_menu_screen(module, x, y, width, height, true);
    }

    std::uint32_t pick_menu_screen(HMODULE module, float x, float y,
                                   float width, float height, bool confirmation) const {
        EngineFrame* root = confirmation ? confirmation_.get() : model_.get();
        if (root == nullptr) throw std::logic_error("Menu picking model is not initialized");
        using Pick = EngineFrame* (WINAPI *)(EngineFrame*, const float*, const float*,
                                             EngineFrame*, void*, std::uint32_t,
                                             std::uint32_t, std::uint32_t, std::uint32_t);
        const auto pick = scene_method<Pick>(scene_.get(), module, 0x104, 0x110f0);
        const auto ray = frontend::menu_pointer_ray(x, y, width, height);
        alignas(16) std::array<std::uint8_t, 64> details{};
        return frontend::menu_pointer_target(ray,
            [&](const frontend::PointerRay& input, std::uint32_t flags) {
                return pick(scene_.get(), input.origin.data(), input.direction.data(),
                            root, details.data(), flags, 0, 0, 0);
            },
            [](EngineFrame* frame) {
                std::uint32_t target = 0;
                std::memcpy(&target, reinterpret_cast<const unsigned char*>(frame) + 0xec,
                            sizeof(target));
                return target;
            });
    }

    EngineFrame* find_control_frame(EngineFrame* root, const char* name, HMODULE module) const {
        EngineFrame* found = nullptr;
        std::uint32_t nodes = 0;
        find_named(root, name, module, 0, nodes, found);
        return found;
    }
    EngineFrame* control_frame(const char* name, HMODULE module,
                               bool confirmation = false, EngineFrame* search_root = nullptr) const {
        EngineFrame* root = search_root ? search_root : (confirmation ? confirmation_.get() : model_.get());
        auto* found = find_control_frame(root, name, module);
        if (found == nullptr) {
            throw std::runtime_error(std::string("Missing original menu frame: ") + name);
        }
        return found;
    }

    // 0066dbf0: create a named engine frame, link it to its parent, set its
    // position and retain it with the frontend. The original accepts other
    // frame kinds; this bounded bridge is for verified kind-six dummy groups.
    EngineFrame* create_group_frame(HMODULE module, EngineFrame* parent,
                                    const char* name, const std::array<float, 3>& position) {
        if (!parent || !name) throw std::invalid_argument("Group frame needs a parent and name");
        auto frame = driver_.create(6);
        if (reinterpret_cast<std::uintptr_t>(frame->methods) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x9bba8) {
            throw std::runtime_error("Unsupported original dummy-frame vtable");
        }
        using Link = std::int32_t (WINAPI *)(EngineFrame*, EngineFrame*, std::uint32_t);
        using Name = std::int32_t (WINAPI *)(EngineFrame*, const char*);
        using Position = void (WINAPI *)(EngineFrame*, const float*);
        if (scene_method<Link>(frame.get(), module, 0x38, 0x1d680)(frame.get(), parent, 0) < 0)
            throw std::runtime_error("Cannot link original dummy group frame");
        if (scene_method<Name>(frame.get(), module, 0x34, 0x1d330)(frame.get(), name) < 0)
            throw std::runtime_error("Cannot name original dummy group frame");
        scene_method<Position>(frame.get(), module, 8, 0x1c9f0)(frame.get(), position.data());
        EngineFrame* result = frame.get();
        group_frames_.push_back(std::move(frame));
        return result;
    }
    EngineFrame* main_model_frame() const { return model_.get(); }
    EngineFrame* confirmation_model_frame() const { return confirmation_.get(); }
    EngineFrame* runtime_confirmation_frame() const { return runtime_confirmation_.get(); }
    void position_menu_model(HMODULE module, EngineFrame* frame, float x, float y, float z) const {
        using Position = void (WINAPI *)(EngineFrame*, const float*);
        const float position[]{x, y, z};
        scene_method<Position>(frame, module, 8, 0x1c9f0)(frame, position);
    }

    EngineFrame* label_visual(const char* name, const char* visual, HMODULE module,
                              bool confirmation = false, EngineFrame* search_root = nullptr) const {
        EngineFrame* found = control_frame(name, module, confirmation, search_root);
        std::uint32_t nodes = 0;
        if (std::strcmp(name, visual) != 0) {
            EngineFrame* child = nullptr;
            nodes = 0;
            find_named(found, visual, module, 0, nodes, child);
            if (child == nullptr) {
                throw std::runtime_error(std::string("Missing visual in menu button: ") + name);
            }
            found = child;
        }
        if (reinterpret_cast<std::uintptr_t>(found->methods) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x9c7c8) {
            throw std::runtime_error(std::string("Unsupported menu label visual: ") + name);
        }
        return found;
    }

    std::array<float, 8> label_bounds(const char* name, const char* visual, HMODULE module,
                                      bool confirmation = false, EngineFrame* search_root = nullptr) const {
        EngineFrame* found = label_visual(name, visual, module, confirmation, search_root);
        const auto entry = GetProcAddress(module, "?UpdateWBoundProc@I3D_visual@@AAGXXZ");
        if (reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x571b0) {
            throw std::runtime_error("Unsupported visual world-bounds export");
        }
        using UpdateBounds = void (WINAPI *)(EngineFrame*);
        UpdateBounds update = nullptr;
        std::memcpy(&update, &entry, sizeof(entry));
        update(found);
        std::array<float, 8> bounds{};
        std::memcpy(bounds.data(), reinterpret_cast<const char*>(found) + 0x170, sizeof(bounds));
        return bounds;
    }

    void set_confirmation_visible(HMODULE module, bool visible) const {
        set_screen_visible(module, true, visible);
    }

    bool screen_visible(bool confirmation) const {
        return screen_visible(confirmation ? confirmation_.get() : model_.get());
    }
    bool screen_visible(EngineFrame* frame) const {
        if (!frame) throw std::logic_error("Screen model is not loaded");
        return (reinterpret_cast<const unsigned char*>(frame)[0xe0] & 1) != 0;
    }

    void set_screen_visible(HMODULE module, bool confirmation, bool visible) const {
        set_screen_visible(module, confirmation ? confirmation_.get() : model_.get(), visible);
    }
    void set_screen_visible(HMODULE module, EngineFrame* frame, bool visible) const {
        set_screen_enabled(module, frame, visible);
        update_screen_frame(module, frame);
    }

    void set_screen_enabled(HMODULE module, bool confirmation, bool visible) const {
        set_screen_enabled(module, confirmation ? confirmation_.get() : model_.get(), visible);
    }
    void set_screen_enabled(HMODULE module, EngineFrame* frame, bool visible) const {
        using Visibility = void (WINAPI *)(EngineFrame*, std::uint8_t);
        scene_method<Visibility>(frame, module, 0x30, 0x1cfe0)(frame, visible ? 1 : 0);
    }

    void update_screen_frame(HMODULE module, bool confirmation) const {
        update_screen_frame(module, confirmation ? confirmation_.get() : model_.get());
    }
    void update_screen_frame(HMODULE module, EngineFrame* frame) const {
        using Update = void (WINAPI *)(EngineFrame*);
        scene_method<Update>(frame, module, 0x24, 0x1e610)(frame);
    }

    class ModalPass {
    public:
        ModalPass(const MenuScene& scene, HMODULE module, EngineFrame* overlay = nullptr)
            : scene_(scene), state_(scene.driver_.render_state(5)),
              overlay_(overlay ? overlay : scene.confirmation_.get()),
              visibility_(scene_method<Visibility>(scene.model_.get(), module, 0x30, 0x1cfe0)),
              update_(scene_method<Update>(scene.model_.get(), module, 0x24, 0x1e610)) {
            if (scene.driver_.set_render_state(5, 0) < 0) {
                throw std::runtime_error("Cannot configure original modal render pass");
            }
            visibility_(scene.model_.get(), 0);
            visibility_(overlay_, 1);
            update_(scene.model_.get());
            update_(overlay_);
        }
        ~ModalPass() {
            if (!restored_ && !restore()) {
                std::fputs("Failed to restore original modal render state\n", stderr);
            }
        }
        bool restore() {
            visibility_(scene_.model_.get(), 1);
            visibility_(overlay_, 1);
            update_(scene_.model_.get());
            update_(overlay_);
            restored_ = true;
            return scene_.driver_.set_render_state(5, state_) >= 0;
        }
        ModalPass(const ModalPass&) = delete;
        ModalPass& operator=(const ModalPass&) = delete;

    private:
        using Visibility = void (WINAPI *)(EngineFrame*, std::uint8_t);
        using Update = void (WINAPI *)(EngineFrame*);
        const MenuScene& scene_;
        std::uint32_t state_;
        EngineFrame* overlay_;
        Visibility visibility_;
        Update update_;
        bool restored_ = false;
    };

    void initialize_confirmation_text_frame(HMODULE module, EngineFrame* root = nullptr) const {
        EngineFrame* frame = label_visual("text", "text", module, true, root);
        using Direction = void (WINAPI *)(EngineFrame*, const float*, const float*);
        using Update = void (WINAPI *)(EngineFrame*);
        const float direction[] = {0, 0, -1};
        scene_method<Direction>(frame, module, 0x0c, 0x1c8d0)(frame, direction, nullptr);
        scene_method<Update>(frame, module, 0x24, 0x1e610)(frame);
    }

    void load_click_sound(HMODULE module) {
        if (!click_sound_) throw std::logic_error("Click-sound frame is not bound");
        using OpenSound = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        const auto status = scene_method<OpenSound>(click_sound_.get(), module, 0x74, 0x545a0)(
            click_sound_.get(), "Sounds\\button.wav", 0);
        if (status < 0) throw std::runtime_error("Cannot load original button.wav sound buffer");
        // 100548d0's active query treats +13c as its buffer pointer. Borrow only.
        EngineFrame* buffer = nullptr;
        std::memcpy(&buffer, reinterpret_cast<const unsigned char*>(click_sound_.get()) + 0x13c,
                    sizeof(buffer));
        if (buffer == nullptr) throw std::runtime_error("Click-sound loader returned no sound buffer");
        click_sound_loaded_ = true;
        std::puts("Original button.wav loaded: sound buffer bound for accepted pointer clicks");
    }

    EngineFrame* click_sound() const { return click_sound_.get(); }
    bool sound_active(EngineFrame* sound, bool actual) const {
        return click_sound_active_(sound, actual) != 0;
    }
    void enable_sound(EngineFrame* sound, bool enabled) const {
        click_sound_enable_(sound, enabled);
    }
    void request_click_sound() const {
        frontend::accepted_menu_click(*this);
        std::printf("Original accepted-click sound callback: %s\n",
                    click_sound_loaded_ ? "buffered" : "no audio buffer loaded");
    }

private:
    static void find_named(EngineFrame* frame, const char* name, HMODULE module,
                            std::uint32_t depth, std::uint32_t& nodes, EngineFrame*& found) {
        if (frame == nullptr || depth > 16 || ++nodes > 256) {
            throw std::runtime_error("Invalid original menu binding hierarchy");
        }
        const char* frame_name = engine_frame_name(frame);
        const auto length = std::strlen(name);
        const bool suffix_pattern = length != 0 && name[length - 1] == '*';
        if (frame_name != nullptr && (std::strcmp(frame_name, name) == 0 ||
                (suffix_pattern && std::strncmp(frame_name, name, length - 1) == 0))) {
            if (found != nullptr) {
                throw std::runtime_error(std::string("Ambiguous original menu frame: ") + name);
            }
            found = frame;
        }
        using Count = std::uint32_t (WINAPI *)(EngineFrame*);
        using Child = EngineFrame* (WINAPI *)(EngineFrame*, std::uint32_t);
        const auto count = scene_method<Count>(frame, module, 0x3c, 0x1db80)(frame);
        if (count > 256) {
            throw std::runtime_error("Invalid original menu binding child count");
        }
        const auto child = scene_method<Child>(frame, module, 0x40, 0x1dba0);
        for (std::uint32_t i = 0; i < count; ++i) {
            find_named(child(frame, i), name, module, depth + 1, nodes, found);
        }
    }

    static void log_hierarchy(EngineFrame* frame, HMODULE module, std::uint32_t depth,
                              std::uint32_t& nodes) {
        if (depth > 16 || ++nodes > 256) {
            throw std::runtime_error("Menu model hierarchy exceeds diagnostic bounds");
        }
        using Count = std::uint32_t (WINAPI *)(EngineFrame*);
        using Child = EngineFrame* (WINAPI *)(EngineFrame*, std::uint32_t);
        const auto count = scene_method<Count>(frame, module, 0x3c, 0x1db80)(frame);
        if (count > 256) {
            throw std::runtime_error("Invalid menu model child count");
        }
        const auto child = scene_method<Child>(frame, module, 0x40, 0x1dba0);
        const char* name = engine_frame_name(frame);
        std::printf("Menu node depth=%u vtable_RVA=0x%08x children=%u name=%.128s\n",
                    static_cast<unsigned>(depth),
                    static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(frame->methods) -
                                          reinterpret_cast<std::uintptr_t>(module)),
                    static_cast<unsigned>(count), name == nullptr ? "<unnamed>" : name);
        for (std::uint32_t i = 0; i < count; ++i) {
            EngineFrame* next = child(frame, i);
            if (next == nullptr) {
                throw std::runtime_error("Menu child getter returned null within child count");
            }
            log_hierarchy(next, module, depth + 1, nodes);
        }
    }

    using Render = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
    std::unique_ptr<MenuDriver> owned_driver_;
    MenuDriver& driver_;
    MenuDriver::Owner scene_;
    MenuDriver::Owner camera_;
    MenuDriver::Owner light_;
    MenuDriver::Owner model_;
    MenuDriver::Owner confirmation_{nullptr, nullptr};
    MenuDriver::Owner runtime_confirmation_{nullptr, nullptr};
    MenuDriver::Owner cursor_model_{nullptr, nullptr};
    MenuDriver::Owner click_sound_{nullptr, nullptr};
    // Released before the models and driver; links retain their engine references.
    std::vector<MenuDriver::Owner> group_frames_;
    using SoundActive = std::uint8_t (WINAPI *)(EngineFrame*, std::uint8_t);
    using SoundEnable = void (WINAPI *)(EngineFrame*, std::uint32_t);
    SoundActive click_sound_active_ = nullptr;
    SoundEnable click_sound_enable_ = nullptr;
    bool click_sound_loaded_ = false;
    using CursorPosition = void (WINAPI *)(EngineFrame*, const float*);
    using CursorUpdate = void (WINAPI *)(EngineFrame*);
    EngineFrame* cursor_frame_ = nullptr; // Borrowed from owned cursor model.
    CursorPosition cursor_position_ = nullptr;
    CursorUpdate cursor_update_ = nullptr;
    mutable bool cursor_update_logged_ = false;
    Render render_ = nullptr;
};

}

#endif
