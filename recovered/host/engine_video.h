#ifndef HD2_HOST_ENGINE_VIDEO_H
#define HD2_HOST_ENGINE_VIDEO_H

#include "host/menu_scene.h"
#include "frontend/opening_playback.h"
#include "frontend/opening_render.h"

namespace hd2::host {

// Verified LS3DF IShow boundary. No direct Bink API or copied client routines.
class EngineVideo {
public:
    EngineVideo(EngineFrame* graphics, HMODULE module) : graphics_(graphics), module_(module) {
        factory_ = scene_method<Factory>(graphics, module, 0x38, 0x69e80);
        clear_ = scene_method<Clear>(graphics, module, 0x64, 0x6a6c0);
        present_ = scene_method<Unary>(graphics, module, 0x70, 0x6b3e0);
        begin_ = scene_method<Unary>(graphics, module, 0x68, 0x6b380);
        end_ = scene_method<Unary>(graphics, module, 0x6c, 0x6b3b0);
        get_state_ = scene_method<GetState>(graphics, module, 0x78, 0x6b220);
        set_state_ = scene_method<SetState>(graphics, module, 0x74, 0x6abc0);
    }
    ~EngineVideo() { close(); }
    EngineVideo(const EngineVideo&) = delete;
    EngineVideo& operator=(const EngineVideo&) = delete;

    void open(const char* filename, std::uint32_t flags) {
        if (video_) throw std::logic_error("Opening movie already owned");
        EngineFrame* created = factory_(graphics_);
        if (!created) throw std::runtime_error("Original video factory returned null");
        // Verify release before taking ownership; the fingerprinted factory
        // installs 1009d9c0, including every slot checked below.
        release_ = scene_method<Result>(created, module_, 4, 0x70fd0);
        video_ = created;
        close_ = scene_method<Result>(video_, module_, 0x10, 0x71b80);
        stop_ = scene_method<Result>(video_, module_, 0x18, 0x71d10);
        play_ = scene_method<Result>(video_, module_, 0x14, 0x71c50);
        status_ = scene_method<Result>(video_, module_, 0x2c, 0x72160);
        time_ = scene_method<GetTime>(video_, module_, 0x38, 0x72400);
        opened_ = scene_method<Opened>(video_, module_, 8, 0x71b70);
        dimensions_ = scene_method<GetRectangle>(video_, module_, 0x28, 0x720e0);
        rectangle_ = scene_method<SetRectangle>(video_, module_, 0x20, 0x72040);
        volume_ = scene_method<Volume>(video_, module_, 0x3c, 0x724d0);
        draw_ = scene_method<Draw>(video_, module_, 0x50, 0x72650);
        const auto open_video = scene_method<Open>(video_, module_, 0x0c, 0x71630);
        const std::string path = std::string("GameMovie\\") + filename;
        const auto status = open_video(video_, path.c_str(), flags);
        std::printf("Opening movie: %s status=%08x opened=%u\n", path.c_str(),
                    static_cast<unsigned>(status), static_cast<unsigned>(opened()));
    }
    bool opened() const { return opened_(video_) != 0; }
    frontend::VideoRectangle dimensions() const {
        frontend::VideoRectangle result{};
        if (dimensions_(video_, &result) < 0)
            throw std::runtime_error("Cannot query original video dimensions");
        return result;
    }
    void rectangle(const frontend::VideoRectangle& rectangle) { rectangle_(video_, &rectangle); }
    void volume(float volume) { volume_(video_, volume); }
    void play() { play_(video_); }
    void stop() { if (video_ && stop_) stop_(video_); }
    std::int32_t playback_status() const { return status_(video_); }
    // Original00688C10 ignores HRESULT. Unopened/error must preserve output.
    void elapsed_time(std::uint32_t& milliseconds) const { time_(video_, &milliseconds); }
    void clear_present() const {
        clear_(graphics_, 0, 1.0f, 3);
        present_(graphics_);
    }
    void close(bool clear_buffers = true) noexcept {
        if (!video_) return;
        if (clear_buffers) for (int i = 0; i < 3; ++i) clear_present();
        if (close_) close_(video_);
        release_(video_);
        video_ = nullptr;
    }
    template<class TextPass>
    void draw_with_text(bool has_text, TextPass&& text_pass, bool present = false) const {
        RenderServices renderer{*this};
        frontend::render_opening_frame(renderer, has_text, std::forward<TextPass>(text_pass), present);
    }
    void draw(bool present = true) const { draw_with_text(false, [] {}, present); }

private:
    struct RenderServices {
        const EngineVideo& owner;
        std::uint32_t graphics_state() const { return owner.get_state_(owner.graphics_, 0); }
        void graphics_state(std::uint32_t value) const { owner.set_state_(owner.graphics_, 0, value); }
        void clear() const { owner.clear_(owner.graphics_, 0, 1.0f, 3); }
        bool has_movie() const { return owner.video_ != nullptr; }
        void begin() const { owner.begin_(owner.graphics_); }
        void end() const { owner.end_(owner.graphics_); }
        void draw_movie() const { owner.draw_(owner.video_, 0); }
        void present() const { owner.present_(owner.graphics_); }
    };
    using Result = std::int32_t (WINAPI *)(EngineFrame*);
    using Unary = void (WINAPI *)(EngineFrame*);
    using Factory = EngineFrame* (WINAPI *)(EngineFrame*);
    using Clear = void (WINAPI *)(EngineFrame*, std::uint32_t, float, std::uint32_t);
    using GetState = std::uint32_t (WINAPI *)(EngineFrame*, std::uint32_t);
    using SetState = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
    using Open = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
    using Opened = std::uint8_t (WINAPI *)(EngineFrame*);
    using GetRectangle = std::int32_t (WINAPI *)(EngineFrame*, frontend::VideoRectangle*);
    using GetTime = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t*);
    using SetRectangle = std::int32_t (WINAPI *)(EngineFrame*, const frontend::VideoRectangle*);
    using Volume = std::int32_t (WINAPI *)(EngineFrame*, float);
    using Draw = std::int32_t (WINAPI *)(EngineFrame*, std::uint8_t);
    EngineFrame* graphics_;
    HMODULE module_;
    EngineFrame* video_ = nullptr;
    Factory factory_{};
    Clear clear_{};
    Unary begin_{}, end_{}, present_{};
    GetState get_state_{};
    SetState set_state_{};
    Result release_{}, close_{}, play_{}, stop_{}, status_{};
    Opened opened_{};
    GetRectangle dimensions_{};
    GetTime time_{};
    SetRectangle rectangle_{};
    Volume volume_{};
    Draw draw_{};
};

} // namespace hd2::host
#endif
