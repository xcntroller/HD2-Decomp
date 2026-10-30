#ifndef HD2_HOST_OPENING_AUDIO_H
#define HD2_HOST_OPENING_AUDIO_H

#include "host/menu_scene.h"

namespace hd2::host {

// Separate ISnd voice/sample owners used by00688040/00689970/00688C10.
// Engine bodies are delegated; these are stdcall self-on-stack interfaces.
class OpeningAudio {
public:
    using Sample = MenuDriver::Owner;
    explicit OpeningAudio(HMODULE module) : module_(module), voice_(nullptr, nullptr) {
        const auto entry = GetProcAddress(module, "ISndGetDriver");
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module) + 0x68fa0)
            throw std::runtime_error("Unsupported opening ISndGetDriver");
        using Getter = EngineFrame* (__cdecl *)();
        Getter getter; std::memcpy(&getter, &entry, sizeof(getter));
        driver_ = getter();
        sample_factory_ = scene_method<Factory>(driver_, module, 0x20, 0x7aa80);
        voice_factory_ = scene_method<Factory>(driver_, module, 0x28, 0x7ab20);
    }
    ~OpeningAudio() { close_voice(); }
    OpeningAudio(const OpeningAudio&) = delete;
    OpeningAudio& operator=(const OpeningAudio&) = delete;
    void create_voice() {
        if (voice_) throw std::logic_error("Opening voice already owned");
        auto* created = voice_factory_(driver_);
        if (!created) throw std::runtime_error("Opening voice factory returned null");
        const auto release = scene_method<MenuDriver::Release>(created, module_, 0, 0x7bfb0);
        voice_ = Sample(created, release);
        close_ = scene_method<Unary>(created, module_, 8, 0x7c070);
        bind_ = scene_method<Bind>(created, module_, 4, 0x7bfe0);
        play_ = scene_method<Play>(created, module_, 0x0c, 0x7c0e0);
        volume_ = scene_method<Volume>(created, module_, 0x20, 0x7c9c0);
    }
    void close_voice() noexcept {
        if (voice_) {
            if (close_) close_(voice_.get());
            voice_.reset();
        }
    }
    bool has_voice() const { return voice_ != nullptr; }
    Sample create_sample() const {
        auto* created = sample_factory_(driver_);
        if (!created) throw std::runtime_error("Opening sample factory returned null");
        const auto release = scene_method<MenuDriver::Release>(created, module_, 0, 0x7cef0);
        return Sample(created, release);
    }
    std::int32_t load_sample(const Sample& sample, const char* path, std::uint32_t flags) const {
        using Load = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        return scene_method<Load>(sample.get(), module_, 4, 0x7cf30)(sample.get(), path, flags);
    }
    void bind_sample(const Sample& sample, std::uint32_t flags) { bind_(voice_.get(), sample.get(), flags); }
    void voice_volume(float value) { volume_(voice_.get(), value); }
    void play_voice(std::uint32_t flags, std::uint32_t offset) { play_(voice_.get(), flags, offset); }
private:
    using Factory = EngineFrame* (WINAPI *)(EngineFrame*);
    using Unary = std::int32_t (WINAPI *)(EngineFrame*);
    using Bind = std::int32_t (WINAPI *)(EngineFrame*, EngineFrame*, std::uint32_t);
    using Play = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
    using Volume = std::int32_t (WINAPI *)(EngineFrame*, float);
    HMODULE module_;
    EngineFrame* driver_;
    Factory sample_factory_, voice_factory_;
    Sample voice_;
    Unary close_ = nullptr;
    Bind bind_ = nullptr;
    Play play_ = nullptr;
    Volume volume_ = nullptr;
};

} // namespace hd2::host
#endif
