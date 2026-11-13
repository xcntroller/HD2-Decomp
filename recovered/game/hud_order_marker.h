#ifndef HD2_GAME_HUD_ORDER_MARKER_H
#define HD2_GAME_HUD_ORDER_MARKER_H
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {
//004B9240/004B9B10. The actual command panel owns template748 and free750.
//Textures are borrowed from SAME artwork/player-state/action and button icons.
//Fresh and reused materials differ; update never repeats fresh preparation.
template<class Frames,class Order,class First,class Actions,class Icons>
void present_hud_order_marker(Frames& frames,typename Frames::Frame model,Order& order,
        bool creation,bool fresh,const First& first,const Actions& actions,const Icons& icons){
    using Frame=typename Frames::Frame;
    const auto find=[&](const char* name){
        const auto visual=frames.actor_find(model,name,1u);
        if(!visual)throw std::logic_error("Order marker requires actual authored visual");
        return visual;
    };
    const auto group_material=[&](Frame visual){
        const auto material=frames.first_person_group_material(frames.first_person_material_lod(visual),0u);
        if(!material)throw std::logic_error("Order marker requires actual first group material");
        return material;
    };
    const auto texture=[&](Frame visual,Frame value){frames.first_person_material_texture(frames.hud_first_material(visual),value);};
    const auto prepare=[&](Frame visual,bool selector){
        if(!fresh)return;
        if(!selector){frames.hud_injury_prepare(visual);frames.hud_group18(visual);}
        frames.hud_material_flags(group_material(visual),0x08000000u);
        frames.hud_material_vector(group_material(visual),0xcu,{{selector?2.f:1.f,selector?0.f:1.f,selector?0.f:1.f,0.f}});
        frames.hud_material_vector(group_material(visual),0x10u,{{0,0,0,0}});
        frames.hud_material_vector(group_material(visual),0x8u,{{0,0,0,0}});
        frames.hud_material14(group_material(visual),0.f);frames.hud_visual_a4(visual,.60000002384185791015625f);
    };
    const auto defaults=[&](std::uint32_t mask,std::uint32_t value){
        auto flags=order.state.template get<std::uint32_t>(0x44);
        if(!(flags&mask)){flags|=value;order.state.set(0x44,flags);}return flags;
    };
    auto visual=find("3Dcommand");prepare(visual,false);Frame selected{};
    switch(order.state.template get<std::uint32_t>(0xc)){
    case 1:selected=first.at(10);break;case 2:selected=first.at(11);break;
    case 3:selected=actions.at(order.state.template get<std::uint32_t>(0x10));break;
    case 4:selected=first.at(19);break;case 6:selected=first.at(13);break;case 7:selected=first.at(14);break;
    }
    if(selected)texture(visual,selected);
    if(!creation)frames.update_scene_object(visual);
    visual=find("3Dpose");prepare(visual,false);auto flags=defaults(7u,4u);
    selected=icons.at(flags&1u?0:flags&2u?1:2);
    if(creation||selected)texture(visual,selected);
    if(!creation)frames.update_scene_object(visual);
    visual=find("3Dmove");prepare(visual,false);flags=defaults(0x1f000u,0x4000u);
    //Creation keeps the previous texture when ONLY native8000 is selected.
    //Update clears its temporary first and sets a NULL texture in that case.
    if(!creation)selected=Frame{};
    if(flags&0x1000u)selected=icons.at(3);else if(flags&0x2000u)selected=icons.at(4);
    else if(flags&0x4000u)selected=icons.at(5);else if(flags&0x10000u)selected=icons.at(6);
    texture(visual,selected);if(!creation)frames.update_scene_object(visual);
    visual=find("3Dstand");prepare(visual,false);flags=defaults(0x700u,0x200u);
    selected=icons.at(flags&0x400u?7:flags&0x200u?8:9);
    texture(visual,selected);if(!creation)frames.update_scene_object(visual);
    if(creation){visual=find("selector_commands");prepare(visual,true);frames.update_scene_object(visual);}
    frames.object_visible(model,false);frames.update_scene_object(model);
}
}
#endif
