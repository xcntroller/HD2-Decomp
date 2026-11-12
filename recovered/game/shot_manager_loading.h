#ifndef HD2_GAME_SHOT_MANAGER_LOADING_H
#define HD2_GAME_SHOT_MANAGER_LOADING_H

#include "frontend/module_registration.h"
#include "frontend/cleanup.h"
#include "game/human_joint_owners.h"
#include "game/shot_surface_marks.h"
#include "game/shot_blood_marks.h"
#include "game/shot_bullet_record.h"
#include "game/shot_muzzle_flash.h"
#include "frontend/spatial_sound.h"
#include <array>
#include <list>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hd2::game {

//0044D920's cold reusable decal records. Unknown bytes are defined zero for
//source safety and earn no native initialization credit. Bullet records now
//use the actual typed mark pool below; decal positive fields remain unavailable.
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
    std::unique_ptr<Record> acquire(){ //00453140, refill the actual pool in batches of16, then take its head.
        if(free_.empty())for(unsigned n=0;n!=16;++n)push(std::make_unique<Record>());
        auto result=std::move(free_.front());free_.pop_front();return result;
    }
    void recycle(std::unique_ptr<Record>& record){ //Allocate the node BEFORE transferring the active owner.
        free_.push_front(nullptr);free_.front()=std::move(record);
    }
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
    using Bullet=ShotBulletRecord<Frames>;using RuntimeRecord=ShotRuntimeRecord<Frames>;
    ShotColdPool<Bullet> marks;ShotColdPool<ShotColdDecal> decals;
    std::vector<RuntimeRecord*> render; //Native80, positive producer remains required.
    using MuzzleFlash=ShotMuzzleFlash<Frames>;
    std::vector<std::unique_ptr<MuzzleFlash>> muzzle_flashes; //NativeB0/B4/B8, SAME process owner.
    using BloodMark=ShotBloodMark<Frames>;
    std::vector<std::unique_ptr<BloodMark>> blood_marks; //Native90/94/98, SAME ShotMgr owner.
    using SurfaceMark=ShotSurfaceMark<Frames>;
    std::vector<std::unique_ptr<SurfaceMark>> surfaces; //Native A0/A4/A8, SAME actual ShotMgr pool.
    std::array<std::vector<std::unique_ptr<RuntimeRecord>>,3> transient; //114/124/134, actual owned records.
    std::optional<ModelOwner> hit_zones,odds;
    std::array<Frame,10> hit_frames{}; //BSS cold zero;00455520 deliberately does not clear these borrowers.
    inline static constexpr std::array<const char*,10> hit_names{{"c","f","l","m","h","i","s","t","o","p"}};
    inline static constexpr std::array<unsigned,10> hit_bones{{1,6,13,14,9,10,20,21,16,17}}; //00455370.
    Frame trace{},shell_third{},shell_first{};
    std::array<std::optional<TextureOwner>,9> decal_textures;
    std::optional<TextureOwner> explosion_texture;
    std::array<std::optional<TextureOwner>,6> blood_textures;
    std::uint32_t maximum148=0,maximum14c=0,maximum150=0,field140=0;
    frontend::SpatialSoundRequest hit_sound; //Actual reusable D0 request, initialized by004374D0.
    HumanVector hit_sound_position{}; //Native full D0..DC copy, W unused by SOUND.
    const void* world=nullptr; //Source borrower guard, not an original scalar or readiness field.
};

//0044DDE0 cold loading and0044E110 retirement. Same process registration/state,
//real model/cache/texture/performance endpoints and owned bullet/light queues.
//Actual selected bullet frame/damage receivers are still required separately.
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
        require_empty(state_.render);require_empty(state_.muzzle_flashes);
        require_empty(state_.surfaces);
        require_empty(state_.blood_marks);
        for(const auto& rows:state_.transient)require_empty(rows);
        state_.world=this;
    }
    ~ShotManagerLoading(){close();}
    ShotManagerLoading(const ShotManagerLoading&)=delete;ShotManagerLoading& operator=(const ShotManagerLoading&)=delete;
    auto& registration(){return registration_.get();}
    void tick(bool authority,std::uint32_t elapsed){ //00451EC0/00451FB0, both absent frozen; no new inventory credit.
        require_open();
        if(authority){
            auto& rows=state_.transient[0];
            for(std::size_t n=0;n<rows.size();){
                if(!rows[n])throw std::logic_error("ShotMgr active record has no owner");
                if(rows[n]->authority_tick(elapsed)!=1u){++n;continue;}
                if(rows[n]->kind()!=1u)throw std::runtime_error("ShotMgr requires actual selected non-bullet retirement");
                auto* bullet=dynamic_cast<typename State::Bullet*>(rows[n].get());
                if(!bullet)throw std::logic_error("ShotMgr kind1 must use its actual bullet record");
                bullet->retire();
                std::unique_ptr<typename State::Bullet> owner(static_cast<typename State::Bullet*>(rows[n].release()));
                try{state_.marks.recycle(owner);}catch(...){rows[n]=std::move(owner);throw;}
                rows.erase(rows.begin()+static_cast<std::ptrdiff_t>(n)); //Stable erase, revisit shifted entry.
            }
            return;
        }
        if(!(mode_&1u)){
            state_.field140+=elapsed;std::int32_t time;std::memcpy(&time,&state_.field140,4);
            if(time>100){state_.field140-=100;tick(true,100);} //Strict signed>100, ONE subtraction BEFORE virtual0C.
            require_empty(state_.transient[1]);
        }
        for(std::size_t n=0;n<state_.transient[0].size();++n){
            auto* record=state_.transient[0][n].get();
            if(!record)throw std::logic_error("ShotMgr presentation record has no owner");
            record->presentation_tick(elapsed); //Reload vector/count after callbacks, no retirement in this pass.
        }
        require_empty(state_.transient[2]);
        for(std::size_t n=0;n<state_.muzzle_flashes.size();){ //Native B0 pass precedes A0, then90.
            if(state_.muzzle_flashes[n]->tick(elapsed)!=2u){++n;continue;}
            state_.muzzle_flashes[n]->clear();state_.muzzle_flashes.erase(state_.muzzle_flashes.begin()+static_cast<std::ptrdiff_t>(n));
        }
        for(auto& mark:state_.surfaces)(void)mark->tick(frames_,elapsed);
        for(auto& mark:state_.blood_marks)(void)mark->tick(frames_,elapsed);
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
        require_empty(state_.render);state_.render.reserve(state_.maximum148);
        require_empty(state_.blood_marks);state_.blood_marks.reserve(state_.maximum14c);
        require_empty(state_.surfaces);state_.surfaces.reserve(state_.maximum150);
        load_textures();state_.field140=0;
    }
    void finish(bool authority){
        require_open();
        if(authority){state_.hit_zones.reset();authority_started_=false;} //00455520 leaves borrowed marker entries intact.
        else{
            require_empty(state_.transient[2]);require_empty(state_.render);
            while(!state_.blood_marks.empty()){state_.blood_marks.back()->clear();state_.blood_marks.pop_back();}
            //0044E110 retires A0 in reverse before B0, BEFORE texture60 and Scene retirement.
            while(!state_.surfaces.empty()){state_.surfaces.back()->clear();state_.surfaces.pop_back();}
            while(!state_.muzzle_flashes.empty()){state_.muzzle_flashes.back()->clear();state_.muzzle_flashes.pop_back();}
            require_empty(state_.transient[1]);state_.decals.clear();
            for(auto& texture:state_.decal_textures)texture.reset();
            state_.explosion_texture.reset();for(auto& texture:state_.blood_textures)texture.reset();
            if(state_.odds&&state_.odds->get()){state_.odds.reset();state_.trace=Frame{};} //Native C8/CC borrowers are not cleared here.
            else state_.odds.reset();
            presentation_started_=false;
        }
        while(!state_.transient[0].empty()){
            state_.transient[0].back()->retire();state_.transient[0].pop_back();
        }
        state_.marks.clear(); //Common tail, including authority-only retirement, AFTER live records.
    }
    void close()noexcept{
        if(state_.world!=this)return;
        const bool had_runtime=authority_started_||presentation_started_||!state_.transient[0].empty()||!state_.muzzle_flashes.empty();
        frontend::CleanupErrors errors;
        if(authority_started_)errors.attempt([&]{finish(true);});
        if(presentation_started_)errors.attempt([&]{finish(false);});
        //A failed engine/particle retirement must remain visible, but callbacks
        //cannot survive their World/Frames borrowers. Drain the actual owners
        //while these dependencies are still alive, including partial activation.
        while(!state_.transient[0].empty()){
            if(state_.transient[0].back())errors.attempt([&]{state_.transient[0].back()->retire();});
            state_.transient[0].pop_back();
        }
        while(!state_.muzzle_flashes.empty()){
            errors.attempt([&]{state_.muzzle_flashes.back()->clear();});state_.muzzle_flashes.pop_back();
        }
        if(had_runtime)state_.marks.clear();
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
