#ifndef HD2_GAME_CAR_STRIP_CONNECTIONS_H
#define HD2_GAME_CAR_STRIP_CONNECTIONS_H

#include "game/car_wheel_geometry.h"

namespace hd2::game {

//004E2E30 at004E59C2–004E618B. Each strip visual is made private before its
//actual LOD0 vertices are locked. These records feed004ED270's wheel flag100.
template<class Frames>
class CarStripConnections {
public:
    using Frame=typename Frames::Frame;
    using Wheel=typename CarWheelGeometry<Frames>::Wheel;
    struct Connection {
        Frame visual{};
        std::uint16_t wheel=0;
        std::vector<std::uint16_t> vertices;
        std::vector<float> offsets;
    };
    void discover(Frames& frames,Frame model,std::vector<Wheel>& wheels) {
        if(wheels.size()>24)throw std::runtime_error("Car strip wheel index overflow");
        for(std::size_t side=0;side!=2;++side) {
            auto& connections=side?right_:left_;
            for(auto visual:frames.car_parts(model,side?"STRIPR*":"STRIPL*",0x10001)) {
                frames.prepare_car_strip_mesh(visual);
                for(auto index=wheels.size();index; ) {
                    --index;if(index%2!=side)continue;
                    const auto& wheel=wheels[index];
                    Connection record;record.visual=visual;record.wheel=static_cast<std::uint16_t>(index);
                    float maximum_y=-1000,adjustment=0;
                    frames.car_strip_vertices(visual,[&](std::size_t count,auto&& vertex) {
                        if(count>0x10000)throw std::runtime_error("Car strip vertex index overflow");
                        for(auto n=count;n; ) {
                            --n;
                            const auto point=frames.car_transform(vertex(n),frames.car_local_matrix(visual));
                            //00810900 is a double. The subtraction is rounded
                            //to float before comparison in the original body.
                            if(std::abs(point[2]-wheel.position[2])<.05000000074505806&&point[1]<wheel.position[1]) {
                                if(maximum_y<point[1]) {
                                    maximum_y=point[1];adjustment=(-wheel.radius+wheel.position[1])-point[1];
                                }
                                record.offsets.push_back(point[1]-wheel.position[1]);
                                record.vertices.push_back(static_cast<std::uint16_t>(n));
                            }
                        }
                    });
                    if(record.vertices.empty())continue;
                    for(auto& offset:record.offsets)offset+=adjustment;
                    connections.push_back(std::move(record));wheels[index].strip_offset+=.10000000149011612f;
                }
            }
        }
    }
    const std::vector<Connection>& left() const {return left_;}
    const std::vector<Connection>& right() const {return right_;}
    std::vector<std::uint16_t> wheel_indices(bool right) const {
        std::vector<std::uint16_t> result;
        for(const auto& record:right?right_:left_)result.push_back(record.wheel);
        return result;
    }
private:
    std::vector<Connection> left_,right_;
};

} //namespace hd2::game
#endif
