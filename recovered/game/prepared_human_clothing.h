#ifndef HD2_GAME_PREPARED_HUMAN_CLOTHING_H
#define HD2_GAME_PREPARED_HUMAN_CLOTHING_H

#include "game/prepared_human_spawn.h"
#include "game/session_time_word.h"

namespace hd2::game {

//00489FD0: reverse child recursion, kind7 native120/128 links copied through
//a temporary pointer vector, then added to matching short-name frame+74.
//These are borrowed engine pointers, not script contexts or owned references.
template<class Frames>
void copy_clothing_frame_links(Frames& frames,typename Frames::Frame source,
                               typename Frames::Frame target,unsigned depth=0) {
    if(depth>1024)throw std::runtime_error("Clothing hierarchy exceeds native traversal bounds");
    for(auto n=frames.actor_child_count(source);n;) {
        const auto child=frames.actor_child(source,--n);
        if(frames.frame_kind(child)==7) {
            const auto links=frames.clothing_frame_links(child);
            std::vector<typename Frames::Frame> snapshot;
            for(auto index=links.size();index;)snapshot.push_back(links[--index]);
            const auto name=frames.car_short_name(child);
            const auto match=frames.actor_find(target,name.c_str(),0xffff);
            for(auto index=snapshot.size();index;) {
                if(!match)throw std::runtime_error("Clothing model lacks required matching kind7 frame");
                frames.add_clothing_frame_link(match,snapshot[--index]);
            }
        }
        copy_clothing_frame_links(frames,child,target,depth+1);
    }
}

//0048A170: recurse only into children owned by the ORIGINAL model100. Move
//foreign children after the reverse scan, in reverse temporary-vector order.
//Preserve LOCAL position/quaternion with LinkTo flag0; absent short name uses
//target root. The same source model owner is passed through all recursion.
template<class Frames>
void move_clothing_foreign_children(Frames& frames,typename Frames::Frame source,
                                    typename Frames::Frame target,typename Frames::Frame owner,unsigned depth=0) {
    if(depth>1024)throw std::runtime_error("Clothing attachment hierarchy exceeds traversal bounds");
    std::vector<typename Frames::Frame> foreign;
    for(auto n=frames.actor_child_count(source);n;) {
        const auto child=frames.actor_child(source,--n);
        const auto model=frames.frame_model_owner(child);
        if(model&&model==owner)move_clothing_foreign_children(frames,child,target,owner,depth+1);
        else foreign.push_back(child);
    }
    for(auto n=foreign.size();n;) {
        const auto child=foreign[--n],parent=frames.object_parent(child);
        if(!parent)throw std::runtime_error("Clothing attachment lost original parent");
        const auto name=frames.car_short_name(parent);
        auto match=frames.actor_find(target,name.c_str(),0xffff);if(!match)match=target;
        const auto position=frames.car_position(child);const auto rotation=frames.car_rotation(child);
        frames.link_scene_object(child,match);
        frames.object_position(child,{{position[0],position[1],position[2]}});frames.object_rotation(child,rotation);
        frames.update_scene_object(child);
    }
}

//0048A780's cold prepared-Human path. Retains the SAME original model identity
//and transfers animation lanes/links/foreign children in both directions. The
//live DD0 and already-rostered camera refresh branches require their containing
//receivers and are rejected before this policy can mutate such a Human.
template<class Frames,class Models,class Effects,class Random>
bool replace_prepared_human_model(Frames& frames,Models& models,Effects& effects,Random& random,
                                  typename Frames::Frame model,const frontend::PreMissionItemCatalogue& catalogue,
                                  std::int32_t clothing) {
    auto temporary=frames.create_prepared_human_model();
    if(!temporary)throw std::runtime_error("Cannot create original clothing transfer model");
    std::vector<typename Frames::Frame> groups;
    const auto material=effects.human_eye_material(model,groups);
    if(!material)return false;
    const auto eye=frames.clothing_eye_material_name(material);if(!eye)return false;
    const auto key=prepared_human_model_key(catalogue,clothing);
    if(models.load_model(temporary.get(),key,0)!=0)return false;
    struct Unlink {
        Frames& frames;typename Frames::Frame model;bool linked=false;
        ~Unlink(){if(linked)try{frames.link_scene_object(model,{});}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
    } unlink{frames,temporary.get()};
    frames.link_scene_object(temporary.get(),frames.object_parent(model));unlink.linked=true;
    const auto original=frames.car_position(model);
    //00475190(6) uses rand*6/32768. Z offset is stored FLOAT first. Actual
    //0080F9F8 is4, giving[-4,2); it is not the geometric half-range3.
    const float z=static_cast<float>(static_cast<long double>(random())*6.f*0x1p-15f-4.f);
    const auto x=static_cast<long double>(random())*6.f*0x1p-15f-4.f;
    frames.object_position(temporary.get(),{{static_cast<float>(x+original[0]),original[1],
        static_cast<float>(static_cast<long double>(z)+original[2])}});
    frames.object_rotation(temporary.get(),frames.car_rotation(model));
    frames.game_camera_name(temporary.get(),frames.frame_name(model).c_str());frames.update_scene_object(temporary.get());
    for(unsigned n=8;n;) {
        --n;const auto clip=frames.human_model_clip(model,n);const bool enabled=frames.human_model_animation_enabled(model,n);
        if(!clip)continue;
        frames.human_model_animation(temporary.get(),clip,n,frames.human_model_weight(model,n),0);
        frames.human_model_animation_flags(temporary.get(),n,frames.human_model_animation_flags(model,n));
        frames.human_model_time(temporary.get(),n,frames.human_model_time(model,n));
        if(!enabled){frames.human_model_remove_animation(temporary.get(),n);frames.human_model_weight(temporary.get(),n,0);}
    }
    copy_clothing_frame_links(frames,model,temporary.get());
    move_clothing_foreign_children(frames,model,temporary.get(),model);frames.update_scene_object(temporary.get());
    (void)models.load_model(model,key,0); //Actual004290A0 result ignored by0048A780.
    for(unsigned n=8;n;) {
        --n;if(!frames.human_model_clip(temporary.get(),n))continue;
        const auto weight=frames.human_model_weight(temporary.get(),n);
        const auto clip=frames.human_model_clip(temporary.get(),n);
        frames.human_model_animation(model,clip,n,weight,0);
        frames.human_model_time(model,n,frames.human_model_time(temporary.get(),n));
        frames.human_model_animation_flags(model,n,frames.human_model_animation_flags(temporary.get(),n));
        if(!frames.human_model_animation_enabled(temporary.get(),n)) {
            frames.human_model_remove_animation(model,n);frames.human_model_weight(model,n,0);
        }
    }
    copy_clothing_frame_links(frames,temporary.get(),model);
    move_clothing_foreign_children(frames,temporary.get(),model,temporary.get());
    frames.link_scene_object(temporary.get(),{});unlink.linked=false;temporary.reset();
    frames.update_scene_object(model); //Native then two DD0 calls, both absent for the guarded cold Human.
    frames.update_scene_object(model);
    return effects.human_eye_texture(model,*eye);
}

template<class Frames,class Models,class Items,class Effects,class Random,class Dubbing,class Control>
class PreparedHumanClothing {
public:
    PreparedHumanClothing(Frames& frames,Models& models,Items& items,Effects& effects,Random& random,
                          Dubbing& dubbing,Control& control,const frontend::PreMissionItemCatalogue& catalogue,SessionTimeWord game_time)
        :frames_(frames),models_(models),items_(items),effects_(effects),random_(random),dubbing_(dubbing),control_(control),catalogue_(catalogue),game_time_(game_time){}
    template<class Human>bool apply_prepared_human_clothing(Human& human,std::int32_t identity) { //005A6930.
        auto* item=find_human_inventory_item(human,items_,identity);if(!item)return false;
        const auto model=human.base().frame();
        //007E5A40 is strstr(resource34, model134), not strcmp. Empty model
        //resource therefore matches; the original game28==0 still forces work.
        if(game_time_&&item->definition().resources[2].find(frames_.car_model_resource(model))!=std::string::npos)return true;
        if(human.flag90||human.fielda0||human.loading_state||human.field25c!=0xffffffffu||control_.human_roster().contains(&human))
            throw std::runtime_error("Active clothing requires actual DD0/Sight/Players camera refresh receivers");
        dubbing_.retire_model(model);
        (void)replace_prepared_human_model(frames_,models_,effects_,random_,model,catalogue_,static_cast<std::int32_t>(item->definition().identifier));
        human.head_marker=frames_.actor_find(model,"f",0x400); //Original008390EC; not the earlier z marker.
        human.activation.flag178=true;return true; //Original ignores effects failure.
    }
private:
    Frames& frames_;Models& models_;Items& items_;Effects& effects_;Random& random_;Dubbing& dubbing_;Control& control_;
    const frontend::PreMissionItemCatalogue& catalogue_;SessionTimeWord game_time_;
};
} //namespace hd2::game
#endif
