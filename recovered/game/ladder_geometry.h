#ifndef HD2_GAME_LADDER_GEOMETRY_H
#define HD2_GAME_LADDER_GEOMETRY_H

#include "game/car_geometry_math.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>

namespace hd2::game {

//Geometry portion of005EA5D0, before its final00610200 initialization. Borrowed
//markers remain owned by the actual model; the containing actor retains it.
//Material parsing follows lowercasing, first substring "matid", then sscanf%d.
template<class Frames>
class LadderGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    LadderGeometry(Frames& frames,Frame model) {
        if(!model)throw std::runtime_error("Ladder initialization requires a frame");
        const auto up=frames.actor_find(model,"up*",0x10020);
        const auto down=frames.actor_find(model,"down*",0x10020);
        auto visual=frames.actor_find(model,"*",0x10001);
        if(!visual)visual=model;
        if(!up)throw std::runtime_error("Ladder requires an up marker");
        if(!down)throw std::runtime_error("Ladder requires a down marker");
        const auto up_world=frames.actor_world_matrix(up);
        const auto down_world=frames.actor_world_matrix(down);
        for(std::size_t i=0;i!=4;++i){up_position_[i]=up_world[12+i];down_position_[i]=down_world[12+i];}
        const auto world=frames.actor_world_matrix(model);
        Vector forward{};for(std::size_t i=0;i!=4;++i)forward[i]=world[8+i];
        forward=car_normalize(forward);forward[1]=0;direction_=car_normalize(forward);
        auto text=frames.actor_user_data(visual);
        for(auto& c:text)if(c>='A'&&c<='Z')c=static_cast<char>(c+('a'-'A'));
        const auto key=text.find("matid");
        if(key!=std::string::npos) {
            const auto* start=text.c_str()+key+5;char* end=nullptr;errno=0;
            const auto number=std::strtoll(start,&end,10);
            if(start!=end) {
                if(errno==ERANGE||number<std::numeric_limits<std::int32_t>::min()||number>std::numeric_limits<std::int32_t>::max())
                    throw std::runtime_error("Ladder material number exceeds int32 bounds");
                material_=static_cast<std::int32_t>(number);
            }
        }
    }
    const Vector& up_position() const {return up_position_;}
    const Vector& down_position() const {return down_position_;}
    const Vector& direction() const {return direction_;}
    std::int32_t material() const {return material_;}
private:
    Vector up_position_{},down_position_{},direction_{};
    std::int32_t material_=25;
};

} //namespace hd2::game
#endif
