#ifndef HD2_GAME_EFFECTS_LOADING_UPDATE_H
#define HD2_GAME_EFFECTS_LOADING_UPDATE_H
#include <array>
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//00487240's source loading update composition. All arguments borrow the actual
//module/process owners; positive water/weather/grass/colour receivers still
//reject before success. Empty queues are actual absent producers, not defaults.
template<class Frames,class Process,class Effects,class Birds,class Movie,class Session,class Players,class Startup>
void update_loading_effects(std::uint32_t elapsed,Frames& frames,Process& process,Effects& effects,Birds& birds,Movie& movie,
                            Session& session,Players& players,Startup& startup){
        //304 pose and210 fade-in producers remain separate.
        effects.special_model().update(elapsed); //00487416 BEFORE fade/time224.
        process.fade.update(elapsed);process.field224+=elapsed;
        if((session.status==1||session.status==5||session.status==6)&&process.water.enabled()&&players.current_player())
            throw std::runtime_error("GEffects water camera update requires actual00487240 retained-camera transitions");
        effects.frame_fades().update(elapsed);effects.human_fades().update(elapsed);
        if(process.sun1b0){
            const auto camera=frames.active_game_camera();
            if(camera){
                const auto world=frames.actor_world_matrix(camera);std::array<float,3> position{};
                for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(world[12+axis])+process.sun_position10[axis]);
                frames.object_position(process.sun1b0,position);frames.update_scene_object(process.sun1b0);
            }
        }
        birds.update(elapsed);
        bool weather_allowed=true;
        if(session.words.template get<std::uint8_t>(0x64)){
            if(session.status==1||session.status==5||session.status==6)
                throw std::runtime_error("GEffects game64 update requires actual HUD004B6180 weather gate");
            weather_allowed=false;
        }
        if(weather_allowed&&process.flag24c)
            throw std::runtime_error("GEffects weather requires actual rain/particle/collision producers");
        if(process.active248)process.decorations.update_loading(startup.profile_settings().quality_byte(4)!=0);
        if(process.colour2f4[0]!=process.colour2f4[1])
            throw std::runtime_error("GEffects colour interpolation requires actual004884A6 scalar producer/update");
        movie.update();

}
} //namespace hd2::game
#endif
