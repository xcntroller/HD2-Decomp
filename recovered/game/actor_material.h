#ifndef HD2_GAME_ACTOR_MATERIAL_H
#define HD2_GAME_ACTOR_MATERIAL_H
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
namespace hd2::game {
//00611090: case-sensitive first substring, atoi after "matid"; absent=-1.
//Unlike Ladder it neither lowercases the text nor requires a numeric suffix.
inline std::int32_t actor_material_id(const std::string& text) {
    const auto at=text.find("matid");if(at==std::string::npos)return -1;
    const auto value=std::strtoll(text.c_str()+at+5,nullptr,10);
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Actor material ID exceeds native atoi bounds");
    return static_cast<std::int32_t>(value);
}
//00611090's NULL-frame gate, shared by the actual D8 receivers below.
template<class Frames>std::int32_t actor_frame_material(Frames& frames,typename Frames::Frame frame){
    return frame?actor_material_id(frames.actor_user_data(frame)):-1;
}
//004DBEE0/005B5F20/005E2910 reject EVERY negative ID, not just-1.
template<class Frames>std::int32_t actor_nonnegative_material(Frames& frames,typename Frames::Frame picked){
    const auto value=actor_frame_material(frames,picked);return value<0?11:value;
}
//006023B0/0060BE40/0060CEE0 use a model fallback only for exactly-1.
template<class Frames>std::int32_t actor_picked_or_model_material(Frames& frames,
        typename Frames::Frame picked,typename Frames::Frame model){
    const auto value=actor_frame_material(frames,picked);if(value!=-1)return value;
    if(picked!=model){const auto fallback=actor_frame_material(frames,model);if(fallback!=-1)return fallback;}
    return 11;
}
} //namespace hd2::game
#endif
