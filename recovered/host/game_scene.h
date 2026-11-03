#ifndef HD2_HOST_GAME_SCENE_H
#define HD2_HOST_GAME_SCENE_H

#include "host/menu_scene.h"
#include "host/graphics_interface.h"
#include <exception>

namespace hd2::host {

// Required client dispatcher006C9260; implement with game::scene_callback and
// actual render services. No empty/default receiver is installed by this bridge.
class GameSceneCallbacks {
public:
    virtual ~GameSceneCallbacks() = default;
    virtual std::uint32_t dispatch_game_scene(std::uint32_t stage) = 0;
};

// Engine part of00698510/00699270 only. Creates the real kind13 scene, installs
// its callback and matches physical viewport, with the original stdcall ABI.
// No menu projection, dummy camera, loaded mission or audio world is fabricated.
// The caller must keep driver/graphics/module/callback receiver alive until close.
class GameScene {
public:
    GameScene(MenuDriver& driver, HMODULE module, Graphics& graphics, GameSceneCallbacks& callbacks)
        : module_(module), graphics_(graphics), callbacks_(callbacks), scene_(driver.create(13)) {
        set_callback_ = scene_method<SetCallback>(scene_.get(), module_, 0x74, 0x46820);
        set_viewport_ = scene_method<SetViewport>(scene_.get(), module_, 0x148, 0x48440);
        // Resolve checked width/height targets before modifying scene registration.
        auto* graphics_frame = reinterpret_cast<EngineFrame*>(&graphics_);
        width_ = scene_method<Dimension>(graphics_frame, module_, 0x18, 0x6de50);
        height_ = scene_method<Dimension>(graphics_frame, module_, 0x1c, 0x6de70);
    }
    ~GameScene() { close(); }
    GameScene(const GameScene&) = delete;
    GameScene& operator=(const GameScene&) = delete;
    void install_callback() {
        if (active_ || registered_) throw std::logic_error("A game scene callback is already registered");
        active_ = this;
        set_callback_(scene_.get(), &callback, 6);
        registered_ = true;
    }
    void fit_viewport() {
        const auto width = width_(reinterpret_cast<EngineFrame*>(&graphics_));
        const auto height = height_(reinterpret_cast<EngineFrame*>(&graphics_));
        std::array<std::int32_t, 4> viewport{};
        std::memcpy(viewport.data(), reinterpret_cast<const char*>(scene_.get()) + 0x244,
                    sizeof(viewport));
        if (viewport != std::array<std::int32_t, 4>{0, 0, width, height})
            set_viewport_(scene_.get(), 0, 0, width, height);
    }
    EngineFrame* frame() const { return scene_.get(); }
    void rethrow_callback_failure() const {
        if (callback_failure_) std::rethrow_exception(callback_failure_);
    }
    void close() noexcept {
        if (registered_) {
            //10046820(null) clears both callback and flags. Clear before release
            // so no engine-held callback can outlive the client dispatcher.
            set_callback_(scene_.get(), nullptr, 0);
            registered_ = false;
        }
        scene_.reset();
        if (active_ == this) active_ = nullptr;
    }
private:
    using Callback = std::uint32_t (WINAPI *)(std::uint32_t, std::uint32_t, std::uint32_t);
    using SetCallback = std::uintptr_t (WINAPI *)(EngineFrame*, Callback, std::uint32_t);
    using SetViewport = void (WINAPI *)(EngineFrame*, std::int32_t, std::int32_t,
                                      std::int32_t, std::int32_t);
    using Dimension = std::int32_t (WINAPI *)(EngineFrame*);
    static std::uint32_t WINAPI callback(std::uint32_t stage, std::uint32_t, std::uint32_t) noexcept {
        // Only the one installed scene owns this entry. Never unwind C++ through
        // LS3DF; latch an actual failure for the caller to rethrow after its call.
        auto& self = *active_;
        if (!self.callback_failure_) {
            try { return self.callbacks_.dispatch_game_scene(stage); }
            catch (...) { self.callback_failure_ = std::current_exception(); }
        }
        return 0; // Original callback always returns0; this is not load success.
    }
    inline static GameScene* active_ = nullptr;
    HMODULE module_;
    Graphics& graphics_;
    GameSceneCallbacks& callbacks_;
    MenuDriver::Owner scene_;
    SetCallback set_callback_ = nullptr;
    SetViewport set_viewport_ = nullptr;
    Dimension width_ = nullptr, height_ = nullptr;
    bool registered_ = false;
    std::exception_ptr callback_failure_;
};

} // namespace hd2::host
#endif
