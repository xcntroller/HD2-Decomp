#ifndef HD2_GAME_RUNTIME_ITEM_SERVICES_H
#define HD2_GAME_RUNTIME_ITEM_SERVICES_H

#include "game/session_time_word.h"
#include "game/item_model_cache.h"
#include "game/item_scuba_models.h"
#include <algorithm>

namespace hd2::game {

//Concrete common item consumers: the actual selected definition model cache,
//the SAME containing game's word28, and007DD860/007DDA10's kind20 registry.
//Frames/catalogue/clock outlive this service; every RuntimeItem using it must be
//destroyed before it. It does not create placed Item actors or transfer storage.
template<class Frames>
class RuntimeItemServices {
public:
    using Frame=typename Frames::Frame;
    using ItemModelOwner=typename ItemPrimaryModelCache<Frames>::ItemModelOwner;
    RuntimeItemServices(Frames& frames,const frontend::PreMissionItemCatalogue& catalogue,
                        const std::string& suffix,SessionTimeWord game_word28)
        : frames_(frames),models_(frames,catalogue,suffix),scuba_(frames,catalogue,suffix),game_word28_(game_word28) {}
    ~RuntimeItemServices(){clear_model_cache();}
    RuntimeItemServices(const RuntimeItemServices&)=delete;
    RuntimeItemServices& operator=(const RuntimeItemServices&)=delete;
    ItemModelOwner make_item_model(const frontend::PreMissionItem& definition) {return models_.make_item_model(definition);}
    Frame first_person_model(const frontend::PreMissionItem& definition) {return models_.first_person(definition);}
    Frame first_person_magazine(const frontend::PreMissionItem& definition) const {return models_.first_person_magazine(definition);}
    const typename ItemScubaModelCache<Frames>::Parts* scuba_parts(const frontend::PreMissionItem& definition) {return scuba_.parts(definition);}
    Frame find_item_model_frame(Frame frame,const char* name,std::uint32_t mask) {return frames_.actor_find(frame,name,mask);}
    void item_model_visible(Frame frame,bool visible) {frames_.object_visible(frame,visible);}
    std::uint32_t item_game_word28() const {return game_word28_;}
    void register_item_kind20(void* item) {
        if(!item)throw std::logic_error("Cannot register a null kind20 item");
        if(kind20_.size()==kind20_.capacity()) {
            if(kind20_.size()>kind20_.max_size()/2)throw std::length_error("Too many kind20 items");
            kind20_.reserve(kind20_.empty()?1:kind20_.size()*2);
        }
        kind20_.push_back(item); //Original appends; it does not suppress duplicates.
    }
    void unregister_item_kind20(void* item) noexcept {
        for(std::size_t i=0;i!=kind20_.size();++i)if(kind20_[i]==item) {
            kind20_[i]=kind20_.back();kind20_.pop_back();return; //First match, swap/pop.
        }
    }
    const std::vector<void*>& kind20_items() const {return kind20_;}
    void clear_loading_kind20_index()noexcept {kind20_.clear();} //007DCFB0/007D8A90:800=7FC, no item destruction.
    void clear_model_cache() noexcept {
        for(auto index=frontend::PreMissionItemCatalogue::capacity;index!=0;) {
            models_.clear_definition(--index);
            if(index==231)scuba_.clear(); //Same definition's30 closes before its50 part array.
        }
    }
private:
    Frames& frames_;
    ItemPrimaryModelCache<Frames> models_;
    ItemScubaModelCache<Frames> scuba_;
    SessionTimeWord game_word28_;
    std::vector<void*> kind20_;
};

} //namespace hd2::game
#endif
