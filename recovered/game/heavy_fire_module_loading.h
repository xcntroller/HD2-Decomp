#ifndef HD2_GAME_HEAVY_FIRE_MODULE_LOADING_H
#define HD2_GAME_HEAVY_FIRE_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/heavy_fire_definitions.h"
#include "game/heavy_fire_requests.h"

namespace hd2::game {

//Process module16. Native190/1A0 lists own support instances; process configuration
//and flags persist across World borrowers. Positive construction is supplied by
//the actual caller services, never a successful empty support object.
class HeavyFireModuleLoading;
class HeavyFireLoadingState {
public:
    const auto& flags()const{return flags_;}
    const auto& definitions()const{return definitions_;}
    HeavyFireModuleLoading& module(){if(!world_)throw std::logic_error("HeavyFire requires actual bound module16");return *world_;}
private:
    friend class HeavyFireModuleLoading;
    std::array<std::uint8_t,4> flags_{}; //Primary1B0..1B3 = module-subobject1AC..1AF; process BSS zero.
    HeavyFireDefinitions definitions_;bool definitions_initialized_=false;
    HeavyFireModuleLoading* world_=nullptr;
};
class HeavyFireModuleLoading {
public:
    HeavyFireModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,HeavyFireLoadingState& state)
        :registration_(registry,registration,16,"HeavyFireSupportModule"),state_(state){
        if(state_.world_)throw std::logic_error("HeavyFireSupport already belongs to an actual world");
        state_.world_=this;
    }
    template<class Files>HeavyFireModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,HeavyFireLoadingState& state,Files& files)
        :HeavyFireModuleLoading(registry,registration,state){
        try{if(!state_.definitions_initialized_){(void)state_.definitions_.load(files);state_.definitions_initialized_=true;}}
        catch(...){close();throw;}
    }
    ~HeavyFireModuleLoading(){close();}
    HeavyFireModuleLoading(const HeavyFireModuleLoading&)=delete;
    HeavyFireModuleLoading& operator=(const HeavyFireModuleLoading&)=delete;
    auto& registration(){return registration_.get();}
    void finish_loading(bool){require_open();} //004115E0 genuine RET4.
    void activate(bool){require_open();state_.flags_.fill(0);} //00472250, BOTH paths, no mode predicate.
    void finish(bool){require_open();clear_support();} //00472270, first190 then1A0, reverse deletion.
    void update(bool authority,std::uint32_t elapsed){ //00472C60/00472510.
        require_open();advance_list(air_,0,authority,elapsed);advance_list(mortar_,2,authority,elapsed);
    }
    template<class Services>std::uint32_t request_air(const HeavyFireAirRequest& request,Services& services,SessionBroadcastPackets& packets){ //00472E50.
        require_open();if(!(services.mode_flags()&1u))return 2;
        const auto side=checked_side(request.side);if(state_.flags_[side])return 1;
        if(request.type<0||request.type>=16)return 3;
        auto placed=request;placed.direction[1]=0.f;
        auto object=services.make_heavy_fire_air(placed,state_.definitions_.at(static_cast<unsigned>(request.type)));
        if(object){air_.push_back({side,std::move(object)});state_.flags_[side]=1;}
        write_heavy_fire_air_packet(packets,placed);return 0;
    }
    template<class Services>std::uint32_t request_mortar(const HeavyFireMortarRequest& request,Services& services,SessionBroadcastPackets& packets){ //00472CE0.
        require_open();if(!(services.mode_flags()&1u))return 2;
        const auto side=checked_side(request.side);if(state_.flags_[2+side])return 1;
        auto placed=request;placed.position[1]=300.f;
        auto object=services.make_heavy_fire_mortar(placed);
        if(object){mortar_.push_back({side,std::move(object)});state_.flags_[2+side]=1;}
        write_heavy_fire_mortar_packet(packets,placed);return 0;
    }
    template<class Services>bool receive_packet(SessionPacketStream& packet,Services& services){ //00472590, module subobject at primary+4.
        require_open();const auto kind=packet.read(1);
        auto vector=[&]{HumanVector value{};for(unsigned a=0;a!=3;++a){const auto bits=packet.read(32);std::memcpy(&value[a],&bits,4);}return value;};
        auto integer=[&]{const auto bits=packet.read(32);std::int32_t value;std::memcpy(&value,&bits,4);return value;};
        if(!kind){
            HeavyFireAirRequest request;request.position=vector();request.direction=vector();request.type=integer();request.count=integer();const auto identity=integer();
            if(!(services.mode_flags()&1u)){
                request.human=services.heavy_fire_received_human(identity);request.side=2;
                if(request.type<0||request.type>=16)throw std::out_of_range("HeavyFire native received type exceeds configuration");
                auto object=services.make_heavy_fire_air(request,state_.definitions_.at(static_cast<unsigned>(request.type)));
                if(object)air_.push_back({2,std::move(object)});
            }
        }else{
            HeavyFireMortarRequest request;request.position=vector();const auto identity=integer();
            if(!(services.mode_flags()&1u)){
                request.human=services.heavy_fire_received_human(identity);request.side=2;
                auto object=services.make_heavy_fire_mortar(request);if(object)mortar_.push_back({2,std::move(object)});
            }
        }
        return true;
    }
    std::size_t air_count()const{return air_.size();}std::size_t mortar_count()const{return mortar_.size();}
    void close()noexcept{if(state_.world_==this){clear_support();state_.world_=nullptr;}}

private:
    struct Support {unsigned side;std::unique_ptr<HeavyFireSupportObject> object;};
    static unsigned checked_side(std::int32_t side){if(side<0||side>1)throw std::out_of_range("HeavyFire support side exceeds native flags");return static_cast<unsigned>(side);}
    void erase_support(std::vector<Support>& list,unsigned flag,std::size_t index){ //00472350/004723F0.
        if(flag+list[index].side>=state_.flags_.size())throw std::runtime_error("HeavyFire native received-mortar retirement writes beyond side flags");
        state_.flags_[flag+list[index].side]=0;
        list[index].object.reset(); //Native delete BEFORE swap-last and size decrement.
        if(index+1!=list.size())list[index]=std::move(list.back());
        list.pop_back();
    }
    void advance_list(std::vector<Support>& list,unsigned flag,bool authority,std::uint32_t elapsed){
        for(auto n=list.size();n;){--n;if(!list[n].object->advance(authority,elapsed))erase_support(list,flag,n);}
    }
    void clear_support()noexcept{ //Finish deletes/erases without clearing support flags; activate clears them separately.
        while(!air_.empty()){air_.back().object.reset();air_.pop_back();}
        while(!mortar_.empty()){mortar_.back().object.reset();mortar_.pop_back();}
    }
    std::vector<Support> air_,mortar_;
    void require_open()const{if(state_.world_!=this)throw std::logic_error("HeavyFireSupport world has been retired");}
    frontend::ClientModuleBinding registration_;HeavyFireLoadingState& state_;
};
} //namespace hd2::game
#endif
