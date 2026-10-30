#ifndef HD2_FRONTEND_OPENING_VIDEO_H
#define HD2_FRONTEND_OPENING_VIDEO_H

#include <cstdint>

namespace hd2::frontend {

// Normal startup branch of controller 006c0ca0 and the video-type/status-13
// branch of 006c20f0. Values are sequence indices stored at video state +0x7c,
// not the state type (10) or its completion status (13).
enum class OpeningVideoAction : std::uint8_t {
    none,
    logo1,
    logo2,
    logo3,
    intro,
    enter_frontend,
};

struct OpeningVideoStep {
    OpeningVideoAction action;
    const char* filename;
    std::uint32_t sequence_index;
    bool stop_current;
};

constexpr OpeningVideoStep normal_opening_start() {
    return {OpeningVideoAction::logo1, "Logo1.bik", 1, false};
}

constexpr OpeningVideoStep normal_opening_completion(std::uint32_t sequence_index) {
    switch (sequence_index) {
    case 1: return {OpeningVideoAction::logo2, "Logo2.bik", 2, true};
    case 2: return {OpeningVideoAction::logo3, "Logo3.bik", 3, true};
    case 3: return {OpeningVideoAction::intro, "Intro.bik", 4, true};
    case 4: return {OpeningVideoAction::enter_frontend, nullptr, 0, false};
    default: return {OpeningVideoAction::none, nullptr, sequence_index, false};
    }
}

} // namespace hd2::frontend

#endif
