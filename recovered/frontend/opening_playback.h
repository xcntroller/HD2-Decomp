#ifndef HD2_FRONTEND_OPENING_PLAYBACK_H
#define HD2_FRONTEND_OPENING_PLAYBACK_H

#include "frontend/opening_video.h"
#include <stdexcept>

namespace hd2::frontend {

struct VideoRectangle {
    std::int32_t x, y, width, height;
};

// 00688ab0: fit the video with five pixels of margin on the limiting axis.
// Keep the original divide-then-multiply and integer truncation order.
inline VideoRectangle opening_video_rectangle(std::int32_t width, std::int32_t height,
                                              std::int32_t movie_width,
                                              std::int32_t movie_height) {
    if (width <= 10 || height <= 10 || movie_width <= 0 || movie_height <= 0)
        throw std::invalid_argument("Invalid opening movie dimensions");
    if (static_cast<long double>(height) / width <
            static_cast<long double>(movie_height) / movie_width) {
        const auto fitted_width = static_cast<std::int32_t>(
            static_cast<long double>(height - 10) / movie_height * movie_width);
        return {(width - fitted_width) / 2, 5, fitted_width, height - 10};
    }
    const auto fitted_height = static_cast<std::int32_t>(
        static_cast<long double>(width - 10) / movie_width * movie_height);
    return {5, (height - fitted_height) / 2, width - 10, fitted_height};
}

// Ordinary opening Bink path of 00688040/00688c10/00688bb0, composed with the
// normal controller completion branch of 006c20f0. Engine owns codec/rendering.
// Subtitle, credits, music service and alternate command-line paths are outside
// this bounded reconstruction. Backend owns/reclaims its engine video object.
template<class Backend>
class OpeningPlayback {
public:
    OpeningPlayback(Backend& backend, std::int32_t width, std::int32_t height,
                    float volume)
        : backend_(backend), width_(width), height_(height), volume_(volume) {
        open(normal_opening_start());
    }

    bool finished() const { return finished_; }
    std::uint32_t sequence_index() const { return index_; }

    void key(std::uint32_t scan) {
        // 00688bb0 accepts Escape unconditionally. Space is only accepted for
        // the +1f latch used by special outro paths, absent in normal startup.
        if (!finished_ && status_ != 13 && scan == 1) complete();
    }

    void update() {
        if (finished_) return;
        if (status_ != 13) {
            const auto status = backend_.playback_status();
            if (status == -1 || status == 1) complete();
        }
        if (status_ != 13) return;
        const auto next = normal_opening_completion(index_);
        // Original 00688460 clears/presents three buffers, closes/releases the
        // video and descriptor; the following 0066eae0 creates a fresh object.
        backend_.close();
        if (next.action == OpeningVideoAction::enter_frontend) {
            finished_ = true;
        } else {
            open(next);
        }
    }

private:
    void complete() {
        backend_.stop();
        backend_.clear_present();
        status_ = 13; // 0066ed50/0066ed80 map video status 2 to controller 13.
    }
    void open(const OpeningVideoStep& step) {
        index_ = step.sequence_index;
        backend_.open(step.filename, 0x400);
        if (backend_.opened()) {
            const auto dimensions = backend_.dimensions();
            backend_.rectangle(opening_video_rectangle(width_, height_,
                                                       dimensions.width, dimensions.height));
            backend_.volume(volume_);
            backend_.play();
        }
        // Failed opens still become status 1; query -1 completes on update.
        status_ = 1;
    }
    Backend& backend_;
    std::int32_t width_, height_;
    float volume_;
    std::uint32_t index_ = 0, status_ = 0;
    bool finished_ = false;
};

} // namespace hd2::frontend
#endif
