#ifndef HD2_HOST_GAME_MISSION_ITEMS_H
#define HD2_HOST_GAME_MISSION_ITEMS_H

#include "game/session_time_word.h"
#include "host/game_runtime_item.h"
#include "host/frame_clock.h"
#include "game/item_identity.h"
#include "game/loading_item_transfer.h"
#include "game/prepared_item_transfer.h"
#include "game/mission_item_records.h"
#include <deque>

namespace hd2::host {

//Actual Items instance index007DC6E0/007DC8A0, module loading007DCEA0,
//activation007DD010 and record producer007D81A0. The SAME selected catalogue,
//model service, entry registry, game word28 and clock are borrowed. Consumers
//provides actual current mode, actor limits and sound/network receivers.
//Destroy runtime/Scene2 actors before this owner, then models/catalogue/services.
template<class Consumers>
class GameMissionItems {
public:
    using Entry=GameActorServices::Entry;
    using RuntimeEntry=GameRuntimeItemEntry<GameRuntimeItemServices>;
    using Native=typename RuntimeEntry::Native;
    struct Placed {
        Entry entry;
        std::array<float,4> position,rotation;
    };
    GameMissionItems(GameActorServices& frames,GameRuntimeItemServices& models,
                     const frontend::PreMissionItemCatalogue& catalogue,ChunkFiles& files,
                     FrameClockPlatform& clock,game::SessionTimeWord game_word28,Consumers& consumers)
        : frames_(frames),models_(models),catalogue_(catalogue),files_(files),clock_(clock),game_word28_(game_word28),consumers_(consumers) {}
    ~GameMissionItems() {clear();}
    GameMissionItems(const GameMissionItems&)=delete;
    GameMissionItems& operator=(const GameMissionItems&)=delete;
    Entry create_item(std::int32_t definition) {
        if(clearing_)throw std::logic_error("Item construction during module teardown");
        tracked_.reserve(tracked_.size()+1);
        auto owner=std::make_unique<OwnedEntry>(*this,definition);
        auto* entry=owner.get();
        frames_.register_entry(std::move(owner));
        tracked_.push_back(entry);
        return entry;
    }
    Entry create_static_weapon_item(std::int32_t definition) { //007D7AD0, actual item family factory and same module publication.
        if(definition<0||definition>=500||!catalogue_.item(definition))return nullptr;
        const auto entry=create_item(definition);
        try{runtime_item(entry).identity=-1;register_loaded_item(entry);}catch(...){frames_.release_entry(entry);throw;}
        return entry;
    }
    const auto& catalogue()const{return catalogue_;}
    Native& runtime_item(Entry entry) {
        if(std::find(tracked_.begin(),tracked_.end(),entry)==tracked_.end())
            throw std::runtime_error("Item has no live native module owner");
        auto* owner=dynamic_cast<OwnedEntry*>(entry);
        if(!owner||&owner->module!=this)throw std::runtime_error("Item belongs to a different native module");
        return owner->value.item();
    }
    Native& runtime_item_identity(const void* identity){ //Read-only UI loan of SAME kind20 registry, membership BEFORE dereference.
        for(auto* entry:tracked_){auto& item=runtime_item(entry);if(&item==identity)return item;}
        throw std::logic_error("Map item borrower is absent from actual Items owner");
    }
    std::uint32_t human_entry_alignment(Entry entry){return runtime_item(entry).definition().fields38[3];} //005A84F0 descriptor44.
    const auto* scuba_parts(const frontend::PreMissionItem& definition) {
        return models_.scuba_parts(definition); //230 borrows the SAME module's definition231 cache.
    }
    auto item_icon(const frontend::PreMissionItem& definition){return models_.item_icon(definition);}
    auto first_person_model(const frontend::PreMissionItem& definition) {return models_.first_person_model(definition);}
    auto first_person_magazine(const frontend::PreMissionItem& definition) const {return models_.first_person_magazine(definition);}
    void register_loaded_item(Entry entry) {if(entry)registered_.push_back(entry);} //007DC6E0 appends, no deduplication.
    template<class Human>Entry create_prepared_human_item(Human& human,std::int32_t definition,std::int32_t slot,std::int32_t identity) { //007D7810, cold no-source loading.
        if(!catalogue_.item(definition))return nullptr;
        if(slot<0||slot>6)return nullptr;
        const auto entry=create_item(definition);auto& item=runtime_item(entry);
        item.identity=identity<0?game::allocate_item_identity(current_identity_,pending_identities_,clock_):identity;
        const auto result=game::transfer_prepared_cold_item(frames_,human,*this,consumers_,entry,slot,game_word28_,enabled_);
        if(result<1){frames_.release_entry(entry);return nullptr;}
        register_loaded_item(entry);return entry;
    }
    void remove_runtime_item(Entry entry) noexcept { //007DC8A0: first match, swap/pop before matching destruction.
        for(auto i=std::size_t{0};i!=registered_.size();++i)if(registered_[i]==entry) {
            registered_[i]=registered_.back();registered_.pop_back();frames_.release_entry(entry);return;
        }
    }
    void finish_loading(bool authority) { //007DCEA0.
        if(bool(consumers_.mode_flags()&1)!=authority)return;
        require_catalogue();current_identity_=-1;pending_identities_.clear();
    }
    template<class Actors,class HandModels>void finish(bool authority,Actors& actors,HandModels& hand_models) { //007DD240.
        if(bool(consumers_.mode_flags()&1)!=authority)return;
        current_identity_=-1;pending_identities_.clear();enabled_=false;
        //007D8A90 visits the SAME Scene2 identities in ascending order. This is
        //inventory retirement, not deletion of all registered83C items/models.
        for(std::size_t n=0;n<actors.scene_actor_identity_count();++n)if(auto* actor=actors.scene_actor_identity(static_cast<std::int32_t>(n))) {
            if(auto* storage=actor->base().storage())storage->reset_spawn_entries(); //007DDEF0.
            actor->base().field7c(0);
        }
        std::vector<Placed>().swap(placed_); //7E8/count/capacity zero; entries already transferred or retired above.
        models_.clear_loading_kind20_index();
        hand_models.clear();
    }
    void unload() { //007DCFB0, authority ignored. Only the still-queued placements own entries here.
        //The source entry sidecar retracts deleted placement references. Native
        //leaves stale rows until007D8A90; consume them to make failed/repeat
        //unload safe without destroying registered83C items a second time.
        while(!placed_.empty()) {
            const auto entry=placed_.front().entry;
            frames_.release_entry(entry);
            if(!placed_.empty()&&placed_.front().entry==entry)placed_.erase(placed_.begin());
        }
        models_.clear_loading_kind20_index();
    }
    void activate(bool authority,const std::string& mission) { //007DD010, required loading path.
        flag7e5_=false;
        if(bool(consumers_.mode_flags()&1)!=authority)return;
        require_catalogue();enabled_=true;current_identity_=-1;pending_identities_.clear();
        if(authority) {
            const auto path="missions\\"+mission+"\\items.dat";
            const auto bytes=frontend::pre_mission_detail::read_file(files_,path.c_str());
            if(bytes)load_records(*bytes);
        }
    }
    void load_records(const std::vector<unsigned char>& bytes) {
        game::SerializedBytes input(bytes);
        const auto records=game::read_mission_item_records(input);
        for(const auto& record:records) {
            if(!catalogue_.item(record.definition)) {
                std::fprintf(stderr,"Mission item definition is unavailable: %s id=%d\n",record.frame_name.c_str(),record.definition);
                continue; //Original diagnoses and continues, without allocating an identity.
            }
            registered_.reserve(registered_.size()+1);
            const auto entry=create_item(record.definition);
            bool published=false;
            try {
                auto& item=runtime_item(entry);
                if(record.field16&&item.family()==Native::Family::weapon&&item.capacity()>0)item.subtract_amount(item.amount());
                item.identity=game::allocate_item_identity(current_identity_,pending_identities_,clock_);
                item.flag1d=record.flag1d;
                if(record.placed) {
                    if(enabled_) {item.flag1c=true;placed_.push_back(Placed{entry,record.position,record.rotation});}
                } else {
                    auto frame=frames_.find_scene_frame(record.frame_name,0x80);
                    const auto* data=frame?frames_.find_actor_frame_data(frame):nullptr;
                    if(!data||!data->words[0]) {frames_.release_entry(entry);continue;}
                    auto* actor=reinterpret_cast<game::SceneActor<GameActorServices>*>(data->words[0]);
                    struct Restore {bool& flag;bool old;~Restore(){flag=old;}} restore{enabled_,enabled_};
                    enabled_=false;
                    Entry displaced=nullptr;
                    const auto result=game::transfer_loaded_item(frames_,*this,consumers_,entry,displaced,actor->base(),record.slot,game_word28_);
                    if(result<1) {
                        std::fprintf(stderr,"Mission item transfer rejected: %s slot=%d result=%d\n",record.frame_name.c_str(),record.slot,result);
                        frames_.release_entry(entry);continue;
                    }
                    if(displaced)frames_.release_entry(displaced);
                }
                register_loaded_item(entry);published=true;
            } catch(...) {
                if(!published)frames_.release_entry(entry);
                throw;
            }
        }
    }
    const std::vector<Placed>& placed_items() const {return placed_;}
    void consume_placed_items() noexcept {placed_.clear();} //00477660 calls after its actual actor/effect consumers.
    const std::vector<Entry>& registered_items() const {return registered_;}
    //Source lifetime guard for actual UI borrowers; no second item owner or
    //virtual game operation. UI detaches before module/World retirement.
    using UiRetirement=void(*)(void*,Entry)noexcept;
    void check_ui_retirement_binding()const{if(ui_retirement_owner_)throw std::logic_error("Items already has its actual UI retirement borrower");}
    void bind_ui_retirement(void* owner,UiRetirement receiver){check_ui_retirement_binding();if(!owner||!receiver)throw std::invalid_argument("Items UI retirement requires owner/receiver");ui_retirement_owner_=owner;ui_retirement_=receiver;}
    void unbind_ui_retirement(void* owner)noexcept{if(ui_retirement_owner_==owner){ui_retirement_=nullptr;ui_retirement_owner_=nullptr;}}
    bool equipment_notifications_enabled()const{return enabled_;} //Actual module7E4, shared with record-loading guard.
    bool dynamic_item_server_policy() const {return consumers_.dynamic_item_server_policy();}
    float dynamic_item_random_unit() {return consumers_.dynamic_item_random_unit();}
    std::string dynamic_item_caption(std::uint32_t id) {return consumers_.dynamic_item_caption(id);}
    template<class Actor>void dynamic_item_contact_sound(Actor& actor) {consumers_.dynamic_item_contact_sound(actor);}
    template<class Actor>void dynamic_item_sleep_notification(Actor& actor) {consumers_.dynamic_item_sleep_notification(actor);}
    void clear() noexcept {
        if(clearing_)return;
        clearing_=true;placed_.clear();
        //Native sidecar guarantees entries release BEFORE the borrowed index
        //and model service, including construction that failed before83C append.
        while(!tracked_.empty())frames_.release_entry(tracked_.back());
        registered_.clear();pending_identities_.clear();clearing_=false;
    }
private:
    class OwnedEntry final : public GameActorServices::Item {
    public:
        OwnedEntry(GameMissionItems& owner,std::int32_t definition)
            : module(owner),value(owner.frames_,owner.models_,owner.catalogue_,definition) {}
        ~OwnedEntry() override {module.forget(this);} //Before pack destruction recursively releases other entries.
        float scalar() const override {return value.scalar();}
        std::uint32_t definition_identifier() const override {return value.definition_identifier();}
        std::uint32_t instance_family() const override {return value.instance_family();}
        long double volume() const override {return value.volume();}
        GameMissionItems& module;RuntimeEntry value;
    };
    void forget(Entry entry) noexcept {
        if(ui_retirement_)ui_retirement_(ui_retirement_owner_,entry); //Before actual RuntimeItem/pack contents destruction.
        const auto remove=[&](auto& list) {
            for(auto i=list.size();i!=0;)if(list[--i]==entry) {list[i]=list.back();list.pop_back();}
        };
        remove(tracked_);remove(registered_);
        //Retire borrowed placement references if a failed load deletes the entry.
        for(auto i=placed_.size();i!=0;)if(placed_[--i].entry==entry) {placed_.erase(placed_.begin()+static_cast<std::ptrdiff_t>(i));}
    }
    void require_catalogue() const {if(!catalogue_.loaded())throw std::runtime_error("Items module requires the actual selected catalogue");}
    GameActorServices& frames_;GameRuntimeItemServices& models_;
    const frontend::PreMissionItemCatalogue& catalogue_;ChunkFiles& files_;FrameClockPlatform& clock_;
    game::SessionTimeWord game_word28_;Consumers& consumers_;
    std::vector<Entry> tracked_,registered_;
    void* ui_retirement_owner_=nullptr;UiRetirement ui_retirement_=nullptr;
    std::vector<Placed> placed_;
    std::int32_t current_identity_=-1;
    std::deque<game::PendingItemIdentity> pending_identities_;
    bool enabled_=false,flag7e5_=false,clearing_=false;
};

} //namespace hd2::host
#endif
