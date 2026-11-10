#ifndef HD2_GAME_CAR_PARTICLE_POOL_H
#define HD2_GAME_CAR_PARTICLE_POOL_H

#include "game/car_particles.h"
#include "frontend/cleanup.h"

namespace hd2::game {

//004D5A50/004ED0C0/004ECFF0: retained actual EMTR frames and their native
//48-byte-equivalent pool states. Construction does not activate emitters or
//publish a simulation world. Services supplies real texture/particle endpoints.
template<class Frames,class Services>
class CarParticlePool {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_car_particle_frame());
    CarParticlePool(Frames& frames,Services& services) : frames_(frames),services_(services) {}
    ~CarParticlePool() {
        try {clear();}catch(...) {frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    CarParticlePool(const CarParticlePool&)=delete;
    CarParticlePool& operator=(const CarParticlePool&)=delete;
    bool initialize(const std::vector<CarParticleDefinition>& definitions) {
        clear();
        if(definitions.empty()) {
            CarParticleState<Frame> state;state.definition=0xffffffffu;
            states_.push_back(state);return false; //Original real sentinel, no visual.
        }
        states_.reserve(30);owners_.reserve(30);
        for(unsigned count=0;count!=30;++count) {
            auto visual=frames_.create_car_particle_frame();
            if(!visual)throw std::runtime_error("Car emitter factory returned null");
            CarParticleState<Frame> state;state.visual=visual.get();
            bool loaded=false;
            try {
                for(std::uint32_t index=1;index<definitions.size();++index)
                    if(initialize_car_particle(services_,state,definitions,index)) {loaded=true;break;}
            }catch(...) {
                try {frames_.link_scene_object(state.visual,nullptr);}
                catch(...) {frames_.record_actor_cleanup_failure(std::current_exception());}
                throw;
            }
            if(!loaded)break; //Local visual releases; original still returns1.
            state.active=false;state.pending=false;
            owners_.push_back(std::move(visual));states_.push_back(state);
        }
        return true;
    }
    void update(std::uint32_t elapsed){ //004ED130, reverse states, engine tick BEFORE pending/count checks.
        for(std::size_t n=states_.size();n;){--n;
            if(!states_.at(n).active)continue;
            auto frame=states_.at(n).visual;if(!frame)throw std::logic_error("Active Car emitter has no actual retained visual");
            frames_.advance_car_particle(frame,elapsed);
            if(!states_.at(n).pending)continue; //Reread actual state AFTER callback.
            frame=states_.at(n).visual;frames_.activate_car_particle(frame,false);
            frame=states_.at(n).visual;
            if(frames_.car_particle_live_count(frame)<1){
                states_.at(n).pending=states_.at(n).active=false;
                frames_.link_scene_object(states_.at(n).visual,nullptr);
            }
        }
    }
    void clear() {
        frontend::CleanupErrors errors;
        for(auto n=owners_.size();n!=0;) {
            auto& owner=owners_[--n];
            if(owner)errors.attempt([&] {frames_.link_scene_object(owner.get(),nullptr);});
            owner.reset();
        }
        states_.clear();owners_.clear();errors.rethrow();
    }
    std::vector<CarParticleState<Frame>>& states() {return states_;}
    const std::vector<CarParticleState<Frame>>& states() const {return states_;}
private:
    Frames& frames_;
    Services& services_;
    std::vector<Owner> owners_;
    std::vector<CarParticleState<Frame>> states_;
};

} //namespace hd2::game
#endif
