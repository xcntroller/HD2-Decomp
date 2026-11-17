#ifndef HD2_GAME_DOG_ROUTE_RECTANGLE_H
#define HD2_GAME_DOG_ROUTE_RECTANGLE_H
#include "game/dog_route_obstacles.h"
#include "game/dog_controller_targets.h"
#include <cmath>
#include <limits>
namespace hd2::game {
using DogRoutePlanar=std::array<float,2>;
struct DogRouteRectangle {DogRoutePlanar minimum{},maximum{};};
struct DogRouteCrossings {unsigned count=0;DogRoutePlanar first{},second{};std::uint32_t first_side=0,second_side=0;};
inline bool dog_route_denominator(float value){const float absolute=std::abs(value);std::int32_t bits;std::memcpy(&bits,&absolute,4);return bits>=0x322bcc77;}
//005CA0C0. Wall order is west/east/bottom/top, independent of segment direction.
//Strict endpoint/corner exclusion, FLOAT crossing stores and native .3 offsets.
inline DogRouteCrossings intersect_dog_route_rectangle(const DogRouteRectangle& rectangle,const DogRoutePlanar& from,const DogRoutePlanar& to){
    DogRouteCrossings out;
    float dx=static_cast<float>(static_cast<long double>(to[0])-from[0]);
    const auto extended_z=static_cast<long double>(to[1])-from[1];
    float dz=static_cast<float>(extended_z);
    const auto square=extended_z*dz+static_cast<long double>(dx)*dx;const float stored_square=static_cast<float>(square);
    if(square==0)return out;
    const auto length=std::sqrt(square);
    if(length>9.99999993922529e-09f){const auto inverse=1.L/length;dx=static_cast<float>(dx*inverse);dz=static_cast<float>(dz*inverse);}
    const auto in_segment=[&](float x,float z,bool top){
        const auto a=static_cast<long double>(x)-from[0],b=static_cast<long double>(z)-from[1];
        if(!(b*b+a*a<stored_square))return false;
        if(top){ //00562DE0 stores the last endpoint's two deltas before00562E00.
            const float c=static_cast<float>(static_cast<long double>(x)-to[0]),d=static_cast<float>(static_cast<long double>(z)-to[1]);
            return static_cast<long double>(c)*c+static_cast<long double>(d)*d<stored_square;
        }
        const auto c=static_cast<long double>(x)-to[0],d=static_cast<long double>(z)-to[1];
        return d*d+c*c<stored_square;
    };
    const auto publish=[&](DogRoutePlanar point,std::uint32_t side){
        if(!out.count){out.first=point;out.first_side=side;}else{out.second=point;out.second_side=side;}++out.count;
    };
    const float vertical=static_cast<float>(static_cast<long double>(dz)*0.f-dx);
    if(dog_route_denominator(vertical)){
        for(unsigned wall=0;wall!=2;++wall){
            const auto edge=wall?rectangle.maximum[0]:rectangle.minimum[0];
            const auto crossed=(-static_cast<long double>(from[1])*dx-(static_cast<long double>(edge)-from[0])*dz)/vertical;
            const float x=static_cast<float>(0.L*crossed+edge),z=static_cast<float>(crossed);
            if(crossed>rectangle.minimum[1]&&z<rectangle.maximum[1]&&in_segment(x,z,false))
                publish({{static_cast<float>(static_cast<long double>(x)+(wall?.30000001192092896f:-.30000001192092896f)),z}},wall?3u:7u);
        }
    }
    const float horizontal=static_cast<float>(static_cast<long double>(dz)-static_cast<long double>(dx)*0.f);
    if(dog_route_denominator(horizontal)){
        for(unsigned wall=0;wall!=2;++wall){
            const auto edge=wall?rectangle.maximum[1]:rectangle.minimum[1];
            const auto crossed=((static_cast<long double>(edge)-from[1])*dx-(-static_cast<long double>(from[0])*dz))/horizontal;
            const float x=static_cast<float>(crossed),z=static_cast<float>(crossed*0.L+edge);
            if(x>rectangle.minimum[0]&&x<rectangle.maximum[0]&&in_segment(x,z,wall!=0))
                publish({{x,static_cast<float>(static_cast<long double>(z)+(wall?.30000001192092896f:-.30000001192092896f))}},wall?5u:1u);
        }
    }
    return out;
}
//005C93B0. Native forward7 ->1 publishes corner4, not0. Keep that value;
//005C95B0's read of the unproduced fifth slot must remain an explicit failure.
inline std::vector<std::uint32_t> dog_route_corner_indices(std::uint32_t from,std::uint32_t to,bool reverse){
    if(from<1||from>7||!(from&1u)||to<1||to>7||!(to&1u))throw std::runtime_error("Dog detour requires actual odd wall identifiers");
    std::vector<std::uint32_t> corners;
    while(from!=to){
        if(reverse){corners.push_back((from-1)/2);from-=2;if(dog_signed_word(from)<1)from=7;}
        else{corners.push_back((from+1)/2);from+=2;if(from>8)from=1;}
    }
    return corners;
}
//005C9080. Full affine position is retained while selecting the nearest side;
//ties favor the higher side because the native search runs3,2,1,0.
template<class Queries>bool escape_dog_route_rectangle(std::vector<DogRoutePoint*>& route,const DogRouteRectangle& rectangle,Queries& q){
    auto position=q.to_local(q.model_position());
    if(!(position[0]>rectangle.minimum[0]&&position[0]<rectangle.maximum[0]&&position[2]>rectangle.minimum[1]&&position[2]<rectangle.maximum[1]))return false;
    if(route.empty())return true;
    if(dog_route_required(dog_route_point(route[0]).flags24)&0x1000u)return false;
    const std::array<float,4> distances{{
        static_cast<float>(static_cast<long double>(position[0])-rectangle.minimum[0]),
        static_cast<float>(static_cast<long double>(rectangle.maximum[0])-position[0]),
        static_cast<float>(static_cast<long double>(position[2])-rectangle.minimum[1]),
        static_cast<float>(static_cast<long double>(rectangle.maximum[1])-position[2])}};
    unsigned side=0;float nearest=std::numeric_limits<float>::max();
    for(unsigned n=4;n;){--n;if(distances[n]<nearest){side=n;nearest=distances[n];}}
    const auto clearance=static_cast<long double>(.30000001192092896f)*10.f;
    if(side==1)position[0]=static_cast<float>(clearance+rectangle.maximum[0]);
    else if(side==2)position[2]=static_cast<float>(rectangle.minimum[1]-clearance);
    else if(side==3)position[2]=static_cast<float>(clearance+rectangle.maximum[1]);
    else position[0]=static_cast<float>(rectangle.minimum[0]-clearance);
    const auto world=q.to_world(position);DogControllerVector result;result.xyz=std::array<float,3>{{world[0],world[1],world[2]}};
    auto& point=q.acquire(result);point.flags24=0x1000u;route.insert(route.begin(),&point);return true;
}
//005C95B0. Both direction metrics intentionally use FIRST hit for both ends;
//the second hit argument is native-unused. Only four corner payloads exist.
//No corner4 wrap or inferred fifth payload is supplied here.
template<class Queries>bool rewrite_dog_route_rectangle(std::vector<DogRoutePoint*>& route,unsigned count,
        const std::array<float,4>& first_hit,std::int32_t first_boundary,std::int32_t last_boundary,
        std::uint32_t first_side,std::uint32_t last_side,const DogRouteRectangle& rectangle,Queries& q){
    if(!count)return true;
    if(count==1){
        const auto kept=static_cast<std::uint32_t>(first_boundary)+1u;
        while(route.size()>kept){q.release(route.back());route.pop_back();}
        DogControllerVector position;position.xyz=std::array<float,3>{{first_hit[0],first_hit[1],first_hit[2]}};auto& point=q.acquire(position);route.push_back(&point);return true;
    }
    if(count!=2)return false;
    const float minx=static_cast<float>(static_cast<long double>(rectangle.minimum[0])-.30000001192092896f);
    const float minz=static_cast<float>(static_cast<long double>(rectangle.minimum[1])-.30000001192092896f);
    const float maxx=static_cast<float>(static_cast<long double>(rectangle.maximum[0])+.30000001192092896f);
    const float maxz=static_cast<float>(static_cast<long double>(rectangle.maximum[1])+.30000001192092896f);
    std::array<std::array<float,4>,4> corners{{{{minx,0,minz,0}},{{maxx,0,minz,0}},{{maxx,0,maxz,0}},{{minx,0,maxz,0}}}};
    for(auto n=corners.size();n;){--n;corners[n]=q.to_world(corners[n]);} //Native reverse transform, W is unproduced padding.
    const auto reverse=dog_route_corner_indices(first_side,last_side,true),forward=dog_route_corner_indices(first_side,last_side,false);
    if(reverse.empty()||forward.empty()||reverse.size()+forward.size()!=4)return false;
    const auto corner=[&](std::uint32_t n)->const std::array<float,4>&{
        if(n>=corners.size())throw std::runtime_error("Dog detour005C95B0 reaches native unproduced corner4 from005C93B0");
        return corners[n];
    };
    const auto segment_length=[&](const std::array<float,4>& a,const std::array<float,4>& b,bool endpoint){
        std::array<float,3> delta{};
        for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(a[n])-b[n]);
        const float square=static_cast<float>(endpoint?(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[1])*delta[1]:
            (static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]);
        return dog_route_length(square);
    };
    const auto metric=[&](const auto& path){
        const float start=static_cast<float>(segment_length(first_hit,corner(path.front()),true));
        float value=static_cast<float>(segment_length(first_hit,corner(path.back()),true)+start);
        for(auto n=path.size()-1;n;){--n;value=static_cast<float>(segment_length(corner(path[n]),corner(path[n+1]),false)+value);}
        return value;
    };
    const auto reverse_length=metric(reverse),forward_length=metric(forward);
    const auto& selected=reverse_length<forward_length?reverse:forward;
    const auto insertion=static_cast<std::int64_t>(first_boundary)+1;
    if(insertion<0||insertion>static_cast<std::int64_t>(route.size())||last_boundary>static_cast<std::int64_t>(route.size()))throw std::runtime_error("Dog detour reaches native invalid route boundary");
    for(auto n=insertion;n<last_boundary;++n){q.release(route.at(static_cast<std::size_t>(insertion)));route.erase(route.begin()+insertion);}
    for(auto n=selected.size();n;){
        DogControllerVector position;const auto& value=corner(selected[--n]);position.xyz=std::array<float,3>{{value[0],value[1],value[2]}};
        auto& point=q.acquire(position);point.flags24=0x1000u;route.insert(route.begin()+insertion,&point);
    }
    return true;
}
//005CA5A0 positive rectangle stage, after the actual Car membership selection.
//Polyline X/Z is combined with the CURRENT Car physical Y before inversion;
//the native code does not flatten that affine source to Y=0.
template<class Queries>bool validate_dog_route_rectangle(std::vector<DogRoutePoint*>& route,const DogRouteRectangle& rectangle,std::size_t first,std::size_t last,Queries& q){
    if(escape_dog_route_rectangle(route,rectangle,q))return true;
    const auto own_z=q.model_position()[2],own_x=q.model_position()[0];
    std::vector<DogRoutePlanar> line{{{own_x,own_z}}};
    const auto before=first-(first!=0),after=last+(last!=route.size()-1);
    for(auto n=before;n<=after;++n){const auto position=dog_route_point(route.at(n)).position0.position();line.push_back({{position[0],position[2]}});}
    for(auto& point:line){const auto physical=q.obstacle_position();const auto local=q.to_local({{point[0],physical[1],point[1],physical[3]}});point={{local[0],local[2]}};}
    unsigned count=0;DogRoutePlanar first_crossing{},last_crossing{};std::uint32_t first_side=0,last_side=4;
    std::int32_t first_boundary=-1,last_boundary=-1;
    for(std::size_t n=first!=0;n+1<line.size();++n){
        const auto index=static_cast<std::int32_t>(n-(first!=0)+first);
        const auto hit=intersect_dog_route_rectangle(rectangle,line[n],line[n+1]);
        if(hit.count){
            if(!count){first_crossing=hit.first;first_side=hit.first_side;first_boundary=index-1;}
            else{last_crossing=hit.first;last_side=hit.first_side;last_boundary=index;}
            if(hit.count==2){last_crossing=hit.second;last_side=hit.second_side;last_boundary=index;}
        }
        count+=hit.count;if(count>=2)break;
    }
    auto first_world=q.to_world({{first_crossing[0],0,first_crossing[1],0}}),last_world=q.to_world({{last_crossing[0],0,last_crossing[1],0}});
    const auto distance=[&](const auto& position){const auto model=q.model_position();return dog_route_length(dog_controller_square(model,position,true));};
    const float first_distance=static_cast<float>(distance(first_world));
    if(distance(last_world)<first_distance){std::swap(first_world,last_world);std::swap(first_side,last_side);}
    return rewrite_dog_route_rectangle(route,count,first_world,first_boundary,last_boundary,first_side,last_side,rectangle,q);
}
}
#endif
