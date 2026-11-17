#ifndef HD2_GAME_DOG_CONTROLLER_VECTOR_H
#define HD2_GAME_DOG_CONTROLLER_VECTOR_H
#include <array>
#include <optional>
#include <stdexcept>
namespace hd2::game {
//Unwritten components retain provenance across native four-word copies.
struct DogControllerVector {
    std::optional<std::array<float,3>> xyz;std::optional<float> w;
    void assign(const std::array<float,4>& value){xyz=std::array<float,3>{{value[0],value[1],value[2]}};w=value[3];}
    const std::array<float,3>& position()const{if(!xyz)throw std::runtime_error("Dog controller requires actual vector XYZ producer");return *xyz;}
    std::array<float,4> full()const{const auto& v=position();if(!w)throw std::runtime_error("Dog controller requires actual vector W producer");return {{v[0],v[1],v[2],*w}};}
};
}
#endif
