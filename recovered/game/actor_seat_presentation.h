#ifndef HD2_GAME_ACTOR_SEAT_PRESENTATION_H
#define HD2_GAME_ACTOR_SEAT_PRESENTATION_H
#include "game/human_hud_loading.h"
#include "game/text_glyphs.h"
namespace hd2::game {
//Read/present the actual attached Actor's original virtual receivers. The
//containing World owns this facade; every Actor/definition/frame is borrowed
//only for the call. This neither attaches a Human nor advances a weapon.
template<class Actor>class ActorSeatPresentation {
public:
    virtual ~ActorSeatPresentation()=default;
    virtual std::uint32_t kind(Actor&)=0; //Checked identity before reading Actor18.
    virtual std::uint32_t flags(Actor&,std::int32_t)=0; //8C.
    virtual bool has_control(Actor&,std::int32_t)=0; //98.
    virtual const frontend::PreMissionItem* definition(Actor&,std::int32_t)=0; //B4.
    virtual std::uint32_t current(Actor&,std::int32_t)=0; //9C.
    virtual std::uint32_t reserve(Actor&,std::int32_t)=0; //A0.
    virtual void view(Actor&,std::int32_t,bool)=0; //C4.
    virtual std::optional<HumanHudProgress> progress(Actor&,std::int32_t)=0; //C8.
    virtual bool hides_cursor(Actor&,std::int32_t)=0; //CC.
};
//004C7670. Reload simulation publishes E0/E4 on the existing control owner.
//An absent publication is an inactive result, including an attached Human.
template<class Control>std::optional<HumanHudProgress> mounted_weapon_hud_progress(const Control& control){
    if(!control.fielde0)return std::nullopt;
    return HumanHudProgress{human_hud_signed(control.fielde0),human_hud_signed(control.fielde4),0x1b9f};
}
//00604A10 parses the FIRST case-sensitive SEAT substring, with x86 shift masking.
inline std::uint32_t actor_marker_seat_flags(const std::string& text){
    const auto at=text.find("SEAT");if(at==std::string::npos)return 0;
    std::uint32_t result=0;
    for(auto n=at+4;n<text.size()&&text[n]!=','&&text[n]!=' ';++n)
        result|=1u<<((static_cast<unsigned char>(text[n])-0x30u)&31u);
    return result;
}
//004DA2A0/00604E30 use case-sensitive substring matching, not a token parser.
inline bool actor_marker_hides_cursor(const std::string& text){return text.find("NO3RD")!=std::string::npos;}
//004C6D90: flag104 and F4 gates; visibility precedes scene update.
template<class Frames,class Control>void mounted_weapon_present_view(Frames& frames,const Control& control,typename Frames::Frame view,bool enabled){
    if(!control.flag104||!view)return;
    frames.object_visible(view,enabled);frames.update_scene_object(view);
}
template<class Frames,class Actor>void require_actor_seat_identity(Frames& frames,Actor& actor){
    for(std::size_t n=0;n<frames.scene_actor_identity_count();++n)
        if(frames.scene_actor_identity(static_cast<std::int32_t>(n))==&actor)return;
    throw std::logic_error("Seat presentation Actor is absent from SAME World identity registry");
}
//004C57B0 and004C6550 deliberately have different failed138 gates.
template<class Items,class Entry>const frontend::PreMissionItem* mounted_weapon_hud_definition(Items& items,bool failed,Entry entry){
    if(failed)return nullptr;if(!entry)throw std::logic_error("Mounted presentation lost its actual item");
    return &items.runtime_item(entry).definition();
}
template<class Items,class Entry>std::uint32_t mounted_weapon_hud_amount(Items& items,Entry entry){
    if(!entry)throw std::logic_error("Mounted ammunition display requires its actual item");
    return static_cast<std::uint32_t>(text_ftol(items.runtime_item(entry).amount()));
}
//00611140 (absent frozen inventory): the ACTOR's sole array, reverse stacks,
//front entry's definition ID and first matching stack count. Never Human ammo.
template<class Actor,class Items>std::uint32_t actor_seat_ammunition(Actor& actor,Items& items,std::int32_t ammunition){
    auto* storage=actor.base().storage();
    if(!storage||!storage->single()||!storage->slots(0))throw std::logic_error("Seat ammunition requires the actual Actor storage");
    auto& slots=*storage->slots(0);
    for(auto n=slots.occupied_stacks();n;){const auto& stack=slots.slots().at(--n);
        if(stack.entries.empty())continue;
        if(static_cast<std::int32_t>(items.runtime_item(stack.entries.front()).definition().identifier)==ammunition)
            return static_cast<std::uint32_t>(stack.entries.size());
    }
    return 0;
}
} //namespace hd2::game
#endif
