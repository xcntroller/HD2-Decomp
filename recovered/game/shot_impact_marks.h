#ifndef HD2_GAME_SHOT_IMPACT_MARKS_H
#define HD2_GAME_SHOT_IMPACT_MARKS_H
#include "game/human_joint_owners.h"
#include "game/human_ai_shot_direction.h"
#include <string>
#include <memory>
#include <optional>
namespace hd2::game {
//0044FBC0's native8-byte projection record. Native80 owns these records;
//there is no per-frame tick.0044E110/0044E4C0 release them before textures.
template<class Frames>class ShotImpactMark {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().create_shot_surface());
    explicit ShotImpactMark(Frames& frames){
        visual_.emplace(frames.create_shot_surface());if(!frame())throw std::runtime_error("Impact mark requires actual projection frame");
        frames.shot_surface_mode(frame(),3);frames.shot_surface_enabled(frame(),true);frames.shot_surface_projection(frame(),true);
    }
    Frame frame()const{return visual_?visual_->get():Frame{};}
    void parent_name(std::string name){parent_name_=std::move(name);}
    const auto& parent_name()const{return parent_name_;}
    void clear(){visual_.reset();parent_name_.reset();} //0044E4C0: frame release BEFORE COW text retirement.
private:std::optional<std::string> parent_name_;std::optional<Owner> visual_;
};
//0044FBC0. Exact maximum equality recycles the oldest mark, BEFORE either
//engine random draw; quality/query/missing texture gates consume no draws.
template<class State,class Frames,class World>void emit_shot_material_mark(State& state,Frames& frames,World& world,
        std::uint32_t material,const HumanVector& point,const HumanVector& direction){
    if(!world.human_surface_quality_enabled())return;
    HumanVector start=point,displacement{};
    for(unsigned a=0;a!=3;++a){
        displacement[a]=static_cast<float>(static_cast<long double>(direction[a])*.5f);
        const auto product=static_cast<long double>(direction[a])*.30000001192092896f;
        start[a]=static_cast<float>(static_cast<long double>(point[a])-(a==2?product:static_cast<float>(product)));
    }
    const auto hit=world.human_geometry_ray(start,displacement,0xffffffffu,0xb90000u,true);if(!hit)return;
    const auto texture=world.shot_material_texture(material);if(!texture)return;
    if(!state.world)throw std::logic_error("Impact mark requires actual world-bound ShotMgr");
    std::unique_ptr<typename State::ImpactMark> mark;
    if(state.render.size()==state.maximum148){
        if(state.render.empty())throw std::runtime_error("Impact mark reached native empty maximum pool");
        mark=std::move(state.render.front());state.render.erase(state.render.begin());
    }else mark=std::make_unique<typename State::ImpactMark>(frames);
    if(!mark||!mark->frame())throw std::logic_error("Impact mark lost actual pooled projection");
    const float size=static_cast<float>((static_cast<long double>(frames.volume_random_unit())+1.f)*.5f);
    auto facing=hit->normal;for(unsigned a=0;a!=3;++a)facing[a]=-facing[a];
    const float roll=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*6.2831854820251465f);
    frames.shot_surface_direction(mark->frame(),facing,roll);frames.shot_surface_scale(mark->frame(),size,size,.10000000149011612f);
    auto position=point;for(unsigned a=0;a!=3;++a)position[a]=static_cast<float>(static_cast<long double>(hit->normal[a])*.05000000074505806f+point[a]);
    frames.object_position(mark->frame(),{{position[0],position[1],position[2]}});
    //Native rereads the material/texture after frame callbacks.
    frames.shot_surface_texture(mark->frame(),world.shot_material_texture(material));
    const auto matrix=frames.actor_world_matrix(mark->frame());
    const HumanVector world_position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const float light=frames.shot_scene_light(world_position);frames.shot_surface_colour(mark->frame(),light);
    frames.shot_surface_link(mark->frame(),frames.shot_material_sector(point));frames.update_scene_object(mark->frame());
    state.render.push_back(std::move(mark));
}
//0044EBD0, the SAME native80 pool as explosion marks. Parent frame filtering
//and material gates precede pool removal and BOTH engine random draws.
template<class State,class Frames,class World>void emit_shot_bullet_mark(State& state,Frames& frames,World& world,
        std::uint32_t material,const HumanVector& point,HumanVector normal,typename Frames::Frame parent){
    if(!world.human_surface_quality_enabled()||!world.shot_bullet_mark_admitted(material))return;
    if(parent){
        const auto kind=frames.shot_frame_native_type(parent);
        if(kind!=0x5f4a424fu&&kind!=0x50414d4cu&&kind!=0x5252494du)return;
        if(frames.shot_impact_visual_flags(parent)&0x3000u)return;
        std::vector<typename Frames::Frame> seen;
        for(auto ancestor=frames.actor_parent(parent);ancestor;ancestor=frames.actor_parent(ancestor)){
            if(std::find(seen.begin(),seen.end(),ancestor)!=seen.end())throw std::runtime_error("Impact mark reached cyclic native parent chain");
            seen.push_back(ancestor);const auto type=frames.shot_frame_native_type(ancestor);
            if(type==0x4d474e53u||type==0x00444853u||type==0x524d4d53u)return;
        }
    }
    if(!world.shot_bullet_mark_texture(material))return;
    if(!state.world)throw std::logic_error("Bullet impact mark requires actual world-bound ShotMgr");
    std::unique_ptr<typename State::ImpactMark> mark;
    if(state.render.size()==state.maximum148){
        if(state.render.empty())throw std::runtime_error("Bullet mark reached native empty maximum pool");
        mark=std::move(state.render.front());state.render.erase(state.render.begin());
    }else mark=std::make_unique<typename State::ImpactMark>(frames);
    if(!mark||!mark->frame())throw std::logic_error("Bullet impact lost actual pooled projection");
    const auto square=(static_cast<long double>(normal[0])*normal[0]+static_cast<long double>(normal[1])*normal[1])+static_cast<long double>(normal[2])*normal[2];
    normalize_human_ai_shot(normal,square);auto facing=normal;for(unsigned a=0;a!=3;++a)facing[a]=-facing[a];
    const float roll=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*3.1415927410125732f);
    frames.shot_surface_direction(mark->frame(),facing,roll);
    const float size=static_cast<float>(static_cast<long double>(frames.volume_random_unit())*.009999999776482582f+.014999999664723873f);
    frames.shot_surface_scale(mark->frame(),size,size,.029999999329447746f);
    auto position=point;for(unsigned a=0;a!=3;++a){const auto offset=static_cast<long double>(normal[a])*.009999999776482582f;
        position[a]=static_cast<float>((a==2?static_cast<float>(offset):offset)+point[a]);}
    frames.object_position(mark->frame(),{{position[0],position[1],position[2]}});
    frames.shot_surface_texture(mark->frame(),world.shot_bullet_mark_texture(material));
    const auto matrix=frames.actor_world_matrix(mark->frame());const HumanVector world_position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const float light=frames.shot_scene_light(world_position,true);
    if(light<.4000000059604645f)frames.shot_surface_colour(mark->frame(),.4000000059604645f);
    else if(!(light>=1.f))frames.shot_surface_colour(mark->frame(),light);
    else frames.shot_surface_white(mark->frame());
    if(parent){frames.shot_surface_link(mark->frame(),parent);mark->parent_name(frames.frame_name(parent));}
    else{mark->parent_name("No Parent");frames.shot_surface_link(mark->frame(),frames.shot_material_sector(point));}
    frames.shot_surface_projection(mark->frame(),true);frames.update_scene_object(mark->frame());state.render.push_back(std::move(mark));
}
} //namespace hd2::game
#endif
