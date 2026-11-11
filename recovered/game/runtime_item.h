#ifndef HD2_GAME_RUNTIME_ITEM_H
#define HD2_GAME_RUNTIME_ITEM_H

#include "frontend/pre_mission_data.h"
#include "game/actor_slot_storage.h"
#include "game/actor_base.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>

namespace hd2::game {

//Native ownership for007DD400/480/4F0/560's four item instance families.
//The selected preparation catalogue is borrowed, never decoded into a competing
//catalogue. Services MUST supply actual model/cache/clone and kind20 registry
//operations; this owner has no model, weapon simulation or network substitutes.
//Models use matching nonthrowing owners, including their child-frame lifetime.
template<class Backend,class Services>
class RuntimeItem {
public:
    using Definition=frontend::PreMissionItem;
    using Catalogue=frontend::PreMissionItemCatalogue;
    using Frame=typename Backend::Frame;
    using ModelOwner=typename Services::ItemModelOwner;
    enum class Family : std::uint32_t {weapon=0,pack=1,basic=2,other=3}; //007D39C0+08.
    static std::unique_ptr<RuntimeItem> create(Backend& backend,Services& services,
                                              const Catalogue& catalogue,std::int32_t id) {
        const auto* definition=catalogue.item(id);
        if(!definition)throw std::runtime_error("Runtime item has no selected catalogue definition");
        const auto family=definition->item_class==1?Family::weapon:definition->item_class==0?Family::basic:
            payload_kind(definition->payloads[0])==7?Family::pack:Family::other;
        auto owner=std::unique_ptr<RuntimeItem>(new RuntimeItem(backend,services,catalogue,*definition,family));
        owner->initialize();return owner;
    }
    ~RuntimeItem() {
        if(slots_) {slots_->clear_entries();slots_.reset();} //007D38D0 explicitly clears even borrowed slots.
        effect_models_[1].reset();effect_models_[0].reset(); //007D6300.
        auxiliary_model_.reset(); //007D4710's+78, before common cleanup.
        if(kind20_registered_)services_.unregister_item_kind20(this); //007DDA10 swap/pop.
        model_.reset(); //007D3A10→007D3AE0(0), matching native model owner.
    }
    RuntimeItem(const RuntimeItem&)=delete;
    RuntimeItem& operator=(const RuntimeItem&)=delete;
    Family family() const {return family_;}
    const Definition& definition() const {return definition_;}
    float scalar() const {return definition_.weight;} //007D3290, shared holder/slot consumer+2C.
    std::int32_t storage_size() const {return frontend::pre_mission_detail::Bytes::integer(definition_.category);} //007D32A0.
    long double volume() const {const auto size=storage_size();return size?static_cast<long double>(size)*2:1.L;} //007D32B0.
    std::int32_t primary_payload_kind() const {return payload_kind(definition_.payloads[0]);}
    std::int32_t primary_payload_command()const{ //Native payload+0C, serialized after its vtable.
        return frontend::pre_mission_detail::Bytes::integer(payload_word(definition_.payloads[0],8));
    }
    float primary_payload_ballistic_speed()const{return real(payload_word(definition_.payloads[0],0x34));} //Native payload+38.
    std::uint32_t ai_shot_interval()const{ //0058BAF0, native payload+18.
        return weapon_payload_time(0x14);
    }
    bool allows_partial_reload()const{ //007E02F0, native payload byte20.
        const auto& payload=definition_.payloads[0];
        if(payload.type<1||payload.type>4)throw std::runtime_error("Weapon reload requires the actual reloadable payload family");
        return payload.bytes.at(0x1c)!=0;
    }
    std::uint32_t reload_duration()const{
        switch(definition_.payloads[0].type){
        case 3:return weapon_payload_time(0x30); //007E0340, native34.
        case 4:return weapon_payload_time(0x2c); //007E03A0, native30.
        default:return 0; //007E02B0.
        }
    }
    std::int32_t reload_sound()const{
        const auto& payload=definition_.payloads[0];
        if(payload.type==3)return frontend::pre_mission_detail::Bytes::integer(payload_word(payload,0x2c));
        if(payload.type==4)return frontend::pre_mission_detail::Bytes::integer(payload_word(payload,0x28));
        return -1; //007E02A0.
    }
    float loaded_ammunition()const{return real(fields50_54[0]);}
    void loaded_ammunition(float value){fields50_54[0]=bits(value);}
    void magazine_amount(float value){
        if(family_!=Family::basic)throw std::logic_error("Reload remainder requires an actual ammunition instance");
        amount_=value;
    }
    bool pack_exposes_contents() const {return primary_payload_kind()==7&&payload_word(definition_.payloads[0],8)==0;}
    std::int32_t identity=-1; //Assigned by the actual item module after construction, not by this factory.
    bool flag1c=false,flag1d=false;
    std::uint32_t field14=0;
    ActorBase<Backend>* holder_actor=nullptr; //Actual native20 pointer, never serialized as uint32.
    std::int32_t holder_slot=0; //Native24.
    std::array<std::uint32_t,2> fields50_54{{0xbf800000u,0xbf800000u}};
    std::uint32_t field58=0,field5c=0,field60=1,field64=0,field68=0,field6c=0;
    Frame projectile_marker{}; //Native74 borrowed from the primary model.
    std::array<float,4> projectile_position{},projectile_rotation{}; //Native30/40, lazy007D5250 snapshot.
    std::uint32_t field7c=1,field88=1,field8c=0,field9c=0,fielda0=0,fielda8=0xffffffffu;
    Frame shot_marker{}; //Native A4 borrowed child, never truncate its identity into a DWORD.
    bool flag90=false;
    float field98=1;
    float amount() const { //007D3320/007D3870/007D4680.
        if(family_==Family::basic)return amount_;
        if(family_!=Family::weapon)return -1;
        if(services_.item_game_word28()<field5c)return 0;
        return real(fields50_54[0]);
    }
    float capacity() const { //007D3330/007D3880/007D46A0.
        return family_==Family::basic?capacity_:family_==Family::weapon?real(fields50_54[1]):-1;
    }
    void subtract_amount(float value) { //007D3240 is RET4 for the other three families.
        if(family_!=Family::weapon)return;
        auto result=real(fields50_54[0])-value;
        if(result<0)result=0;
        fields50_54[0]=bits(result); //007D4650: negative clamp, no upper clamp.
    }
    ActorSlotStorage<Backend>* slots() {return slots_.get();}
    Frame model(unsigned part=0) { //007D3AA0 accepts only part0.
        if(part)return Frame{};
        if(!model_||!model_->get())model_.emplace(services_.make_item_model(definition_));
        return model_?model_->get():Frame{};
    }
    Frame flash_frame() const {return flash_;} //Borrowed from primary model.
    std::uint32_t reload_animation_family() { //Item virtual40,007D46D0 / genuine007D32F0 zero.
        if(family_!=Family::weapon)return 0;
        if(field7c==1)classify_projectile_marker();
        return field7c;
    }
    Frame shot_frame() { //0058BA80: absent shdum is NOT cached as a successful fallback.
        if(!shot_marker){
            const auto frame=model();if(!frame)throw std::runtime_error("Item shot marker requires actual primary model");
            shot_marker=services_.find_item_model_frame(frame,"shdum",0x20);
        }
        return shot_marker;
    }
    std::uint32_t other_field28() const {return other_field28_;}
    float auxiliary_amount()const{return family_==Family::other?real(other_field28_):-1.f;} //007D41A0/007D3280 virtual20.
private:
    void classify_projectile_marker() { //007D5250, classification and clone use the actual model hierarchy.
        const auto frame=model();
        projectile_marker=Frame{};auxiliary_model_.reset();field7c=0;
        if(!frame)return;
        projectile_marker=services_.find_item_model_frame(frame,"projectile",0xffff);
        if(!projectile_marker)projectile_marker=services_.find_item_model_frame(frame,"mag1",0xffff);
        if(projectile_marker)field7c=2;
        else {
            projectile_marker=services_.find_item_model_frame(frame,"mag2",0xffff);
            if(projectile_marker)field7c=3;
            else {
                projectile_marker=services_.find_item_model_frame(frame,"mag3",0xffff);
                if(projectile_marker)field7c=4;
            }
        }
        if(projectile_marker){
            //Native code ignores the clone result and still captures the marker pose.
            auxiliary_model_.emplace(services_.clone_item_model_frame(projectile_marker));
            projectile_position=services_.item_model_frame_position(projectile_marker);
            projectile_rotation=services_.item_model_frame_rotation(projectile_marker);
        }
    }
    std::uint32_t weapon_payload_time(std::size_t offset)const{
        const auto value=static_cast<long double>(real(payload_word(definition_.payloads[0],offset)));
        if(!std::isfinite(value)||value<std::numeric_limits<std::int64_t>::min()||
           value>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))
            throw std::runtime_error("Weapon payload time is outside original conversion range");
        return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
    }
    RuntimeItem(Backend& backend,Services& services,const Catalogue& catalogue,const Definition& definition,Family family)
        : services_(services),catalogue_(catalogue),definition_(definition),family_(family) {
        static_assert(noexcept(std::declval<Services&>().unregister_item_kind20(std::declval<RuntimeItem*>())),
                      "Item registry cleanup must not throw");
        if(family_==Family::pack)slots_=std::make_unique<ActorSlotStorage<Backend>>(backend,2); //007D3800.
    }
    static float real(std::uint32_t word) {return frontend::pre_mission_detail::Bytes::real(word);}
    static std::uint32_t bits(float value) {std::uint32_t word;std::memcpy(&word,&value,4);return word;}
    static std::uint32_t payload_word(const frontend::PreMissionItemPayload& payload,std::size_t offset) {
        if(offset>payload.bytes.size()||payload.bytes.size()-offset<4)
            throw std::runtime_error("Runtime item payload field is truncated");
        return frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+offset);
    }
    static std::int32_t payload_kind(const frontend::PreMissionItemPayload& payload) {
        //007DFCF0 reads serialized payload into native+04, including native+08
        //kind. Outer tag8 normally carries kind7; the tag itself is NOT kind7.
        if(!payload.type)return -1;
        if(payload.type==10&&payload.bytes.empty())return 9; //007DF330/340 are zero-byte serializers.
        return frontend::pre_mission_detail::Bytes::integer(payload_word(payload,4));
    }
    static float definition_capacity(const Definition& definition) {
        return definition.item_class==0?real(definition.derived[0]):-1.f; //007E0410 /007DF3D0.
    }
    void initialize() {
        field14=0;flag1d=definition_.kind()==10; //007D3A40.
        if(definition_.kind()==20) {
            services_.register_item_kind20(this);kind20_registered_=true;
        }
        if(family_==Family::basic) {amount_=capacity_=definition_capacity(definition_);return;} //007D3370.
        if(family_==Family::pack) { //007D3940; native+10/14/18 map to serialized+0C/10/14.
            const auto& payload=definition_.payloads[0];
            slots_->first_scalar(real(payload_word(payload,0xc)));
            slots_->second_scalar(real(payload_word(payload,0x10)));
            slots_->flags(payload_word(payload,0x14));return;
        }
        if(family_==Family::other) {
            if(definition_.kind()==13&&definition_.payloads[0].type)
                other_field28_=payload_word(definition_.payloads[0],8);
            return; //007D41E0.
        }
        float capacity=-1;
        const auto ammunition=definition_.ammunition();
        if(ammunition>=0) {
            const auto* definition=catalogue_.item(ammunition);
            if(!definition)throw std::runtime_error("Weapon ammunition definition is unavailable");
            capacity=definition_capacity(*definition);
        } else if(definition_.kind()==6)capacity=1;
        fields50_54={bits(capacity),bits(capacity)};field58=0;field60=1;field5c=0;
        if(auto frame=model()) {
            flash_=services_.find_item_model_frame(frame,"flash",1);
            if(flash_)services_.item_model_visible(flash_,false);
        }
        field9c=fielda0=field8c=0;shot_marker=Frame{};fielda8=0xffffffffu; //007D4780.
    }
    Services& services_;
    const Catalogue& catalogue_;
    const Definition& definition_;
    Family family_;
    bool kind20_registered_=false;
    std::unique_ptr<ActorSlotStorage<Backend>> slots_;
    std::optional<ModelOwner> model_,auxiliary_model_;
    std::array<std::optional<ModelOwner>,2> effect_models_;
    Frame flash_{};
    float amount_=0,capacity_=0;
    std::uint32_t other_field28_=0;
};

} //namespace hd2::game
#endif
