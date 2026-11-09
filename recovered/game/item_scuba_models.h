#ifndef HD2_GAME_ITEM_SCUBA_MODELS_H
#define HD2_GAME_ITEM_SCUBA_MODELS_H

#include "game/item_model_cache.h"

namespace hd2::game {

//007DF850's separate definition50 cache.007DF430 releases these FOUR root
//references directly; it does not call007DC460 here. The primary30 cache and
//catalogue remain the existing owners. Engine/model input determines the actual
//four source frame trees; this helper does not manufacture missing parts.
template<class Frames>
class ItemScubaModelCache {
public:
    using Frame=typename Frames::Frame;
    using Definition=frontend::PreMissionItem;
    using Catalogue=frontend::PreMissionItemCatalogue;
    struct Release {
        Frames* frames=nullptr;
        void operator()(Frame frame) const noexcept {
            if(frame)try {frames->release_item_frame(frame);}
            catch(...) {frames->record_actor_cleanup_failure(std::current_exception());}
        }
    };
    using Owner=std::unique_ptr<std::remove_pointer_t<Frame>,Release>;
    using Parts=std::array<Owner,4>;
    ItemScubaModelCache(Frames& frames,const Catalogue& catalogue,const std::string& suffix)
        :frames_(frames),catalogue_(catalogue),suffix_(suffix) {}
    ~ItemScubaModelCache(){clear();}
    ItemScubaModelCache(const ItemScubaModelCache&)=delete;
    ItemScubaModelCache& operator=(const ItemScubaModelCache&)=delete;
    const Parts* parts(const Definition& definition) {
        if(definition.identifier>=Catalogue::capacity||catalogue_.item(static_cast<std::int32_t>(definition.identifier))!=&definition)
            throw std::logic_error("Scuba model definition is outside the selected catalogue");
        if(definition.identifier!=231)return nullptr;
        if(cached_)return cached_.get();
        if(definition.resources[2].empty())return nullptr;
        auto model=frames_.create_item_template_model();
        if(!model)throw std::runtime_error("Cannot create required scuba template model");
        if(frames_.load_item_template_model(model.get(),definition.resources[2]+suffix_)!=0)return nullptr;
        auto next=std::make_unique<Parts>();
        constexpr std::array<const char*,4> names{{"mask","finL","finR","tank"}}; //00863978.
        for(unsigned index=0;index!=names.size();++index) {
            const auto source=frames_.actor_find(model.get(),names[index],1);
            if(source) {
                auto copy=ItemFrameTree<Frames>::clone(frames_,source);
                (*next)[index]=Owner(copy.release(),Release{&frames_});
            }
        }
        cached_=std::move(next);return cached_.get();
    }
    void clear() noexcept {
        if(cached_)for(auto& part:*cached_)part.reset(); //007DF430, ascending mask/finL/finR/tank.
        cached_.reset();
    }
private:
    Frames& frames_;const Catalogue& catalogue_;const std::string& suffix_;
    std::unique_ptr<Parts> cached_;
};
} //namespace hd2::game
#endif
