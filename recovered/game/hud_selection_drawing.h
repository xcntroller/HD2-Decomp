#ifndef HD2_GAME_HUD_SELECTION_DRAWING_H
#define HD2_GAME_HUD_SELECTION_DRAWING_H
#include "game/actor_world_corners.h"
#include "game/text_glyphs.h"
namespace hd2::game {
//0049E260 geometry AFTER actual Scene98 projection of eight virtualDC corners.
inline auto hud_selection_vertices(const std::array<std::array<float,4>,8>& projected,float width,float height){
    float left=width,right=0,top=height,bottom=0;
    for(const auto& point:projected){
        if(right<point[0])right=point[0];
        if(point[0]<left)left=point[0];
        if(bottom<point[1])bottom=point[1];
        if(point[1]<top)top=point[1];
    }
    if(left<10.f)left=10.f;
    if(static_cast<long double>(width)-10.f<right)right=static_cast<float>(static_cast<long double>(width)-10.f);
    if(top<10.f)top=10.f;
    if(static_cast<long double>(height)-10.f<bottom)bottom=static_cast<float>(static_cast<long double>(height)-10.f);
    auto span_x=static_cast<float>(static_cast<long double>(right)-left);
    auto span_y=static_cast<long double>(bottom)-top;
    const auto minimum_x=static_cast<long double>(width)*.025f;
    const auto minimum_y=static_cast<float>(static_cast<long double>(height)*.025f);
    if(span_x<minimum_x){
        const auto delta=(minimum_x-span_x)*.5f;
        left=static_cast<float>(static_cast<long double>(left)-delta);right=static_cast<float>(delta+right);span_x=static_cast<float>(minimum_x);
    }
    if(span_y<minimum_y){
        const auto delta=(minimum_y-span_y)*.5f;
        top=static_cast<float>(static_cast<long double>(top)-delta);bottom=static_cast<float>(delta+bottom);span_y=minimum_y;
    }
    const auto inner_left=static_cast<float>(static_cast<long double>(span_x)*.25f+left);
    const auto inner_right=static_cast<float>(static_cast<long double>(right)-static_cast<long double>(span_x)*.25f);
    const auto inner_top=static_cast<float>(span_y*.25f+top);
    const auto inner_bottom=static_cast<float>(static_cast<long double>(bottom)-span_y*.25f);
    const std::array<std::array<float,2>,16> positions{{
        {{left,top}},{{left,inner_top}},{{left,top}},{{inner_left,top}},
        {{right,top}},{{right,inner_top}},{{right,top}},{{inner_right,top}},
        {{right,bottom}},{{right,inner_bottom}},{{inner_right,bottom}},{{right,bottom}},
        {{left,bottom}},{{left,inner_bottom}},{{left,bottom}},{{inner_left,bottom}}
    }};
    std::array<TextGlyphVertex,16> result{};
    for(std::size_t n=0;n!=result.size();++n)result[n]={positions[n][0],positions[n][1],.5f,2.f,0x80ffffffu,0,0,0};
    return result;
}
template<class Frames,class Factory,class Actor,class Dialogs,class Command>
void draw_hud_selection(Frames& frames,Factory& factory,const std::array<Actor*,2>& actors,Dialogs& dialogs,Command& command){
    //6B8F8 takes priority only with active signals; otherwise use6B8FC.
    auto* actor=actors[0]&&dialogs.signals_active()?actors[0]:actors[1];
    if(!actor||command.visible())return;
    const auto world=actor_world_corners(frames,*actor);
    const auto projected=frames.hud_project_world_corners(world); //Actual WORLD Scene+98, not HUD scene/camera+98.
    const auto width=static_cast<float>(factory.hud_display_width());
    const auto height=static_cast<float>(factory.hud_display_height());
    factory.hud_draw_selection_vertices(hud_selection_vertices(projected,width,height));
}
} //namespace hd2::game
#endif
