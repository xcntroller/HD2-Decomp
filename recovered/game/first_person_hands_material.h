#ifndef HD2_GAME_FIRST_PERSON_HANDS_MATERIAL_H
#define HD2_GAME_FIRST_PERSON_HANDS_MATERIAL_H

#include <cstdint>
#include <stdexcept>
#include <string>

namespace hd2::game {
//00492610: borrow the last matching source group's ARM/ARS texture and assign
//it to matching view groups. Engine material1C owns the texture assignment;
//this receiver neither clones a material nor takes a separate texture reference.
template<class Frames>
class FirstPersonHandsMaterial {
public:
    using Frame=typename Frames::Frame;
    explicit FirstPersonHandsMaterial(Frames& frames):frames_(frames) {}
    template<class Human>void apply(std::uint32_t mode,const Human* player,Frame model) {
        if(!(mode&2u)||!player)return;
        if(!model)throw std::runtime_error("First-person material transfer has no actual view model");
        const auto source=frames_.actor_find(player->base().frame(),"a",0xffff);
        const auto source_lod=lod(source);
        Frame texture{};
        for(auto count=groups(source_lod);count;) {
            const auto material=frames_.first_person_group_material(source_lod,--count);
            const auto candidate=frames_.first_person_material_texture(material);
            if(matches(candidate)){texture=candidate;break;}
        }
        if(texture) {
            const auto destination=lod(frames_.actor_find(model,"a",0xffff));
            for(auto count=groups(destination);count;) {
                --count;const auto material=frames_.first_person_group_material(destination,count);
                if(matches(frames_.first_person_material_texture(material))) {
                    //Original repeats the group lookup before material1C.
                    frames_.first_person_material_texture(frames_.first_person_group_material(destination,count),texture);
                }
            }
        }
        frames_.update_scene_object(model); //Also when source has no matching material.
    }
private:
    Frame lod(Frame visual) {
        if(!visual)throw std::runtime_error("First-person hands visual a is missing");
        const auto result=frames_.first_person_material_lod(visual);
        if(!result)throw std::runtime_error("First-person hands visual has no actual LOD0");
        return result;
    }
    std::uint32_t groups(Frame mesh) {
        const auto count=frames_.first_person_material_count(mesh);
        if(count>0x100000u)throw std::runtime_error("First-person material group count exceeds bounds");
        return count;
    }
    bool matches(Frame texture) {
        if(!texture)return false;
        const auto name=frames_.first_person_texture_name(texture);
        return name.find("ARM")!=std::string::npos||name.find("ARS")!=std::string::npos;
    }
    Frames& frames_;
};
} //namespace hd2::game
#endif
