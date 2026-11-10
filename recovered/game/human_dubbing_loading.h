#ifndef HD2_GAME_HUMAN_DUBBING_LOADING_H
#define HD2_GAME_HUMAN_DUBBING_LOADING_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hd2::game {

//Actual00436470/0071DF30 world consumers for the SAME process DubbingSound.
//Human/Players and MissionSubtitlesLoading outlive synchronous playback and
//all channels; retire channels before Human models/attachments/scene disappear.
//No duplicate channel, renderer, attachment or current-player state is owned.
template<class Human,class Players,class Subtitles>class HumanDubbingLoading {
public:
    HumanDubbingLoading(Players& players,Subtitles& subtitles):players_(players),subtitles_(subtitles){}
    auto dubbing_actor_model(void* actor){return actual(actor).base().frame();}
    std::array<void*,2> dubbing_actor_attachments(void* actor){
        auto& human=actual(actor);return {{human.field2b8,human.field2bc}}; //Native+24 first, +20 second.
    }
    Human* dubbing_current_player(){return players_.current_player();} //SAME0045D1D0/2C/override58.
    void dubbing_subtitle(std::int32_t identity,std::uint32_t duration){subtitles_.speech(static_cast<std::uint32_t>(identity),duration);} //0048F180, not auto-show/select.
private:
    static Human& actual(void* actor){
        if(!actor)throw std::runtime_error("Dubbing Human lookup requires its actual source identity");
        using Actor=typename std::decay_t<decltype(std::declval<Human&>().perception.lists()[0])>::value_type;
        auto* human=dynamic_cast<Human*>(static_cast<Actor>(actor));
        if(!human||human->base().type()!=3)throw std::runtime_error("Dubbing received another actor family");
        return *human;
    }
    Players& players_;Subtitles& subtitles_;
};
} //namespace hd2::game
#endif
