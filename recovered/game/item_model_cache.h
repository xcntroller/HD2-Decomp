#ifndef HD2_GAME_ITEM_MODEL_CACHE_H
#define HD2_GAME_ITEM_MODEL_CACHE_H

#include "frontend/pre_mission_data.h"
#include <exception>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace hd2::game {

//007DC3A0/007DC460: linking is a hierarchy operation, NOT a reference transfer
//(engine1001D680). Each recursively created frame keeps its factory reference.
//Cleanup unlinks children in reverse and skips the original special families.
//Those detached PROJ/EMTR/sound/light references retain their original engine
//lifetime; this helper neither fabricates an effect consumer nor releases them.
template<class Frames>
class ItemFrameTree {
public:
    using Frame=typename Frames::Frame;
    struct Release {
        Frames* frames=nullptr;
        void operator()(Frame frame) const noexcept {
            if(!frame)return;
            try{destroy(*frames,frame,0);}
            catch(...){frames->record_actor_cleanup_failure(std::current_exception());}
        }
    };
    using Owner=std::unique_ptr<std::remove_pointer_t<Frame>,Release>;
    static Owner clone(Frames& frames,Frame source) {return clone(frames,source,Frame{},0);}
private:
    static void destroy(Frames& frames,Frame frame,unsigned depth) {
        if(!frame||frames.item_frame_references(frame)<=0)return; //Original signed short predicate.
        if(depth>=1024)throw std::runtime_error("Item frame cleanup exceeds hierarchy bounds");
        for(auto count=frames.actor_child_count(frame);count;count=frames.actor_child_count(frame)) {
            if(count>0x100000u)throw std::runtime_error("Item frame child count exceeds bounds");
            const auto child=frames.actor_child(frame,count-1);
            if(!child)throw std::runtime_error("Item frame cleanup has a null child");
            destroy(frames,child,depth+1);
            if(frames.actor_child_count(frame)>=count)
                throw std::runtime_error("Item frame cleanup did not detach its child");
        }
        frames.link_scene_object(frame,Frame{});
        if(frames.visual_type(frame)==0x4a4f5250u)return; //PROJ; second query occurs only otherwise.
        if(frames.visual_type(frame)==0x52544d45u)return; //EMTR.
        const auto kind=frames.frame_kind(frame);
        if(kind!=4&&kind!=2)frames.release_item_frame(frame);
    }
    static Owner clone(Frames& frames,Frame source,Frame parent,unsigned depth) {
        if(!source)return Owner(nullptr,Release{&frames});
        if(depth>=1024)throw std::runtime_error("Item frame clone exceeds hierarchy bounds");
        Owner owner(frames.create_item_frame(source),Release{&frames});
        if(!owner)return owner;
        frames.copy_item_frame(owner.get(),source);
        if(parent)frames.link_scene_object(owner.get(),parent);
        const auto count=frames.actor_child_count(source);
        if(count>0x100000u)throw std::runtime_error("Item frame clone exceeds child bounds");
        for(std::uint32_t i=0;i!=count;++i) {
            const auto source_child=frames.actor_child(source,i);
            auto child=clone(frames,source_child,owner.get(),depth+1);
            if(!child)throw std::runtime_error("Cannot clone an item frame child");
            //The parent does not retain this reference. Its matching recursive
            //deleter will release it explicitly, after unlinking its descendants.
            child.release();
        }
        return owner;
    }
};

//007DF750's primary definition cache(native30/resource34) and007D3AA0's
//per-instance clone. Definitions are the SAME selected preparation catalogue;
//clear before replacing/destroying it. Frames/engine/resources outlive all owners.
//Suffix is the containing runtime's actual0084E8E4 value (initially empty), not
//an invented extension rule. The first-person cache uses native20/resource24 and a separately retained
//class1 magazine at64. Neither cache substitutes for a live item/view owner.
template<class Frames>
class ItemPrimaryModelCache {
public:
    using Definition=frontend::PreMissionItem;
    using Catalogue=frontend::PreMissionItemCatalogue;
    using Frame=typename Frames::Frame;
    using ItemModelOwner=typename ItemFrameTree<Frames>::Owner;
    ItemPrimaryModelCache(Frames& frames,const Catalogue& catalogue,const std::string& suffix)
        : frames_(frames),catalogue_(catalogue),suffix_(suffix) {}
    ~ItemPrimaryModelCache() {clear();}
    ItemPrimaryModelCache(const ItemPrimaryModelCache&)=delete;
    ItemPrimaryModelCache& operator=(const ItemPrimaryModelCache&)=delete;
    Frame primary(const Definition& definition) {
        validate(definition);
        const auto id=static_cast<std::size_t>(definition.identifier);
        if(primary_[id])return primary_[id]->get();
        const auto& name=definition.resources[2]; //007DFCF0: serialized+28→native34.
        if(name.empty())return Frame{};
        auto model=frames_.create_item_template_model();
        if(!model.get())throw std::runtime_error("Item template model factory returned null");
        //007DF750 passes this resource key directly to004290A0; unlike Scene2
        //filenames, it does NOT strip a suffix at the last dot.
        if(frames_.load_item_template_model(model.get(),name+suffix_)!=0)return Frame{};
        const auto source=frames_.actor_find(model.get(),name.c_str(),1);
        if(!source)return Frame{};
        auto copy=ItemFrameTree<Frames>::clone(frames_,source);
        if(!copy)return Frame{};
        primary_[id].emplace(std::move(copy));return primary_[id]->get();
    }
    ItemModelOwner make_item_model(const Definition& definition) {
        return ItemFrameTree<Frames>::clone(frames_,primary(definition));
    }
    //007DF510: unlike the primary cache, find literal fpv_weapon, then retain
    //the original class1 magazine before recursively cloning the weapon tree.
    Frame first_person(const Definition& definition) {
        validate(definition);const auto id=static_cast<std::size_t>(definition.identifier);
        if(first_person_[id])return first_person_[id]->get();
        const auto& name=definition.resources[0]; //007DFCF0 serialized+00→native24.
        if(name.empty())return Frame{};
        auto model=frames_.create_item_template_model();
        if(!model)throw std::runtime_error("First-person item model factory returned null");
        if(frames_.load_item_template_model(model.get(),name+suffix_)!=0)return Frame{};
        const auto source=frames_.actor_find(model.get(),"fpv_weapon",1);
        if(definition.item_class==1)if(const auto magazine=frames_.actor_find(model.get(),"magazine",0x21)) {
            //A repeated failed clone can revisit this native slot. Replace its
            //owned reference safely instead of native overwrite/leak behavior.
            auto retained=frames_.retain_actor_frame(magazine);
            magazines_[id].emplace(std::move(retained));
        }
        if(!source)return Frame{};
        auto copy=ItemFrameTree<Frames>::clone(frames_,source);
        if(!copy)return Frame{};
        first_person_[id].emplace(std::move(copy));return first_person_[id]->get();
    }
    Frame first_person_magazine(const Definition& definition) const {
        validate(definition);const auto& owner=magazines_[definition.identifier];
        return owner?owner->get():Frame{};
    }
    void clear_definition(std::size_t index) noexcept {
        //007E0560 releases derived64 before007DF430 destroys20, then30.
        clear_first_person_definition(index);clear_primary_definition(index);
    }
    void clear_first_person_definition(std::size_t index)noexcept{magazines_[index].reset();first_person_[index].reset();}
    void clear_primary_definition(std::size_t index)noexcept{primary_[index].reset();}
    void clear() noexcept {for(auto i=primary_.size();i!=0;)clear_definition(--i);}
private:
    void validate(const Definition& definition) const {
        if(definition.identifier>=Catalogue::capacity||
           catalogue_.item(static_cast<std::int32_t>(definition.identifier))!=&definition)
            throw std::logic_error("Item model definition is outside the selected catalogue");
    }
    Frames& frames_;
    const Catalogue& catalogue_;
    const std::string& suffix_;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    std::array<std::optional<ItemModelOwner>,Catalogue::capacity> primary_{},first_person_{};
    std::array<std::optional<FrameOwner>,Catalogue::capacity> magazines_{};
};

} //namespace hd2::game
#endif
