#ifndef HD2_GAME_HUD_TEXT_LOADING_H
#define HD2_GAME_HUD_TEXT_LOADING_H
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
namespace hd2::game {
//006449D0: actual camera/visual world-Z and near/far clip, not an invented
//screen-space depth. A null scene returns0; missing visual/camera or out-of-range
//depth returns-1. Native unordered comparisons retain NaN.
template<class Frames>float hud_text_depth(Frames& frames,typename Frames::Frame scene,typename Frames::Frame visual) {
    if(!scene)return 0;
    const auto camera=frames.hud_active_camera(scene);
    if(!visual||!camera)return -1;
    const auto clips=frames.hud_camera_clips(camera); //Read BEFORE either world-matrix update.
    const auto camera_matrix=frames.actor_world_matrix(camera);
    const auto visual_matrix=frames.actor_world_matrix(visual);
    const auto result=(std::abs(static_cast<long double>(visual_matrix[14])-camera_matrix[14])-.1f-clips[0])/
        (static_cast<long double>(clips[1])-clips[0]);
    return result<0||result>1?-1.f:static_cast<float>(result);
}
//00644A80, actual TextState geometry8/bytes70. Signed native WORD dimensions,
//FLOAT constants and x87 store order matter. No layout/rendering is invented.
template<class Frames,class Text>void bind_hud_text_geometry(Frames& frames,Text& text,
        typename Frames::Frame scene,typename Frames::Frame visual) {
    if(!visual)return;
    const auto bounds=frames.hud_text_world_bounds(visual);
    const auto signed_word=[](std::uint16_t bits){std::int16_t value;std::memcpy(&value,&bits,2);return value;};
    const auto width=static_cast<long double>(signed_word(text.width())),height=static_cast<long double>(signed_word(text.height()));
    const auto padding=static_cast<std::int32_t>(std::trunc(width*.0009765625f*text.text.inset));
    const auto store=[&](unsigned index,float value){std::memcpy(&text.geometry8[index],&value,4);};
    const auto load=[&](unsigned index){float value;std::memcpy(&value,&text.geometry8[index],4);return value;};
    store(0,static_cast<float>(width*bounds[0]+padding));
    store(1,static_cast<float>(height*bounds[1]*-1.33333337306976318359375f-1.f));
    store(2,static_cast<float>(width*bounds[4]-padding));
    store(3,static_cast<float>(1.f-height*bounds[5]*1.33333337306976318359375f));
    store(4,static_cast<float>(static_cast<long double>(load(2))-load(0)));
    store(5,static_cast<float>(static_cast<long double>(load(1))-load(3)));
    store(6,hud_text_depth(frames,scene,visual));
    text.bytes70[2]=text.bytes70[0]=1;text.text.dirty=true;
}
} //namespace hd2::game
#endif
