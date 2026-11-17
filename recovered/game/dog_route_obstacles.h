#ifndef HD2_GAME_DOG_ROUTE_OBSTACLES_H
#define HD2_GAME_DOG_ROUTE_OBSTACLES_H
#include "game/dog_route_geometry.h"
namespace hd2::game {
inline DogRoutePoint& dog_route_point(DogRoutePoint* point){if(!point)throw std::runtime_error("Dog route reaches native NULL point");return *point;}
//005C9FE0. Outputs remain untouched when no marked span exists. Only the FIRST
//contiguous0200 span is returned; earlier unmarked records are skipped.
inline bool dog_route_marked_span(const std::vector<DogRoutePoint*>& route,std::size_t& first,std::size_t& last){
    bool found=false;
    for(std::size_t n=0;n<route.size();++n){
        if(dog_route_required(dog_route_point(route[n]).flags24)&0x200u){if(!found){first=n;found=true;}last=n;}
        else if(found)return true;
    }
    return found;
}
//005CA030. Reverse actual process Car registry, requiring EVERY route20 index
//in the inclusive span. Each membership query reads the SAME Car loading marks.
template<class Cars,class Contains>auto dog_route_marking_car(const std::vector<DogRoutePoint*>& route,std::size_t first,std::size_t last,const Cars& cars,Contains&& contains)->typename Cars::value_type{
    for(auto n=cars.size();n;){
        auto* car=cars[--n];bool covered=true;
        for(auto index=first;index<=last;++index)if(!contains(car,dog_route_required(dog_route_point(route.at(index)).index20))){covered=false;break;}
        if(covered)return car;
    }
    return nullptr;
}
}
#endif
