#ifndef HD2_GAME_HUMAN_FACE_HIT_GEOMETRY_H
#define HD2_GAME_HUMAN_FACE_HIT_GEOMETRY_H

#include "game/human_face_mesh_pool.h"
#include "game/human_ai_shot_direction.h"
#include <algorithm>
#include <limits>

namespace hd2::game::human_face_hit {

using Position=HumanFaceMeshData::Position;
using Node=HumanFaceMeshData::Node;
using Group=HumanFaceMeshData::Group;

inline long double squared_distance(const Position& first,const Position& second){
    const auto x=static_cast<long double>(first[0])-second[0];
    const auto y=static_cast<long double>(first[1])-second[1];
    const auto z=static_cast<long double>(first[2])-second[2];
    return (z*z+y*y)+x*x;
}

inline Node* neighbour(Node& node,const HumanFaceMeshData::Edge& edge){
    if(edge.first==&node)return edge.second;
    return edge.second==&node?edge.first:nullptr;
}

template<class Random>Node* random_node(Group& group,Random& random){
    const auto draw=random();
    if(group.nodes.empty())throw std::runtime_error("Human face search reached its native empty-node division fault");
    auto* node=group.nodes[draw%group.nodes.size()].get();
    if(!node)throw std::logic_error("Human face search requires its actual graph node");
    return node;
}

template<class Mesh,class Random>
Node* walk_nearest_node(Group& group,const Position& point,Mesh& mesh,Node* node,
        std::vector<std::uint8_t>& visits,Random& random){ //00797E40.
    if(!node)node=random_node(group,random);
    ++visits.at(node->vertex);
    auto distance=squared_distance(mesh.position(node->vertex),point);
    for(;;){
        Node* selected=nullptr;
        for(auto index=node->edges.size();index;){
            const auto* edge=node->edges[--index];
            if(!edge)throw std::logic_error("Human face search has a missing actual edge");
            auto* candidate=neighbour(*node,*edge);
            if(!candidate||visits.at(candidate->vertex)==visits.at(node->vertex))continue;
            const auto next=squared_distance(mesh.position(candidate->vertex),point);
            if(next<=distance){selected=candidate;distance=static_cast<float>(next);} //Native ordered <=, including ties.
        }
        if(!selected)return node;
        visits.at(selected->vertex)=visits.at(node->vertex);
        node=selected;
    }
}

template<class Mesh,class Random>
Node* closest_head_node(Group& group,const Position& point,Mesh& mesh,Random& random){ //00797620.
    //Native operator_new leaves these bytes indeterminate. Define the temporary
    //scratch in the C++ port while retaining the actual four randomized walks.
    std::vector<std::uint8_t> visits(group.vertex_count,0);
    Node* selected=nullptr;
    float distance=100000000.f;
    for(unsigned attempt=0;attempt!=4;++attempt){
        auto* node=walk_nearest_node(group,point,mesh,nullptr,visits,random);
        const auto position=mesh.position(node->vertex);
        const auto x=static_cast<long double>(point[0])-position[0];
        const auto y=static_cast<long double>(point[1])-position[1];
        const auto z=static_cast<long double>(point[2])-position[2];
        const auto next=(z*z+x*x)+y*y;
        if(next<distance){distance=static_cast<float>(next);selected=node;}
    }
    return selected;
}

template<class Mesh,class Random>
Node* search_nearest_node(Group& group,const Position& point,Mesh& mesh,Node* node,
        std::vector<std::uint8_t>& visits,Random& random,float first,float second,float third){ //00798010.
    if(!node)node=random_node(group,random);
    ++visits.at(node->vertex);
    Node* selected=node;
    float distance=static_cast<float>(squared_distance(mesh.position(node->vertex),point));
    for(auto index=node->edges.size();index;){
        const auto* edge=node->edges[--index];
        if(!edge)throw std::logic_error("Human face recursive search has a missing actual edge");
        auto* candidate=neighbour(*node,*edge);
        if(!candidate||visits.at(candidate->vertex)==visits.at(node->vertex))continue;
        const auto next=squared_distance(mesh.position(candidate->vertex),point);
        if(!(next<third))continue;
        auto* found=search_nearest_node(group,point,mesh,candidate,visits,random,static_cast<float>(next),first,second);
        const auto found_distance=squared_distance(mesh.position(found->vertex),point);
        if(found_distance<distance){selected=found;distance=static_cast<float>(found_distance);}
    }
    return selected;
}

template<class Mesh,class Random>
Node* closest_body_node(Group& group,const Position& point,Mesh& mesh,Random& random){ //00797780.
    std::vector<std::uint8_t> visits(group.vertex_count,0);
    Node* selected=nullptr;
    float distance=100000000.f;
    for(unsigned offset:{0x30u,0x34u,0xacu,0x6cu}){
        std::fill(visits.begin(),visits.end(),0);
        const auto index=group.fields2c.at((offset-0x2c)/4);
        auto* start=group.nodes.at(index).get();
        auto* node=search_nearest_node(group,point,mesh,start,visits,random,100000000.f,100000000.f,100000000.f);
        const auto next=squared_distance(mesh.position(node->vertex),point);
        if(next<distance){selected=node;distance=static_cast<float>(next);}
    }
    return selected;
}

template<class Mesh>std::array<float,2> texture_coordinates(Node& node,const Position& point,Mesh& mesh){ //00799BB0 projection prefix.
    const auto original=mesh.uv(node.vertex);
    if(node.edges.size()<2)return original;
    const auto endpoint=[&](unsigned index){
        const auto* edge=node.edges.at(index);
        if(!edge)throw std::logic_error("Human face stain requires its actual first two edges");
        auto* other=neighbour(node,*edge);
        if(!other)throw std::logic_error("Human face stain requires connected endpoint nodes");
        return other->vertex;
    };
    const auto first=endpoint(0),second=endpoint(1);
    const auto position=mesh.position(node.vertex);
    const auto direction=[&](std::uint16_t index){
        const auto other=mesh.position(index);
        HumanVector value{{other[0]-position[0],other[1]-position[1],other[2]-position[2],0}};
        const auto square=(static_cast<long double>(value[1])*value[1]+
            static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[0])*value[0];
        normalize_human_ai_shot(value,square);
        return value;
    };
    const auto first_direction=direction(first),second_direction=direction(second);
    const auto x=static_cast<long double>(point[0])-position[0];
    const float y=point[1]-position[1],z=point[2]-position[2];
    const auto project=[&](const HumanVector& axis){return static_cast<float>((static_cast<long double>(z)*axis[2]+
        static_cast<long double>(y)*axis[1])+x*axis[0]);};
    const auto first_projection=project(first_direction),second_projection=project(second_direction);
    const auto first_uv=mesh.uv(first),second_uv=mesh.uv(second);
    std::array<float,2> result;
    for(unsigned axis=0;axis!=2;++axis)
        result[axis]=static_cast<float>((static_cast<long double>(first_projection)*(static_cast<long double>(first_uv[axis])-original[axis])+
            static_cast<long double>(second_projection)*(static_cast<long double>(second_uv[axis])-original[axis]))+original[axis]);
    return result;
}

inline std::uint16_t blend_pixel(std::uint16_t source,std::uint16_t destination){
    const std::uint32_t from=source,to=destination,a=from>>12,b=to>>12,total=a+b;
    if(!total)return 0;
    auto result=std::max(a,b)<<12;
    for(unsigned bits:{0xf00u,0xf0u,0xfu})result|=(((to&bits)*b+(from&bits)*a)/total)&bits;
    return static_cast<std::uint16_t>(result);
}

inline std::int16_t coordinate(long double value){ //Native __ftol LOW WORD, including its indefinite QWORD.
    if(!std::isfinite(value)||value<static_cast<long double>(std::numeric_limits<std::int64_t>::min())||
       value>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))return 0;
    const auto bits=static_cast<std::uint16_t>(static_cast<std::int64_t>(value));
    return static_cast<std::int16_t>(bits&0x8000u?static_cast<int>(bits)-65536:bits);
}

template<class Pixels>void stamp(std::vector<std::uint16_t>& destination,std::uint32_t width_mask,
        std::uint32_t height_mask,std::uint32_t width_log,std::int16_t centre_x,std::int16_t centre_y,
        const Pixels& source,std::uint16_t width,std::uint16_t height){ //0079A420, texture copy follows in the actual frame bridge.
    if(static_cast<std::size_t>(width)*height>source.size())throw std::runtime_error("Human face stamp has a short actual pattern");
    auto y=static_cast<std::uint16_t>(static_cast<std::uint16_t>(centre_y)-(height>>1));
    std::size_t pixel=0;
    for(unsigned row=0;row!=height;++row,++y){
        auto x=static_cast<std::uint16_t>(static_cast<std::uint16_t>(centre_x)-(width>>1));
        for(unsigned column=0;column!=width;++column,++x,++pixel){
            if((y&~static_cast<std::uint16_t>(height_mask))||(x&~static_cast<std::uint16_t>(width_mask)))continue;
            const auto index=((static_cast<std::uint32_t>(static_cast<std::int16_t>(y))&height_mask)<<(width_log&31u))+
                (static_cast<std::uint32_t>(static_cast<std::int16_t>(x))&width_mask);
            auto& value=destination.at(index);
            value=blend_pixel(source[pixel],value);
        }
    }
}

} //namespace hd2::game::human_face_hit
#endif
