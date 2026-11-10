#ifndef HD2_GAME_SHOT_MANAGER_LOADING_H
#define HD2_GAME_SHOT_MANAGER_LOADING_H

#include "frontend/module_registration.h"
#include "frontend/cleanup.h"
#include "game/human_joint_owners.h"
#include <array>
#include <list>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hd2::game {

//0044D920's cold reusable records. Unknown bytes are defined zero for source
//safety and earn no native initialization credit. No projectile/decal producer
//can populate their unrecovered pointer/resource fields through this owner.
struct alignas(16) ShotColdMark {
    ShotColdMark(){words.set(0x5c,std::uint32_t{1});words.set(0x64,std::uint32_t{0});words.set(0x68,std::uint32_t{0});words.set(0x6c,std::uint8_t{0});} //0044DB40.
    HumanJointWords<0xc0> words;
};
struct alignas(16) ShotColdDecal {
    ShotColdDecal(){for(auto offset:{0x20u,0x24u,0x28u})words.set(offset,std::uint32_t{0});} //0044DB10, empty resource/vector fields only.
    HumanJointWords<0x40> words;
};
template<class Record>class ShotColdPool {
public:
    ShotColdPool(){for(unsigned n=0;n!=16;++n)push(std::make_unique<Record>());} //Native pool limit18=16; original aligned allocation.
    ShotColdPool(const ShotColdPool&)=delete;ShotColdPool& operator=(const ShotColdPool&)=delete;
    void clear()noexcept{while(!free_.empty())free_.pop_front();} //LIFO cold null-resource destructors, then their list nodes.
    std::size_t size()const{return free_.size();}
private:
    void push(std::unique_ptr<Record> record){if(record)free_.push_front(std::move(record));} //004531D0/00453270 cold insertion, no acquisition API.
    std::list<std::unique_ptr<Record>> free_;
};

//Actual process ShotMgr resources/pools and separate0089A940 hit-zone owner.
//Construct once with Startup; a world facade retires engine resources BEFORE
//its frames/scene, retaining process vector capacity and definition metadata.
template<class Frames>struct ShotManagerLoadingState {
    using Frame=typename Frames::Frame;
    using ModelOwner=decltype(std::declval<Frames&>().create_shot_model());
    using TextureOwner=typename Frames::ResourceOwner;
    ShotColdPool<ShotColdMark> marks;ShotColdPool<ShotColdDecal> decals;
    struct RuntimeRecord; //Actual active projectile/decal producers are not exposed by loading.
    std::array<std::vector<RuntimeRecord*>,4> render; //Native80/90/A0/B0.
    std::array<std::vector<RuntimeRecord*>,3> transient; //114/124/134.
    std::optional<ModelOwner> hit_zones,odds;
    std::array<Frame,10> hit_frames{}; //BSS cold zero;00455520 deliberately does not clear these borrowers.
    inline static constexpr std::array<const char*,10> hit_names{{"c","f","l","m","h","i","s","t","o","p"}};
    inline static constexpr std::array<unsigned,10> hit_bones{{1,6,13,14,9,10,20,21,16,17}}; //00455370.
    Frame trace{},shell_third{},shell_first{};
    std::array<std::optional<TextureOwner>,9> decal_textures;
    std::optional<TextureOwner> explosion_texture;
    std::array<std::optional<TextureOwner>,6> blood_textures;
    std::uint32_t maximum148=0,maximum14c=0,maximum150=0,field140=0;
    const void* world=nullptr; //Source borrower guard, not an original scalar or readiness field.
};

//0044DDE0 cold loading and0044E110 retirement. Same process registration/state,
//real model/cache/texture/performance endpoints; no live shot/damage/rendering
//handler or positive populated runtime-list cleanup is supplied.
template<class Frames>class ShotManagerLoading {
public:
    using State=ShotManagerLoadingState<Frames>;using Frame=typename Frames::Frame;
    ShotManagerLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,
            Frames& frames,State& state,const std::uint32_t& mode)
        :registration_(registry,registration,14,"ShotMgr"),frames_(frames),state_(state),mode_(mode){
        if(state_.world)throw std::logic_error("ShotMgr already belongs to an actual world");
        if(state_.hit_zones||state_.odds||state_.explosion_texture)throw std::logic_error("Retire prior ShotMgr resources before binding another world");
        for(const auto& texture:state_.decal_textures)if(texture)throw std::logic_error("Retire prior ShotMgr decal textures");
        for(const auto& texture:state_.blood_textures)if(texture)throw std::logic_error("Retire prior ShotMgr blood textures");
        for(const auto& rows:state_.render)require_empty(rows);
        for(const auto& rows:state_.transient)require_empty(rows);
        state_.world=this;
    }
    ~ShotManagerLoading(){close();}
    ShotManagerLoading(const ShotManagerLoading&)=delete;ShotManagerLoading& operator=(const ShotManagerLoading&)=delete;
    auto& registration(){return registration_.get();}
    void tick(bool authority,std::uint32_t elapsed){ //00451EC0/00451FB0, both absent frozen; cold source queues only, no new credit.
        require_open();
        if(authority){require_empty(state_.transient[0]);return;}
        if(!(mode_&1u)){
            state_.field140+=elapsed;std::int32_t time;std::memcpy(&time,&state_.field140,4);
            if(time>100){state_.field140-=100;tick(true,100);} //Strict signed>100, ONE subtraction BEFORE virtual0C.
            require_empty(state_.transient[1]);
        }
        require_empty(state_.transient[0]); //Projectile visual virtual0C pass, no retirement in this pass.
        require_empty(state_.transient[2]);
        require_empty(state_.render[3]);require_empty(state_.render[2]);require_empty(state_.render[1]);
        //Native render80 vector is not updated here. Actual populated queue
        //producers/records remain unavailable; never dispatch an opaque pointer.
    }
    void finish_loading(bool)noexcept{} //004115E0 genuine RET4, module14 virtual14.
    void activate(bool authority){
        require_open();
        frames_.shot_query_performance_clock(); //QPF THEN QPC on BOTH paths; native ignores results.
        if(authority){
            if(authority_started_)throw std::logic_error("Retire ShotMgr hit zones before another activation");
            authority_started_=true;
            state_.hit_zones.emplace(frames_.create_shot_model());
            const auto model=state_.hit_zones->get();if(!model)throw std::runtime_error("Cannot create actual hit-zone model");
            if(frames_.load_shot_model(model,"humanzones")!=0){state_.hit_zones.reset();return;} //00455490 requires EXACT zero.
            for(unsigned n=0;n!=10;++n)state_.hit_frames[n]=frames_.actor_find(model,State::hit_names[n],0xffff);
            frames_.link_scene_object(model,frames_.shot_hidden_sector());frames_.object_visible(model,false);return;
        }
        if(presentation_started_)throw std::logic_error("Retire ShotMgr presentation before another activation");
        presentation_started_=true;
        state_.maximum148=state_.maximum14c=100;state_.maximum150=(mode_&7u)==7u?50u:20u;
        state_.trace=state_.shell_third=state_.shell_first=Frame{};
        state_.odds.emplace(frames_.create_shot_model());
        const auto model=state_.odds->get();if(!model)throw std::runtime_error("Cannot create actual WeaponOdds model");
        if(frames_.load_shot_model(model,"WeaponOdds")>=0){ //Distinct from humanzones' exact-zero test.
            state_.trace=frames_.actor_find(model,"trace",1);state_.shell_third=frames_.actor_find(model,"shell3rd",1);state_.shell_first=frames_.actor_find(model,"shellFPV",1);
        }
        for(unsigned n=0;n!=3;++n){
            require_empty(state_.render[n]);
            state_.render[n].reserve(n==0?state_.maximum148:n==1?state_.maximum14c:state_.maximum150);
        }
        load_textures();state_.field140=0;
    }
    void finish(bool authority){
        require_open();
        if(authority){state_.hit_zones.reset();authority_started_=false;} //00455520 leaves borrowed marker entries intact.
        else{
            require_empty(state_.transient[2]);for(const auto& rows:state_.render)require_empty(rows);
            require_empty(state_.transient[1]);state_.decals.clear();
            for(auto& texture:state_.decal_textures)texture.reset();
            state_.explosion_texture.reset();for(auto& texture:state_.blood_textures)texture.reset();
            if(state_.odds&&state_.odds->get()){state_.odds.reset();state_.trace=Frame{};} //Native C8/CC borrowers are not cleared here.
            else state_.odds.reset();
            presentation_started_=false;
        }
        require_empty(state_.transient[0]);state_.marks.clear(); //Common tail, including authority-only retirement.
    }
    void close()noexcept{
        if(state_.world!=this)return;
        frontend::CleanupErrors errors;
        if(authority_started_)errors.attempt([&]{finish(true);});
        if(presentation_started_)errors.attempt([&]{finish(false);});
        state_.world=nullptr;
        if(errors.failed())try{errors.rethrow();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
private:
    void require_open()const{if(state_.world!=this)throw std::logic_error("ShotMgr world borrower is closed");}
    template<class Rows>static void require_empty(const Rows& rows){if(!rows.empty())throw std::runtime_error("ShotMgr runtime records require their actual projectile/decal producer and retirement");}
    static std::string texture_name(const char* prefix,unsigned n){return std::string(prefix)+(n<10?"0":"")+std::to_string(n)+".tga";}
    void load_textures(){ //0044E520, all native status results ignored; retain actual output pointers.
        for(unsigned n=0;n!=9;++n)state_.decal_textures[n].emplace(frames_.load_shot_texture(texture_name("x_decal_",n)));
        state_.explosion_texture.emplace(frames_.load_shot_texture("x_expl_00.tga"));
        for(unsigned n=0;n!=6;++n)state_.blood_textures[n].emplace(frames_.load_shot_texture(texture_name("x_blood_",n)));
    }
    frontend::ClientModuleBinding registration_;Frames& frames_;State& state_;const std::uint32_t& mode_;
    bool authority_started_=false,presentation_started_=false;
};
} //namespace hd2::game
#endif
