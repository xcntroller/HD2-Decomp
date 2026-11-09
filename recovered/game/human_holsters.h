#ifndef HD2_GAME_HUMAN_HOLSTERS_H
#define HD2_GAME_HUMAN_HOLSTERS_H

#include "game/human_holster_definitions.h"
#include "frontend/cleanup.h"
#include "frontend/pre_mission_data.h"
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace hd2::game {

//00791720 consumes the actual definition virtual+10, class04, identifier08
//and field0C (serialized3C).14 is the original no-holster group.
inline unsigned human_item_holster_group(const frontend::PreMissionItem& definition) {
    const auto id=definition.identifier;
    switch(definition.kind()) {
    case 0:return 1;
    case 2:return 0;
    case 9:return id==61?7:id==63?8:6;
    case 10:case 11:case 21:return id==66||id==251?10:id==67?11:9;
    case 13:return id==53?12:id==54?13:14;
    case 14:return 4;
    default:
        if(!definition.item_class)return definition.field0c==0?2:definition.field0c==1?3:14;
        return id<239||id==251||id==253?5:14;
    }
}

inline bool human_holster_name_equal(const std::string& first,const char* second) {
    std::size_t index=0;
    for(;index<first.size()&&second[index];++index) {
        auto fold=[](unsigned char c){return c>='A'&&c<='Z'?static_cast<unsigned char>(c+32):c;};
        if(fold(static_cast<unsigned char>(first[index]))!=fold(static_cast<unsigned char>(second[index])))return false;
    }
    return index==first.size()&&!second[index]; //007E88F0's ASCII comparison.
}

//0078CE20/0078CDB0: the230 equipment owner's complete holster-loading
//dependency. Frames borrow the SAME model cache; Animations owns0098ADB0's
//actual tecko reference in the SAME animation cache. No equipment/item producers
//or activation are supplied by this owner. Clear before releasing Human model.
template<class Frames,class Animations>
class HumanHolsters {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Group {std::uint32_t field0=0;unsigned count=0;std::array<std::optional<Owner>,4> frames{};};
    HumanHolsters(Frames& frames,Animations& animations):frames_(frames),animations_(animations) {}
    ~HumanHolsters(){clear();}
    HumanHolsters(const HumanHolsters&)=delete;
    HumanHolsters& operator=(const HumanHolsters&)=delete;
    void initialize(Frame model) {
        if(started_)throw std::logic_error("Human holsters initialized twice");
        if(!model)throw std::runtime_error("Human holsters require an actual model");
        started_=true;
        for(auto& group:groups_)group.field0=0; //0078CE20 zeroes all fourteen records.
        try {
            const Snapshot saved{frames_.human_model_clip(model,7),integer_weight(frames_.human_model_weight(model,7)),
                frames_.human_model_time(model,7),frames_.human_model_animation_flags(model,7),
                frames_.human_model_animation_enabled(model,7)};
            const auto c=frames_.actor_find(model,"c",0xffff),d=frames_.actor_find(model,"d",0xffff),o=frames_.actor_find(model,"o",0xffff);
            const auto pose=animations_.holster_pose();pose_accessed_=true;
            struct Restore {
                HumanHolsters& owner;Frame model;const Snapshot& saved;bool armed=true;
                ~Restore(){if(armed)try{owner.restore(model,saved);}catch(...){owner.frames_.record_actor_cleanup_failure(std::current_exception());}}
                void finish(){armed=false;owner.restore(model,saved);}
            } restore{*this,model,saved};
            frames_.human_model_animation(model,pose,7,1.f,2);
            frames_.human_model_advance(model,pose_times());
            auto name=frames_.car_model_resource(model);
            if(const auto slash=name.find_last_of('\\');slash!=std::string::npos)name.erase(0,slash+1);
            if(const auto dot=name.find('.');dot!=std::string::npos)name.resize(dot); //007E5B30=strchr, first dot.
            const HumanHolsterDefinition* definition=nullptr;
            for(const auto& entry:human_holster_definitions)if(human_holster_name_equal(name,entry.name)){definition=&entry;break;}
            if(!definition)throw std::runtime_error("Human model is absent from original holster definitions: "+name);
            if(definition->set>=0)load_parts(model,*definition,c,d,o);
            restore.finish();
        }catch(...) {clear();throw;}
    }
    void clear() noexcept { //14 groups ascending, all4 slots regardless of count.
        if(!started_)return;
        for(auto& group:groups_) {
            group.count=0;
            for(auto& owner:group.frames)if(owner) {
                try {frames_.link_scene_object(owner->get(),Frame{});}
                catch(...) {frames_.record_actor_cleanup_failure(std::current_exception());}
                owner.reset();
            }
        }
        if(pose_accessed_)animations_.release_holster_pose();
        pose_accessed_=false;started_=false;
    }
    const auto& groups() const{return groups_;}
    void show_items(const frontend::PreMissionItem& definition,std::int32_t amount,float opacity) { //007919C0.
        if(!started_)throw std::logic_error("Human holsters are not initialized");
        const auto index=human_item_holster_group(definition);if(index==14)return;
        auto& group=groups_[index];
        const auto previous=frontend::pre_mission_detail::Bytes::integer(group.field0);
        if(previous<0)throw std::runtime_error("Negative Human holster index");
        const auto divisor=index==2||index==3?3:1;
        const auto end=static_cast<std::int64_t>(previous)+amount;
        //Native still loops after every model is visible, but makes no more
        //calls; bounded traversal preserves the repeated calls for groups2/3.
        for(auto item=static_cast<std::int64_t>(previous);item<end&&item/divisor<group.count;++item) {
            auto& owner=group.frames[static_cast<std::size_t>(item/divisor)];
            if(!owner)throw std::runtime_error("Missing Human holster frame allocation");
            const auto frame=owner->get();frames_.object_visible(frame,true);
            if(frames_.visual_type(frame)==0x5f4a424fu)frames_.human_equipment_opacity(frame,opacity>1?1.f:opacity); //00789C80.
        }
        group.field0+=static_cast<std::uint32_t>(amount);
    }
    void hide_items(const frontend::PreMissionItem& definition,std::int32_t amount) { //00791910.
        if(!started_)throw std::logic_error("Human holsters are not initialized");
        const auto index=human_item_holster_group(definition);if(index==14)return;
        auto& group=groups_[index];group.field0-=static_cast<std::uint32_t>(amount);
        const auto remaining=frontend::pre_mission_detail::Bytes::integer(group.field0);
        if(remaining<0){group.field0=0;return;} //Native underflow returns before changing visibility.
        const auto visible=(index==2||index==3)&&remaining?(remaining-1)/3+1:remaining;
        for(unsigned n=0;n<group.count;++n) {
            if(!group.frames[n])throw std::runtime_error("Missing Human holster frame allocation");
            frames_.object_visible(group.frames[n]->get(),static_cast<std::int32_t>(n)<visible);
        }
    }
private:
    struct Snapshot {Frame clip;std::int32_t weight,time;std::uint32_t flags;bool enabled;};
    static std::int32_t integer_weight(float weight) {
        if(!std::isfinite(weight)||weight<std::numeric_limits<std::int32_t>::min()||
            static_cast<long double>(weight)>std::numeric_limits<std::int32_t>::max())
            throw std::runtime_error("Human saved animation weight exceeds original integer range");
        return static_cast<std::int32_t>(weight); //Native __ftol before restoring as FLOAT(integer).
    }
    static std::array<std::int32_t,8> pose_times(){return {{0,0,0,0,0,0,0,100}};}
    void restore(Frame model,const Snapshot& saved) {
        frontend::CleanupErrors errors;
        errors.attempt([&]{frames_.human_model_weight(model,7,0.f);});
        errors.attempt([&]{frames_.human_model_advance(model,pose_times());});
        errors.attempt([&]{frames_.human_model_remove_animation(model,7);});
        if(saved.enabled) {
            errors.attempt([&]{frames_.human_model_animation(model,saved.clip,7,static_cast<float>(saved.weight),saved.flags);});
            errors.attempt([&]{frames_.human_model_time(model,7,saved.time);});
        }
        errors.rethrow();
    }
    Frame attachment(const HumanHolsterDefinition& definition,const HumanHolsterMarker& marker,Frame c,Frame d,Frame o) const {
        if(marker.group==4)return d;
        if(human_holster_name_equal(definition.name,"EN06")) {
            if(human_holster_name_equal(marker.name,"1"))return o;
            if(human_holster_name_equal(marker.name,"83")||human_holster_name_equal(marker.name,"51"))return d;
            return c;
        }
        if(human_holster_name_equal(definition.name,"US03")) {
            if(human_holster_name_equal(marker.name,"81")||human_holster_name_equal(marker.name,"82")||human_holster_name_equal(marker.name,"83"))return d;
            return c;
        }
        if(human_holster_name_equal(marker.name,"83")||
            (human_holster_name_equal(marker.name,"42")&&!definition.flag))return d;
        return c;
    }
    void load_parts(Frame model,const HumanHolsterDefinition& definition,Frame c,Frame d,Frame o) {
        if(static_cast<std::size_t>(definition.set)>=human_holster_sets.size())throw std::runtime_error("Invalid original Human holster set");
        auto set=frames_.create_human_holster_model();
        if(!set)throw std::runtime_error("Cannot create required Human holster model");
        if(frames_.load_human_holster_model(set.get(),human_holster_sets[static_cast<std::size_t>(definition.set)])<0)
            throw std::runtime_error("Cannot load required Human holster set");
        struct Unlink {
            Frames& frames;Frame model;bool armed=true;
            ~Unlink(){if(armed)try{frames.link_scene_object(model,Frame{});}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
            void finish(){armed=false;frames.link_scene_object(model,Frame{});}
        } unlink{frames_,set.get()};
        frames_.link_scene_object(set.get(),model);
        for(const auto& marker:human_holster_markers) {
            auto frame=frames_.actor_find(set.get(),marker.name,0xffff);if(!frame)continue;
            auto& group=groups_[marker.group];if(group.count==4)continue;
            auto owner=frames_.retain_actor_frame(frame);
            group.frames[group.count++].emplace(std::move(owner));
            frames_.remove_human_holster_frame(set.get(),frame);
            frames_.actor_link_preserving_world(frame,attachment(definition,marker,c,d,o));
            frames_.object_visible(frame,false);
        }
        unlink.finish(); //Then the real temporary model releases, after its extracted parts.
    }
    Frames& frames_;Animations& animations_;std::array<Group,14> groups_{};
    bool started_=false,pose_accessed_=false;
};
} //namespace hd2::game
#endif
