#ifndef HD2_GAME_DOG_ROUTE_GEOMETRY_H
#define HD2_GAME_DOG_ROUTE_GEOMETRY_H
#include "game/dog_controller_process.h"
#include "game/dog_damage.h"
namespace hd2::game {
template<class T>const T& dog_route_required(const std::optional<T>& value){if(!value)throw std::runtime_error("Dog route requires actual point payload producer");return *value;}
inline long double dog_route_length(float square){
    std::uint32_t bits;std::memcpy(&bits,&square,4);
    return dog_signed_word(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
}
//005CB9B0: every route point is projected/clamped, including the tail. No
//Human type/radius gate or floor smoothing is substituted. Each adjusted point
//becomes the next origin. Native temporary W is unproduced on BOTH branches.
inline void smooth_dog_route(std::vector<DogRoutePoint*>& route,const std::array<float,3>& origin,const std::array<float,3>& destination){
    auto previous=origin;
    for(std::size_t n=0;n<route.size();++n){
        auto* point=route[n];if(!point)throw std::runtime_error("Dog route smoothing reaches native NULL point");
        const auto centre=point->position0.position();
        const auto next=n+1==route.size()?destination:[&]{if(!route[n+1])throw std::runtime_error("Dog route smoothing reaches native NULL next point");return route[n+1]->position0.position();}();
        std::array<long double,3> reverse{};std::array<float,3> line{},projected{},offset{};
        for(unsigned axis=0;axis!=3;++axis){reverse[axis]=static_cast<long double>(previous[axis])-next[axis];line[axis]=static_cast<float>(static_cast<long double>(next[axis])-previous[axis]);}
        const auto numerator=((static_cast<long double>(centre[2])-previous[2])*line[2]+(static_cast<long double>(centre[1])-previous[1])*line[1])+(static_cast<long double>(centre[0])-previous[0])*line[0];
        const float fraction=static_cast<float>(numerator/((reverse[2]*reverse[2]+reverse[1]*reverse[1])+reverse[0]*reverse[0]));
        for(unsigned axis=0;axis!=3;++axis){const float shift=static_cast<float>(static_cast<long double>(line[axis])*fraction);projected[axis]=static_cast<float>(static_cast<long double>(shift)+previous[axis]);offset[axis]=static_cast<float>(static_cast<long double>(projected[axis])-centre[axis]);}
        const float distance_square=static_cast<float>((static_cast<long double>(offset[2])*offset[2]+static_cast<long double>(offset[1])*offset[1])+static_cast<long double>(offset[0])*offset[0]);
        const auto radius=dog_route_required(point->radius28);
        if(dog_route_length(distance_square)>radius){ //Unordered length/radius takes projected branch, like native4100 mask.
            long double x=offset[0],y=static_cast<long double>(projected[1])-centre[1];float z=offset[2];
            const auto square=(y*y+static_cast<long double>(z)*z)+x*x;const float stored=static_cast<float>(square);
            if(!(std::abs(square-1)<9.99999993922529e-09f)){
                if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;y*=inverse;z=static_cast<float>(static_cast<long double>(z)*inverse);}
                else if(x!=0)x=x<0?-1:1;
                else if(z!=0)z=z<0?-1.f:1.f;
                else y=y<0?-1:1;
            }
            const std::array<float,3> shift{{static_cast<float>(x*radius),static_cast<float>(y*radius),static_cast<float>(static_cast<long double>(z)*radius)}};
            for(unsigned axis=0;axis!=3;++axis)projected[axis]=static_cast<float>(static_cast<long double>(shift[axis])+centre[axis]);
        }
        point->position0.xyz=projected;point->position0.w.reset();previous=projected;
    }
}
}
#endif
