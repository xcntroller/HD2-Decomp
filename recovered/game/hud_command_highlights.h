#ifndef HD2_GAME_HUD_COMMAND_HIGHLIGHTS_H
#define HD2_GAME_HUD_COMMAND_HIGHLIGHTS_H
#include "game/actor_world_corners.h"
#include "game/text_glyphs.h"
#include <cmath>
namespace hd2::game {
enum class HudCommandHighlight {current,previous,target,selected}; //004BB260/004BC300/004BD3D0/004BE670.
namespace hud_command_highlight_detail {
inline std::uint32_t colour_word(float value){
    const float stored=static_cast<float>(static_cast<long double>(value)*255.f);
    const auto rounded=std::nearbyint(static_cast<long double>(stored));
    if(!std::isfinite(rounded)||rounded< -2147483648.L||rounded>2147483647.L)return 0x80000000u;
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded));
}
inline std::uint32_t colour(const std::array<float,3>& rgb){return (((colour_word(.5f)<<8)|colour_word(rgb[0]))<<8|colour_word(rgb[1]))<<8|colour_word(rgb[2]);}
inline auto camera_direction(std::array<float,4> value){
    constexpr float epsilon=9.99999993922529e-09f;
    const auto extended=(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2];
    const float square=static_cast<float>(extended);
    if(!(std::abs(extended-1)<epsilon)){
        if(!(square<epsilon)){const auto inverse=1/std::sqrt(static_cast<long double>(square));for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(value[n]*inverse);}
        else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
    }
    return value;
}
inline bool facing(const std::array<float,16>& model,const std::array<float,4>& origin,const std::array<float,4>& forward){
    constexpr float epsilon=9.99999993922529e-09f;
    auto x=static_cast<long double>(model[12])-origin[0],y=static_cast<long double>(model[13])-origin[1];
    const auto raw_z=static_cast<long double>(model[14])-origin[2];float z=static_cast<float>(raw_z),stored_y=static_cast<float>(y);
    const auto extended=(raw_z*z+y*y)+x*x;const float square=static_cast<float>(extended);
    if(!(std::abs(extended-1)<epsilon)){
        if(!(square<epsilon)){const auto inverse=1/std::sqrt(static_cast<long double>(square));x*=inverse;y*=inverse;z=static_cast<float>(static_cast<long double>(z)*inverse);}
        else if(x!=0){x=x<0?-1.f:1.f;y=stored_y;}
        else if(z!=0)z=z<0?-1.f:1.f;
        else y=y<0?-1.f:1.f;
    }
    return (static_cast<long double>(forward[2])*z+static_cast<long double>(forward[1])*y)+x*forward[0]>=0;
}
struct Rectangle {float left,right,top,bottom,quarter_x,quarter_y;};
inline Rectangle rectangle(const std::array<std::array<float,4>,8>& points,float width,float height,HudCommandHighlight kind){
    float left=width,right=0,top=height,bottom=0;
    for(const auto& p:points){if(right<p[0])right=p[0];if(p[0]<left)left=p[0];if(bottom<p[1])bottom=p[1];if(p[1]<top)top=p[1];}
    const auto minimum_x=static_cast<long double>(width)*.025f;
    const auto minimum_y=static_cast<float>(static_cast<long double>(height)*.025f);
    auto span_x=static_cast<float>(static_cast<long double>(right)-left);auto span_y=static_cast<long double>(bottom)-top;
    if(span_x<minimum_x){const auto pad=(minimum_x-span_x)*.5f;left=static_cast<float>(static_cast<long double>(left)-pad);right=static_cast<float>(pad+right);span_x=static_cast<float>(minimum_x);}
    if(span_y<minimum_y){const auto pad=(minimum_y-span_y)*.5f;top=static_cast<float>(static_cast<long double>(top)-pad);bottom=static_cast<float>(pad+bottom);span_y=minimum_y;}
    const float qx=static_cast<float>(static_cast<long double>(span_x)*.25f),qy=static_cast<float>(span_y*.25f);
    //004BC300 insets;004BD3D0 outsets AFTER minimum size and quarter lengths.
    //These command rectangles deliberately lack0049E260's ten-pixel clamp.
    if(kind==HudCommandHighlight::previous||kind==HudCommandHighlight::target){
        const auto x=static_cast<long double>(width)*.005f;const float y=static_cast<float>(static_cast<long double>(height)*.005f);
        const auto sign=kind==HudCommandHighlight::previous?1.L:-1.L;
        left=static_cast<float>(left+sign*x);right=static_cast<float>(right-sign*x);
        top=static_cast<float>(static_cast<long double>(top)+sign*y);bottom=static_cast<float>(static_cast<long double>(bottom)-sign*y);
    }
    return {left,right,top,bottom,qx,qy};
}
template<std::size_t N>auto vertices(const std::array<std::array<float,2>,N>& points,std::uint32_t colour){
    std::array<TextGlyphVertex,N> output{};
    for(std::size_t n=0;n<N;++n)output[n]={points[n][0],points[n][1],.5f,2.f,colour,0,0,0};
    return output;
}
} //namespace hud_command_highlight_detail
template<class Frames,class Factory,class Actor>void draw_hud_command_highlight(Frames& frames,Factory& factory,Actor& actor,HudCommandHighlight kind,const std::array<float,3>& rgb){
    const auto model=actor.frame();if(!model)throw std::logic_error("Command highlight requires actual actor model");
    //Native11C is last-render epoch, NOT scene membership or an actor ID.
    if(frames.vertex_frame_epoch(model)!=frames.vertex_scene_epoch())return;
    auto camera=frames.active_game_camera();if(!camera)throw std::logic_error("Command highlight requires SAME active world camera");
    const auto first=frames.actor_world_matrix(camera);const std::array<float,4> origin{{first[12],first[13],first[14],first[15]}};
    camera=frames.active_game_camera();if(!camera)throw std::logic_error("Command highlight camera retired between original matrix queries");
    const auto second=frames.actor_world_matrix(camera);
    const auto forward=hud_command_highlight_detail::camera_direction({{second[8],second[9],second[10],second[11]}});
    if(!hud_command_highlight_detail::facing(frames.actor_world_matrix(model),origin,forward))return;
    const auto corners=actor_world_corners(frames,actor);const auto projected=frames.hud_project_world_corners(corners);
    const auto width=static_cast<float>(factory.hud_display_width()),height=static_cast<float>(factory.hud_display_height());
    const auto r=hud_command_highlight_detail::rectangle(projected,width,height,kind);const auto colour=hud_command_highlight_detail::colour(rgb);
    using Point=std::array<float,2>;
    if(kind==HudCommandHighlight::selected){
        const std::array<Point,8> points{{{{r.left,r.top}},{{r.right,r.top}},{{r.right,r.top}},{{r.right,r.bottom}},{{r.right,r.bottom}},{{r.left,r.bottom}},{{r.left,r.bottom}},{{r.left,r.top}}}};
        factory.hud_draw_command_highlight(hud_command_highlight_detail::vertices(points,colour));
    }else{
        const float il=static_cast<float>(static_cast<long double>(r.left)+r.quarter_x),ir=static_cast<float>(static_cast<long double>(r.right)-r.quarter_x);
        const float it=static_cast<float>(static_cast<long double>(r.top)+r.quarter_y),ib=static_cast<float>(static_cast<long double>(r.bottom)-r.quarter_y);
        const std::array<Point,16> points{{{{r.left,r.top}},{{r.left,it}},{{r.left,r.top}},{{il,r.top}},{{r.right,r.top}},{{r.right,it}},{{r.right,r.top}},{{ir,r.top}},{{r.right,r.bottom}},{{r.right,ib}},{{ir,r.bottom}},{{r.right,r.bottom}},{{r.left,r.bottom}},{{r.left,ib}},{{r.left,r.bottom}},{{il,r.bottom}}}};
        factory.hud_draw_command_highlight(hud_command_highlight_detail::vertices(points,colour));
    }
}
} //namespace hd2::game
#endif
