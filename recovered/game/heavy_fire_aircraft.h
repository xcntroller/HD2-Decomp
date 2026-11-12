#ifndef HD2_GAME_HEAVY_FIRE_AIRCRAFT_H
#define HD2_GAME_HEAVY_FIRE_AIRCRAFT_H
#include "game/heavy_fire_requests.h"
#include "game/heavy_fire_definitions.h"
#include <cmath>
namespace hd2::game {
//00470530/00471090 and00471150/00471170. Typed owned models and SOUND
//handles replace native plane pointers/records; frame calls remain real engine
//endpoints. No aircraft Scene2 actor, alternate process or clock is introduced.
template<class Services>class HeavyFireAircraft final:public HeavyFireSupportObject {
    using Owner=typename Services::HeavyFireOwner;
    struct Plane {
        Services& services;HumanVector local{};std::optional<Owner> model;std::int32_t voice=-1;
        explicit Plane(Services& s):services(s){}
        ~Plane(){model.reset();if(voice!=-1)try{services.heavy_fire_stop_voice(voice);}catch(...){services.heavy_fire_cleanup_failure(std::current_exception());}}
        auto frame()const{return model?model->get():nullptr;}
    };
public:
    HeavyFireAircraft(const HeavyFireAirRequest& request,const HeavyFireDefinition& definition,Services& services)
        :services_(services),target_(request.position),direction_(normalize(request.direction)),definition_(definition),human_(request.human){
        try{
            if(human_)identity_=services_.heavy_fire_actor_identity(human_);
            position_=target_;
            const auto seconds=15000.L*.0010000000474974513f;
            for(unsigned a=0;a!=3;++a){
                const auto component=seconds*direction_[a];
                if(a==2)position_[a]=static_cast<float>(target_[a]-component*definition_.speed);
                else{const float distance=static_cast<float>(static_cast<long double>(static_cast<float>(component))*definition_.speed);position_[a]=target_[a]-distance;}
            }
            for(std::int32_t n=0;n<request.count;++n){
                auto plane=std::make_unique<Plane>(services_);auto* current=plane.get();planes_.push_back(std::move(plane));
                if(!(services_.mode_flags()&2u))continue;
                current->model.emplace(services_.heavy_fire_model(definition_.model));
                if(!current->frame())throw std::runtime_error("HeavyFire aircraft requires actual allocated model");
                if(n==0){
                    services_.heavy_fire_model_root(current->frame());services_.heavy_fire_model_position(current->frame(),position_,true);
                    services_.heavy_fire_model_direction(current->frame(),direction_);
                }else{
                    services_.heavy_fire_model_parent(current->frame(),planes_[static_cast<std::size_t>(n-1)]->frame());
                    const float extent=static_cast<float>(static_cast<long double>(services_.heavy_fire_model_extent(current->frame()))*1.5f);
                    current->local={{n%2?extent:extent*-2.f,0,n%2?-extent:0.f,0}};
                    services_.heavy_fire_model_position(current->frame(),current->local,true);
                    services_.heavy_fire_model_direction(current->frame(),HumanVector{{0,0,1,0}});
                }
                services_.heavy_fire_model_update(current->frame());
                HumanVector velocity{};for(unsigned a=0;a!=3;++a)velocity[a]=static_cast<float>(static_cast<long double>(direction_[a])*definition_.speed);
                current->voice=services_.heavy_fire_plane_voice(current->frame(),definition_.effect,velocity);
            }
            if((services_.mode_flags()&2u)&&human_&&services_.heavy_fire_sound_available()){
                speech_.emplace(services_.heavy_fire_air_speech(human_,identity_));
                voice_delay_=services_.heavy_fire_voice_length(90005601)+1000u;
            }
        }catch(...){clear();throw;}
    }
    ~HeavyFireAircraft()override{clear();}
    bool advance(bool authority,std::uint32_t elapsed)override{
        if(authority&&(services_.mode_flags()&2u))return true;
        const auto before=signed_time(time_);time_-=static_cast<std::uint64_t>(static_cast<std::int64_t>(signed_word(elapsed)));
        if(signed_word(voice_delay_)>0){
            voice_delay_-=elapsed;
            if(signed_word(voice_delay_)<=0){voice_delay_=0;if(human_&&(services_.mode_flags()&2u)&&speech_)services_.heavy_fire_air_speech_reply(speech_->get());}
        }else if(speech_&&!services_.heavy_fire_sound_playing(speech_->get())){services_.heavy_fire_sound_disable(speech_->get());speech_.reset();}
        const auto seconds=static_cast<long double>(signed_time(time_))*.0010000000474974513f;
        const auto x=seconds*direction_[0]*definition_.speed;
        const auto y=static_cast<long double>(static_cast<float>(seconds*direction_[1]))*definition_.speed;
        const float z=static_cast<float>(static_cast<long double>(static_cast<float>(seconds*direction_[2]))*definition_.speed);
        position_[0]=static_cast<float>(target_[0]-x);position_[1]=static_cast<float>(target_[1]-y);position_[2]=target_[2]-z;
        if(planes_.empty())throw std::runtime_error("HeavyFire native frame dereferences first plane of empty request");
        if(planes_[0]->frame()){
            services_.heavy_fire_model_position(planes_[0]->frame(),position_);services_.heavy_fire_model_update(planes_[0]->frame());
            for(std::size_t n=1;n+1<planes_.size();++n){ //Native leaves LAST member's local pose unchanged.
                auto& plane=*planes_[n];if(!plane.frame())continue;
                const auto time=static_cast<long double>(signed_time(time_));HumanVector p=plane.local;
                p[0]=static_cast<float>(std::sin(.0007999999797903001f*time+static_cast<long double>(n*3))+plane.local[0]);
                p[1]=static_cast<float>(std::cos(time*.0010000000474974513f+n))+plane.local[1];
                p[2]=static_cast<float>(std::sin(time*.000699999975040555f+static_cast<long double>(n)*2.5f))+plane.local[2];
                services_.heavy_fire_model_position(plane.frame(),p);services_.heavy_fire_model_update(plane.frame());
            }
        }
        if(signed_time(time_)<=0&&before>=0){
            if(services_.mode_flags()&2u){scatter_delay_=0;scatter_count_=static_cast<std::uint32_t>(definition_.count)*static_cast<std::uint32_t>(planes_.size());}
            if(services_.mode_flags()&1u)if(const auto hit=services_.heavy_fire_ground(target_)){
                auto centre=*hit;centre[1]=static_cast<float>(static_cast<long double>(centre[1])+1.f);
                services_.heavy_fire_blast(human_,identity_,centre,definition_.radius,definition_.power);
            }
        }
        if(scatter_count_&&(services_.mode_flags()&2u)){
            scatter_delay_-=elapsed;
            if(signed_word(scatter_delay_)<0){
                scatter_delay_+=services_.speech_random()%300u;--scatter_count_;
                const float half=definition_.radius*.5f;
                const auto z_draw=services_.human_sound_random();
                const auto x_draw=services_.human_sound_random();HumanVector location=target_;
                location[0]=static_cast<float>((static_cast<long double>(half)-static_cast<long double>(x_draw)*definition_.radius)+target_[0]);
                const float offset=static_cast<float>(static_cast<long double>(half)-static_cast<long double>(z_draw)*definition_.radius);location[2]=offset+target_[2];
                if(const auto hit=services_.heavy_fire_ground(location)){
                    constexpr std::array<std::uint32_t,3> effects{{43,48,49}};
                    services_.heavy_fire_particle(effects[services_.speech_random()%3u],*hit);
                    auto sound=*hit;sound[1]=static_cast<float>(static_cast<long double>(sound[1])+1.f);services_.heavy_fire_scatter_sound(sound,1u);
                }
            }
        }
        return signed_time(time_)>=-15000;
    }
private:
    static HumanVector normalize(HumanVector value){
        const auto extended=(static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0];
        const float square=static_cast<float>(extended);
        if(!std::isfinite(extended)||!std::isfinite(square))throw std::runtime_error("HeavyFire nonfinite native direction");
        constexpr float epsilon=9.99999993922529e-9f;
        if(std::abs(extended-1.f)>=epsilon){
            if(square>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(value[a]*scale);}
            else{const unsigned axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    static std::int64_t signed_time(std::uint64_t value){std::int64_t result;std::memcpy(&result,&value,8);return result;}
    void clear()noexcept{while(!planes_.empty())planes_.pop_back();speech_.reset();}
    Services& services_;HumanVector target_,direction_,position_{};HeavyFireDefinition definition_;void* human_;std::int32_t identity_=-1;
    std::vector<std::unique_ptr<Plane>> planes_;std::optional<Owner> speech_;
    std::uint64_t time_=15000;std::uint32_t voice_delay_=0,scatter_delay_=0,scatter_count_=0;
};
} //namespace hd2::game
#endif
