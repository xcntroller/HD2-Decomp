#ifndef HD2_GAME_HEAVY_FIRE_MORTAR_H
#define HD2_GAME_HEAVY_FIRE_MORTAR_H
#include "game/heavy_fire_requests.h"
namespace hd2::game {
//004719F0/00471B30/00471B50/00471B10. Process owner retains this object;
//services borrows actual World/ray/SOUND/particle/blast endpoints. Source actor
//loans must be validated by those endpoints before dereference after callbacks.
template<class Services>class HeavyFireMortar final:public HeavyFireSupportObject {
public:
    HeavyFireMortar(const HeavyFireMortarRequest& request,Services& services)
        :services_(services),position_(request.position),human_(request.human){
        if(human_)identity_=services_.heavy_fire_actor_identity(human_);
        if((services_.mode_flags()&2u)&&human_){
            services_.heavy_fire_voice(human_,identity_,"90005600",2.f,15.f,.8999999761581421f);
            voice_delay_=services_.heavy_fire_voice_length(90005600)+1000u;
        }
    }
    bool advance(bool authority,std::uint32_t elapsed)override{
        if(authority&&(services_.mode_flags()&2u))return true; //00471B30: presentation owns advancement in combined mode.
        const auto before=signed_time(time_);time_-=static_cast<std::uint64_t>(static_cast<std::int64_t>(signed_word(elapsed)));
        if(signed_word(voice_delay_)>0){
            voice_delay_-=elapsed;
            if(signed_word(voice_delay_)<=0){
                voice_delay_=0;
                if(human_&&(services_.mode_flags()&2u))services_.heavy_fire_voice(human_,identity_,"90005602",2.f,10.f,.5f);
            }
        }
        if(signed_time(time_)<=0&&before>=0){
            if(services_.mode_flags()&2u){scatter_delay_=0;scatter_count_=20;}
            if(const auto hit=services_.heavy_fire_ground(position_)){
                auto centre=*hit;centre[1]=static_cast<float>(static_cast<long double>(centre[1])+1.f);
                services_.heavy_fire_blast(human_,identity_,centre,20.f,1000.f);
            }
        }
        if(scatter_count_&&(services_.mode_flags()&2u)){
            scatter_delay_-=elapsed;
            if(signed_word(scatter_delay_)<0){
                scatter_delay_+=services_.speech_random()%1000u;--scatter_count_;
                const float half=20.f*.5f;
                const float z=static_cast<float>(static_cast<long double>(half)-static_cast<long double>(services_.human_sound_random())*20.f);
                HumanVector location=position_;
                location[0]=static_cast<float>((static_cast<long double>(half)-static_cast<long double>(services_.human_sound_random())*20.f)+position_[0]);
                location[2]=z+position_[2];
                if(const auto hit=services_.heavy_fire_ground(location)){
                    constexpr std::array<std::uint32_t,3> effects{{43,48,49}};
                    services_.heavy_fire_particle(effects[services_.speech_random()%3u],*hit);
                    auto sound=*hit;sound[1]=static_cast<float>(static_cast<long double>(sound[1])+1.f);
                    services_.heavy_fire_scatter_sound(sound,0u);
                }
            }
        }
        return signed_time(time_)>=-7000;
    }
    std::int64_t remaining()const{return signed_time(time_);}
    std::uint32_t scatter_count()const{return scatter_count_;}
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    static std::int64_t signed_time(std::uint64_t value){std::int64_t result;std::memcpy(&result,&value,8);return result;}
    Services& services_;HumanVector position_;void* human_;
    std::int32_t identity_=-1;
    std::uint64_t time_=7000;std::uint32_t scatter_delay_=0,scatter_count_=0,voice_delay_=0;
};
} //namespace hd2::game
#endif
