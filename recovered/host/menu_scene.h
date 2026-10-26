#ifndef HD2_HOST_MENU_SCENE_H
#define HD2_HOST_MENU_SCENE_H

#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>

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
                      "Unsupported menu ABI: slot 0x%x, expected RVA 0x%x",
                      static_cast<unsigned>(offset), static_cast<unsigned>(rva));
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

private:
    using Close = std::int32_t (WINAPI *)(EngineFrame*);
    HMODULE module_;
    EngineFrame* driver_ = nullptr;
    Close close_ = nullptr;
    Factory factory_ = nullptr;
};

class MenuScene {
public:
    explicit MenuScene(HMODULE module)
        : driver_(module), scene_(driver_.create(13)), camera_(driver_.create(3)),
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

        std::puts("Opening original model: Models\\main_menu.4ds");
        std::fflush(stdout);
        const auto status = scene_method<Open>(model_.get(), module, 0x74, 0x340b0)(
            model_.get(), "Models\\main_menu", 0, nullptr, nullptr);
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
        render_ = scene_method<Render>(scene_.get(), module, 0x78, 0x46860);
        std::puts("Original menu model opened; static artwork preview, no menu interaction");
        std::fflush(stdout);
    }

    std::int32_t render() const {
        return render_(scene_.get(), 0);
    }

private:
    using Render = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
    MenuDriver driver_;
    MenuDriver::Owner scene_;
    MenuDriver::Owner camera_;
    MenuDriver::Owner light_;
    MenuDriver::Owner model_;
    Render render_ = nullptr;
};

}

#endif
