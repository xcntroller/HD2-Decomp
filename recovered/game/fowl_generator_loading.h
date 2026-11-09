#ifndef HD2_GAME_FOWL_GENERATOR_LOADING_H
#define HD2_GAME_FOWL_GENERATOR_LOADING_H
#include "game/simple_scene_actors.h"
#include "game/fowl_bird.h"
#include "game/prepared_spawn_packets.h"
#include "frontend/spatial_sound.h"
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>
namespace hd2::game {

//005BA670/005BA8B0/005BB140/005BB290/005BB3C0/005BB4D0.
//Actual generated mode1/2 birds; borrowed mode0 requires its shared native pool
//and is explicitly rejected. No common ActorBase activation/deactivation calls:
//these original derived receivers do not make them. Services is the SAME world.
template<class Frames,class Services>class FowlGeneratorLoading final:public FowlLoadingLifetime {
public:
    using Actor=FowlGeneratorSceneActor<Frames>;using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Bird=FowlBird<Frames,Services>;
    FowlGeneratorLoading(Frames& frames,Services& services,Actor& actor):frames_(frames),services_(services),actor_(actor){}
    ~FowlGeneratorLoading()override {try{deactivate(true);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    std::size_t bird_count()const override{return birds_.size();}
    const Bird& bird(std::size_t index)const{return *birds_.at(index);}
    void activate(bool authority)override { //005BA670.
        if(!authority&&(services_.mode_flags()&1u))return;
        if(partial_)throw std::logic_error("Retire partial Fowl activation before retry");
        partial_=true;elapsed_c0_=0;
        if(actor_.properties.fielda0==0)throw std::runtime_error("Fowl mode0 requires the actual shared borrowed-bird pool");
        if(actor_.properties.fielda0==2)create_birds();
        if(authority) {
            if(actor_.fielda4!=0xffffffffu)throw std::logic_error("Retire the actual Fowl sound listener before authority reactivation");
            actor_.fielda4=static_cast<std::uint32_t>(services_.add_fowl_listener(actor_.base().frame()));
        }
        timer_b8_=static_cast<std::uint32_t>(fowl_random_word(services_)%1000);launched_bc_=false;partial_=false;
    }
    void deactivate(bool authority)override { //005BB290, generated flag60=true means actual scene remove.
        frontend::CleanupErrors errors;
        while(!birds_.empty()){auto bird=std::move(birds_.back());birds_.pop_back();errors.attempt([&]{bird->close();});}
        partial_=false;
        if(authority&&actor_.fielda4!=0xffffffffu) {
            const auto slot=fowl_signed_word(actor_.fielda4);
            errors.attempt([&]{services_.remove_fowl_listener(slot);actor_.fielda4=0xffffffffu;});
        }
        errors.rethrow();
    }
    void heard(std::uint32_t group,float volume)override { //005BA860: receiver/frame/EC is resolved by actual host callback.
        if((group==0||group==2)&&volume>.800000011920929f)launch(true,0);
    }
    void launch(bool notify,std::int32_t event_value)override { //005BB140.
        if(launched_bc_)return;
        if(actor_.properties.fielda0==1)create_birds();
        for(auto n=birds_.size();n;)birds_[--n]->launch();
        if((services_.mode_flags()&2u)&&event_value<1000) {
            const auto world=frames_.actor_world_matrix(actor_.base().frame());frontend::SpatialSoundRequest request;
            request.position={{world[12],world[13],world[14],world[15]}};request.flags=1;request.group=11;request.entry=birds_.size()>3?1:0;
            services_.emit_fowl_sound(request);
        }
        launched_bc_=true;elapsed_c0_=0;
        if(notify) {
            auto& packet=services_.session_broadcast_packets().begin(13,0);
            packet.write(static_cast<std::uint32_t>(actor_.base().id10()),10);packet.finish_broadcast(); //Genuine payload-free actor notification.
        }
    }
    void update_presentation(std::uint32_t elapsed)override { //005BB3C0, backward loop and swap/pop.
        if(birds_.empty())return;
        for(auto n=birds_.size();n;) {
            --n;if(birds_[n]->update(elapsed))continue;
            auto old=std::move(birds_[n]);if(n+1!=birds_.size())birds_[n]=std::move(birds_.back());birds_.pop_back();old->close();
        }
        if(launched_bc_){elapsed_c0_+=elapsed;if(birds_.empty())elapsed_c0_+=10000;}
    }
    void update_authority(std::uint32_t elapsed)override { //005BB4D0: real Scene2 identity slots, then actual active camera.
        timer_b8_-=elapsed;if(launched_bc_||fowl_signed_word(timer_b8_)>0)return;
        //Equivalent native repeated +1000, bounded arithmetic instead of billions of iterations.
        const auto signed_timer=fowl_signed_word(timer_b8_);const auto steps=(-static_cast<std::int64_t>(signed_timer))/1000+1;
        timer_b8_+=static_cast<std::uint32_t>(steps*1000);
        const auto world=frames_.actor_world_matrix(actor_.base().frame());const Vector origin{{world[12],world[13],world[14],world[15]}};
        const auto radius=real(actor_.properties.fieldac);
        for(unsigned i=0;i!=32;++i) {
            const auto* actor=services_.fowl_scene_actor(i);if(!actor)break;
            const auto position=frames_.actor_world_matrix(actor->base().frame());
            Vector delta{{stored(static_cast<long double>(position[12])-origin[0]),stored(static_cast<long double>(position[13])-origin[1]),stored(static_cast<long double>(position[14])-origin[2]),0}};
            const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
            const auto bits=word(stored(square));long double distance;
            if(fowl_signed_word(bits)<0x322bcc77)distance=0;
            else if(std::abs(square-1)>=9.99999993922529e-09f)distance=std::sqrt(square);else distance=1;
            if(distance<radius){launch(true,0);break;}
        }
        for(auto n=services_.fowl_scene_actor_count();n>32;) {
            const auto* actor=services_.fowl_scene_actor(--n);if(!actor)break;
            if(actor->base().type()!=3||!services_.fowl_human_proximity_eligible(*actor))continue;
            const auto matrix=frames_.actor_world_matrix(actor->base().frame());
            if(distance_square({{matrix[12],matrix[13],matrix[14],matrix[15]}},origin)<static_cast<long double>(radius)*radius){launch(true,0);break;}
        }
        if(distance_square(services_.fowl_camera_position(),origin)<static_cast<long double>(radius)*radius)launch(true,0);
    }
private:
    static float real(std::uint32_t bits){float result;std::memcpy(&result,&bits,4);return result;}
    static std::uint32_t word(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
    static float stored(long double value){return static_cast<float>(value);}
    static long double distance_square(const Vector& p,const Vector& q) {
        const auto x=static_cast<long double>(p[0])-q[0],y=static_cast<long double>(p[1])-q[1],z=static_cast<long double>(p[2])-q[2];return (x*x+y*y)+z*z;
    }
    void create_birds() { //005BA8B0: floor(sqrt(count)) squared, descending grid order.
        if(actor_.properties.fielda0==0)return;
        const auto signed_count=fowl_signed_word(actor_.properties.fieldb0);const auto root=stored(std::sqrt(static_cast<long double>(signed_count)));
        const auto radius=real(actor_.properties.fielda8);
        if(!std::isfinite(root)||root<0||root>1024||!std::isfinite(radius))throw std::runtime_error("Fowl grid exceeds bounded native count/radius");
        const auto step=stored((static_cast<long double>(radius)+radius)/root);
        const auto world=frames_.actor_world_matrix(actor_.base().frame());auto model=frames_.create_fowl_model();
        if(!model)throw std::runtime_error("Cannot create required Fowl template model");
        if(frames_.load_fowl_template(model.get(),actor_.properties.name)!=0)throw std::runtime_error("Cannot load required Fowl template model: "+actor_.properties.name);
        const auto side=static_cast<std::uint32_t>(root);if(!side)return;
        const auto jitter=stored(static_cast<long double>(step)*.4000000059604645f);const auto spread=stored(static_cast<long double>(step)*.4000000059604645f*2);
        for(auto ix=side;ix;) {
            --ix;const auto x=stored(static_cast<long double>(ix)*step+world[12]-radius);
            for(auto iz=side;iz;) {
                --iz;Vector position{{x,world[13],stored(static_cast<long double>(iz)*step+world[14]-radius),0}};
                position[0]=stored(fowl_random(services_,spread)-jitter+position[0]);position[2]=stored(fowl_random(services_,spread)-jitter+position[2]);
                auto frame=frames_.create_fowl_model();if(!frame)throw std::runtime_error("Cannot create required generated Fowl model");
                frames_.clone_fowl_model(frame.get(),model.get());
                bool added=false;
                try {
                    //Set before the call so a callback failure after native publication still retires membership.
                    added=true;frames_.add_fowl_model(frame.get());frames_.link_scene_object(frame.get(),frames_.camera_root_sector());
                    if(actor_.properties.fielda0==2)if(auto hit=services_.fowl_ground(position))position=*hit;
                    frames_.object_visible(frame.get(),true);frames_.object_position(frame.get(),{{position[0],position[1],position[2]}});
                    const auto dx=stored(fowl_random(services_,1.f)),dz=stored(fowl_random(services_,1.f));frames_.car_direction(frame.get(),{{dx,0,dz,0}});
                    //5BB023 stores a DOUBLE argument although the actual format
                    //uses %d: preserve its low DWORD, rather than invent a grid ID.
                    const double row=static_cast<double>(static_cast<long double>(static_cast<float>(ix))*root);
                    std::uint64_t row_bits;std::memcpy(&row_bits,&row,sizeof(row));
                    char name[80];const auto index=fowl_signed_word(static_cast<std::uint32_t>(row_bits));
                    const auto length=std::snprintf(name,sizeof(name),"_bird_%d_%d",actor_.base().id10(),index);
                    if(length<0||static_cast<std::size_t>(length)>=sizeof(name))throw std::runtime_error("Fowl generated name overflow");
                    frames_.fowl_name(frame.get(),name);frames_.update_scene_object(frame.get());
                    //The Bird constructor owns rollback of scene membership from this point.
                    auto bird=std::make_unique<Bird>(frames_,services_,frame.get());added=false;birds_.push_back(std::move(bird));
                }catch(...){if(added)try{frames_.remove_model(frame.get());}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}throw;}
            }
        }
    }
    Frames& frames_;Services& services_;Actor& actor_;std::vector<std::unique_ptr<Bird>> birds_;
    std::uint32_t timer_b8_=0,elapsed_c0_=0;bool launched_bc_=false,partial_=false;
};
} //namespace hd2::game
#endif
