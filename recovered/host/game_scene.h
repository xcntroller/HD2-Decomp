#ifndef HD2_HOST_GAME_SCENE_H
#define HD2_HOST_GAME_SCENE_H

#include "host/menu_scene.h"
#include "host/graphics_interface.h"
#include <exception>
#include <map>

namespace hd2::host {

// Required client dispatcher006C9260; implement with game::scene_callback and
// actual render services. No empty/default receiver is installed by this bridge.
class GameSceneCallbacks {
public:
    virtual ~GameSceneCallbacks() = default;
    virtual std::uint32_t dispatch_game_scene(std::uint32_t stage) = 0;
};

class GameSceneLoadingProgress {
public:
    virtual ~GameSceneLoadingProgress() = default;
    virtual void scene_loading_progress(float progress) = 0;
};

// Engine part of00698510/00699270 only. Creates the real kind13 scene, installs
// its callback and matches physical viewport, with the original stdcall ABI.
// No menu projection, dummy camera, loaded mission or audio world is fabricated.
// The caller must keep driver/graphics/module/callback receiver alive until close.
class GameScene {
public:
    GameScene(MenuDriver& driver, HMODULE module, Graphics& graphics, GameSceneCallbacks& callbacks)
        : driver_(driver), module_(module), graphics_(graphics), callbacks_(callbacks), scene_(driver.create(13)) {
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
    void bind_callbacks(GameSceneCallbacks& callbacks){
        if(borrowed_callbacks_&&borrowed_callbacks_!=&callbacks)throw std::logic_error("Game scene already has a borrowed dispatcher");
        borrowed_callbacks_=&callbacks;
        try{if(!registered_)install_callback();}catch(...){borrowed_callbacks_=nullptr;throw;}
    }
    void unbind_callbacks(GameSceneCallbacks& callbacks)noexcept{if(borrowed_callbacks_==&callbacks)borrowed_callbacks_=nullptr;}
    void suspend_callback()noexcept{
        if(registered_){set_callback_(scene_.get(),nullptr,0);registered_=false;}
        if(active_==this)active_=nullptr;
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
    using Names = std::map<std::string, EngineFrame*>;
    std::int32_t open_mission_scene(const std::string& resource, GameSceneLoadingProgress& progress) {
        //006D4180: five stack arguments, scene/path/7/006D4C70/0. The cached
        //10047D20 decompilation loses one argument; the original ends RET20.
        using Open = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t,
                                           LoadingCallback, std::uint32_t);
        const auto open = scene_method<Open>(scene_.get(), module_, 0xf4, 0x47d20);
        if (loading_) throw std::logic_error("A mission scene open is already active");
        loading_ = this;
        progress_ = &progress;
        struct Reset {
            GameScene& owner;
            ~Reset() { owner.progress_ = nullptr; owner.loading_ = nullptr; }
        } reset{*this};
        const auto path = std::string("missions\\") + resource + "\\Scene";
        const auto result = open(scene_.get(), path.c_str(), 7, &loading_callback, 0);
        rethrow_callback_failure();
        return result;
    }
    void advance(std::uint32_t elapsed){ //0069AF10/0069B230, actual engine scene virtual20.
        using Advance=void(WINAPI*)(EngineFrame*,std::uint32_t);
        checked_engine_method<Advance>(scene_.get(),module_,0x20)(scene_.get(),elapsed);
        rethrow_callback_failure();
    }
    void render(){ //0069B870/Scene78(scene,0), actual checked LS3DF target.
        using Render=void(WINAPI*)(EngineFrame*,std::uint32_t);
        scene_method<Render>(scene_.get(),module_,0x78,0x46860)(scene_.get(),0);
        rethrow_callback_failure();
    }
    void refresh() {
        using Refresh = void (WINAPI *)(EngineFrame*);
        scene_method<Refresh>(scene_.get(), module_, 0xf0, 0x4be10)(scene_.get());
        rethrow_callback_failure();
    }
    void reset_mission_scene() {
        using Reset = void (WINAPI *)(EngineFrame*);
        scene_method<Reset>(scene_.get(), module_, 0xf8, 0x47db0)(scene_.get());
        rethrow_callback_failure();
    }
    EngineFrame* find_frame(const std::string& name, std::uint32_t mask = 0xcffff) const {
        using Find = EngineFrame* (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        return scene_method<Find>(scene_.get(), module_, 0x7c, 0x46920)(scene_.get(), name.c_str(), mask);
    }
    EngineFrame* resolve_frame_path(const std::string& name) const {
        //Driver+88 searches registered names within this actual scene; it is
        //distinct from scene+7C's subtree search. Callers select original order.
        using Resolve = EngineFrame* (WINAPI *)(EngineFrame*, EngineFrame*, const char*);
        return scene_method<Resolve>(driver_.frame(), module_, 0x88, 0x1a0f0)(
            driver_.frame(), scene_.get(), name.c_str());
    }
    void register_existing_frames(Names& names) {
        //006CBF30/006CC580. The scratch map borrows frames; sector entries are
        //inserted after enumeration, and duplicate/empty names are overwritten.
        NameEnumeration context{names, {}};
        using Enumerate = std::int32_t (WINAPI *)(EngineFrame*, NameCallback, void*,
                                                 std::uint32_t, const char*);
        scene_method<Enumerate>(scene_.get(), module_, 0x44, 0x46880)(
            scene_.get(), &register_name, &context, 0x4ffff, nullptr);
        //10046880 can return80000006 after a successful traversal. The original
        //ignores its result; callback failures are latched separately here.
        if (context.failure) std::rethrow_exception(context.failure);
        for (std::size_t offset : {0x1f8u,0x1fcu,0x200u}) {
            auto* sector = frame_field(offset);
            if (!sector) throw std::runtime_error("Mission scene sector is unavailable");
            const auto* name = engine_frame_name(sector);
            names[name ? name : ""] = sector;
        }
    }
    EngineFrame* prepare_mission_camera() {
        //006CBF30 configures the scene camera before applying Scene2. This
        //fallback is an original loader branch, not a substitute menu camera.
        auto* camera = frame_field(0x15c);
        using Pair = void (WINAPI *)(EngineFrame*, EngineFrame*);
        const auto active = scene_method<Pair>(scene_.get(), module_, 0x90, 0x471a0);
        if (!camera) {
            auto owner = driver_.create(3);
            camera = owner.get();
            active(scene_.get(), camera);
            scene_method<Pair>(scene_.get(), module_, 0x80, 0x469a0)(scene_.get(), camera);
            using Direction = void (WINAPI *)(EngineFrame*, const float*, const float*);
            using Position = void (WINAPI *)(EngineFrame*, const float*);
            //006CC2CC..006CC2E4 actual stack stores: (-1,-.1,-1).
            const float direction[] = {-1.f,-.1f,-1.f};
            const float position[] = {10.f,5.f,10.f};
            scene_method<Direction>(camera, module_, 0x1c, 0x1c400)(camera, direction, nullptr);
            scene_method<Position>(camera, module_, 8, 0x1c9f0)(camera, position);
            update_frame(camera);
            //Scene active/add references retain it after this local owner dies.
        }
        using Projection = void (WINAPI *)(EngineFrame*, std::uint8_t);
        using Range = void (WINAPI *)(EngineFrame*, float, float);
        using FieldOfView = void (WINAPI *)(EngineFrame*, float);
        scene_method<Projection>(camera, module_, 0x88, 0x09ff0)(camera, 0);
        scene_method<Range>(camera, module_, 0x7c, 0x09ef0)(camera, .1f, 500.f);
        const std::uint32_t bits = 0x3fb2b8c3;
        float field_of_view;
        std::memcpy(&field_of_view, &bits, sizeof(bits));
        scene_method<FieldOfView>(camera, module_, 0x74, 0x09e30)(camera, field_of_view);
        active(scene_.get(), camera);
        update_frame(camera);
        return camera; //Borrowed; usable-world readiness still belongs to loader.
    }
    void rethrow_callback_failure() {
        //Source exception delivery is consumed once, like actor cleanup errors.
        //A prior callback failure must not poison later unload/reset/re-entry.
        auto failure=std::exchange(callback_failure_,{});
        if(failure)std::rethrow_exception(failure);
    }
    void close() noexcept {
        suspend_callback(); //10046820(null), before any borrowed dispatcher/scene dies.
        borrowed_callbacks_=nullptr;
        scene_.reset();
        if (active_ == this) active_ = nullptr;
    }
private:
    using LoadingCallback = void (WINAPI *)(std::uint32_t, float, std::uint32_t, std::uint32_t);
    using NameCallback = std::uint32_t (WINAPI *)(EngineFrame*, void*);
    struct NameEnumeration { Names& names; std::exception_ptr failure; };
    static std::uint32_t WINAPI register_name(EngineFrame* frame, void* opaque) noexcept {
        auto& context = *static_cast<NameEnumeration*>(opaque);
        if (!context.failure) {
            try {
                if (!frame) throw std::runtime_error("Null frame in scene enumeration");
                const auto* name = engine_frame_name(frame);
                context.names[name ? name : ""] = frame;
            } catch (...) { context.failure = std::current_exception(); }
        }
        return 0; //006CC636 XOR EAX,EAX;006CC642 RET8.
    }
    static void WINAPI loading_callback(std::uint32_t event, float progress,
                                         std::uint32_t, std::uint32_t) noexcept {
        auto& owner = *loading_;
        if (event != 0x10 && event != 0x20 && !owner.callback_failure_) {
            try { owner.progress_->scene_loading_progress(progress); }
            catch (...) { owner.callback_failure_ = std::current_exception(); }
        }
    }
    EngineFrame* frame_field(std::size_t offset) const {
        EngineFrame* result = nullptr;
        std::memcpy(&result, reinterpret_cast<const unsigned char*>(scene_.get()) + offset, sizeof(result));
        return result;
    }
    void update_frame(EngineFrame* frame) const {
        using Update = void (WINAPI *)(EngineFrame*);
        scene_method<Update>(frame, module_, 0x24, 0x1e610)(frame);
    }
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
            try { return (self.borrowed_callbacks_?*self.borrowed_callbacks_:self.callbacks_).dispatch_game_scene(stage); }
            catch (...) { self.callback_failure_ = std::current_exception(); }
        }
        return 0; // Original callback always returns0; this is not load success.
    }
    inline static GameScene* active_ = nullptr;
    inline static GameScene* loading_ = nullptr;
    MenuDriver& driver_;
    HMODULE module_;
    Graphics& graphics_;
    GameSceneCallbacks& callbacks_;
    GameSceneCallbacks* borrowed_callbacks_=nullptr; //Explicit containing presentation loan, retracted before its owners retire.
    MenuDriver::Owner scene_;
    SetCallback set_callback_ = nullptr;
    SetViewport set_viewport_ = nullptr;
    Dimension width_ = nullptr, height_ = nullptr;
    bool registered_ = false;
    GameSceneLoadingProgress* progress_ = nullptr; //Borrowed only during synchronous open.
    std::exception_ptr callback_failure_;
};

} // namespace hd2::host
#endif
