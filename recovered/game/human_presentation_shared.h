#ifndef HD2_GAME_HUMAN_PRESENTATION_SHARED_H
#define HD2_GAME_HUMAN_PRESENTATION_SHARED_H

#include "game/human_presentation_textures.h"
#include <limits>

namespace hd2::game {

//Shared first/last Human portion of00785BE0/00786030 and0078A4F0/0078A5F0.
//Loading starts with empty mark/model queues. Their runtime producers remain
//absent; this owner exposes no method that fabricates marks or rendering.
template<class Frames,class Services>
class HumanPresentationShared {
public:
    struct Settings {bool winter=false;std::uint32_t terrain=0;float amount=0,scale=1;};
    HumanPresentationShared(Frames& frames,Services& services):frames_(frames),services_(services),textures_(frames) {}
    ~HumanPresentationShared(){clear();}
    HumanPresentationShared(const HumanPresentationShared&)=delete;
    HumanPresentationShared& operator=(const HumanPresentationShared&)=delete;
    void acquire() {
        if(users_==static_cast<unsigned>(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Human presentation reference overflow");
        flag_f308_=false;
        if(!users_) {
            flag_a6ac_=false;textures_.load(services_.mode_flags());
            try {
                if((services_.mode_flags()&7)!=7&&!services_.presentation_application_fallback())
                    frames_.human_presentation_driver_state(0x2b,0);
                const auto& mission=services_.presentation_mission_name();settings_=mission_settings(mission);
                if(services_.mode_flags()&2u){enabled_=true;dimensions_={{40,40}};lifetime_=500;}
            }catch(...){textures_.clear();throw;}
        }
        ++users_;
    }
    void release() noexcept {
        if(!users_)return;
        //Last-user0078A5F0 clears queues/400 words/textures before the remaining
        //Human unregister/deactivation. Loading has no runtime mark producer.
        if(!--users_){words_.fill(0);textures_.clear();}
    }
    void clear() noexcept {users_=0;words_.fill(0);textures_.clear();}
    unsigned users() const{return users_;}
    const Settings& settings() const{return settings_;}
    static Settings mission_settings(const std::string& name) {
        Settings result;
        for(const char* prefix:{"Arcti","Norwa","Prole","Arden","Co_Ar"})if(starts(name,prefix))result.winter=true;
        if(starts(name,"Bre")||starts(name,"Co_Bre")||starts(name,"Norway")||starts(name,"Sicily1")||starts(name,"Co_Sicily1"))result.terrain=1;
        else if(starts(name,"Prole")||starts(name,"Bur")||starts(name,"Co_Bu")||starts(name,"Cze")||starts(name,"Sicily2")||starts(name,"Co_Sicily2"))result.terrain=2;
        if(starts(name,"Arctic1")){result.amount=.32f;result.scale=.45f;}
        else if(starts(name,"Arctic3")){result.amount=.26f;result.scale=.45f;}
        else if(starts(name,"Africa4")){result.amount=1;result.scale=.45f;}
        return result;
    }
private:
    static bool starts(const std::string& text,const char* prefix) {
        for(std::size_t n=0;prefix[n];++n) {
            if(n>=text.size())return false;
            auto first=static_cast<unsigned char>(text[n]),second=static_cast<unsigned char>(prefix[n]);
            if(first>='A'&&first<='Z')first+=32;
            if(second>='A'&&second<='Z')second+=32;
            if(first!=second)return false;
        }
        return true; //007E8BD0, original default C-locale mission names.
    }
    Frames& frames_;Services& services_;HumanPresentationTextures<Frames> textures_;
    unsigned users_=0;Settings settings_;std::array<std::uint32_t,400> words_{};
    bool flag_f308_=false,flag_a6ac_=false,enabled_=false;
    std::array<std::uint32_t,2> dimensions_{};std::uint32_t lifetime_=0;
};
} //namespace hd2::game
#endif
