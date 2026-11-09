#ifndef HD2_GAME_HUD_MATERIAL_LOOKUP_H
#define HD2_GAME_HUD_MATERIAL_LOOKUP_H
#include "frontend/startup_language.h"
#include <stdexcept>
#include <string>
namespace hd2::game {
//004A89E0/004A8A90: reverse actual LOD0 groups; exact ASCII-case-insensitive
//texture name, not substring matching. Returned texture/material are BORROWED.
template<class Frames>
class HudMaterialLookup {
public:
    using Frame=typename Frames::Frame;
    explicit HudMaterialLookup(Frames& frames):frames_(frames) {}
    Frame texture(Frame visual,const char* name) { //ECX=visual, EDX=name; no prepare call.
        for(auto count=group_count(visual);count;) {
            --count;const auto candidate=frames_.first_person_material_texture(required(material(visual,count)));
            if(!candidate)throw std::runtime_error("HUD texture lookup encountered missing texture");
            if(matches(candidate,name))return frames_.first_person_material_texture(required(material(visual,count)));
        }
        return Frame{};
    }
    Frame material_for_replacement(Frame visual,const char* name) { //004A8A90 prepares visual then matching GROUP+18.
        required(visual);frames_.hud_injury_prepare(visual);
        for(auto count=group_count(visual);count;) {
            --count;const auto candidate=material(visual,count);
            if(!candidate||!frames_.first_person_material_texture(candidate))continue;
            const auto texture=frames_.first_person_material_texture(required(material(visual,count)));
            if(!texture)throw std::runtime_error("HUD replacement texture disappeared");
            if(matches(texture,name)) {
                frames_.hud_material_group18(lod(visual),count);
                return material(visual,count); //Repeat LOD/group read AFTER group18 may replace material.
            }
        }
        return Frame{};
    }
private:
    static Frame required(Frame value){if(!value)throw std::runtime_error("Required HUD material artwork is missing");return value;}
    Frame lod(Frame visual){return required(frames_.first_person_material_lod(required(visual)));}
    std::uint32_t group_count(Frame visual){const auto count=frames_.first_person_material_count(lod(visual));if(count>0x100000u)throw std::runtime_error("HUD material group count exceeds bounds");return count;}
    Frame material(Frame visual,std::uint32_t index){return frames_.hud_group_material(lod(visual),index);}
    bool matches(Frame texture,const char* name){if(!name)throw std::invalid_argument("HUD material lookup has no name");return frontend::startup_case_equal(frames_.first_person_texture_name(texture),name);}
    Frames& frames_;
};
} //namespace hd2::game
#endif
