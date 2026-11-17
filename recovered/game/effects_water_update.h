#ifndef HD2_GAME_EFFECTS_WATER_UPDATE_H
#define HD2_GAME_EFFECTS_WATER_UPDATE_H
#include <stdexcept>
namespace hd2::game {
//00487240 after224 clock increment and initial status/water/player gates.
//World provides SAME Human inline98/DD0.007860B0's AL result is ignored;
//its explicit below flag and possibly unwritten height output control this path.
//Retained-camera transitions remain required at their actual positive boundary.
template<class Process,class Frames,class Players,class World>
void update_loading_water_camera(Process& process,Frames& frames,Players& players,World& world){
    if(process.water.underwater()&&process.water.camera_wobble_enabled())
        throw std::runtime_error("GEffects underwater camera oscillation requires actual00487240 current-camera receiver");
    auto* human=players.current_player(); //Fresh query AFTER the optional oscillation stage.
    if(!human)throw std::logic_error("Water camera update lost its actual selected Human");
    bool below=false;
    (void)world.human_inline(*human).query_obstruction_height(frames,below,process.water_query_height);
    if(below||(human->field160&&human->field160->base().type()==20)||process.water.underwater())
        throw std::runtime_error("GEffects water camera update requires actual00487240 retained-camera transitions");
    //Original skips the complete transition body here. No missing camera,
    //dummy surface, disabled water setting or successful reflection substitute.
}
}
#endif
