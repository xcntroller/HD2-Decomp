#ifndef HD2_GAME_HUMAN_SCENE_MOVEMENT_DETOUR_H
#define HD2_GAME_HUMAN_SCENE_MOVEMENT_DETOUR_H

#include "game/human_scene_movement_geometry.h"

namespace hd2::game::human_scene_movement {

struct DetourSide {
    std::array<HumanVector,4> points{};
    unsigned count=0;
};
template<class Actor>struct DetourObstacle {
    DetourSide sides[2];
    Actor* human=nullptr;
    Actor* car=nullptr;
    unsigned intersection=1;
};
template<class Actor>struct alignas(16) DetourNode {
    struct Edge{DetourNode* next=nullptr;bool blocked=false;};
    HumanVector position{};
    MissionTableRecord* floor=nullptr;
    std::array<Edge,8> children{};
    unsigned child_count=0;
    std::array<Actor*,4> humans{};
    unsigned human_count=0;
    Actor* car=nullptr;
    bool ignores_human(Actor* actor)const{
        return std::find(humans.begin(),humans.begin()+human_count,actor)!=humans.begin()+human_count;
    }
    void avoid(const DetourObstacle<Actor>& obstacle){
        if(obstacle.human&&human_count<humans.size())humans[human_count++]=obstacle.human;
        if(obstacle.car)car=obstacle.car;
    }
    void append(DetourNode& node){children[child_count++]={&node,false};}
};

//00563F90 owns 200 aligned work records and ten selected-route records in the
//008AADD0 process. The graph uses typed edges with the original low-bit state.
template<class Actor>class DetourSearch {
public:
    using Node=DetourNode<Actor>;
    template<class Probe,class Floor>
    void run(const HumanVector& from,MissionTableRecord* floor,const HumanVector& destination,
            MissionTableRecord* target,Probe probe,Floor refresh_floor){
        nodes_[0]={};
        nodes_[1]={};
        nodes_[0].position=from;
        nodes_[0].floor=floor;
        nodes_[1].position=destination;
        nodes_[1].floor=target;
        nodes_[0].append(nodes_[1]);
        used_=2;
        selected_=0;
        best_distance_=best_cost_=100000000.f;
        expand(nodes_[0],0,probe,refresh_floor);
        if(used_!=2){
            std::array<const Node*,10> path{};
            choose(nodes_[0],0,0,path);
        }
    }
    const Node& selected(unsigned index)const{return best_.at(index);}
    unsigned selected_count()const{return selected_;}
    float remaining_distance()const{return best_distance_;}
private:
    static constexpr unsigned capacity=200;
    std::unique_ptr<Node[]> nodes_=std::make_unique<Node[]>(capacity);
    std::array<Node,10> best_{};
    unsigned used_=0,selected_=0;
    float best_distance_=0,best_cost_=0;

    bool insert_side(Node& parent,Node& destination,const DetourSide& side,
            const DetourObstacle<Actor>& obstacle){ //0056A8E0's two branches.
        if(!side.count||parent.child_count==parent.children.size())return true;
        for(auto n=parent.child_count;n;){
            const auto& edge=parent.children[--n];
            if(square(difference(side.points[0],edge.next->position))<1.e-16f)return true;
        }
        auto* link=&parent;
        for(unsigned n=0;n<side.count;++n){
            if(used_==capacity)return false;
            auto& node=nodes_[used_++];
            node={};
            node.position=side.points[n];
            node.avoid(obstacle);
            link->append(node);
            link=&node;
        }
        if(obstacle.intersection==1){
            link->append(destination);
            destination.avoid(obstacle);
        }else{
            //The native kind2 record carries only the first two child pointers.
            if(destination.child_count>2)throw std::logic_error("Scene detour kind2 requires initialized continuation edges");
            for(unsigned n=0;n<destination.child_count;++n)link->children[n]=destination.children[n];
            link->child_count=destination.child_count;
        }
        return true;
    }
    template<class Probe,class Floor>
    void expand(Node& node,unsigned depth,Probe& probe,Floor& refresh_floor){
        if(depth==10)return;
        const auto offset=difference(node.position,nodes_[1].position);
        if(static_cast<long double>(offset[0])*offset[0]+static_cast<long double>(offset[2])*offset[2]<epsilon){
            node.child_count=0;
            node.floor=nodes_[1].floor;
            return;
        }
        for(unsigned n=0;n<node.child_count;++n){
            auto& edge=node.children[n];
            if(edge.blocked)continue;
            auto& child=*edge.next;
            const auto obstacle=probe(node,child);
            if(!obstacle){edge.blocked=true;continue;}
            if(obstacle->human||obstacle->car){
                if(!insert_side(node,child,obstacle->sides[0],*obstacle)||
                   !insert_side(node,child,obstacle->sides[1],*obstacle))return;
                edge.blocked=true;
                continue;
            }
            expand(child,depth+1,probe,refresh_floor);
            if(!child.floor){
                child.floor=node.floor;
                refresh_floor(child);
                if(!child.floor)edge.blocked=true;
            }
        }
    }
    void choose(const Node& node,float cost,unsigned depth,std::array<const Node*,10>& path){ //0056A6D0.
        if(depth==10)return;
        bool leaf=true;
        for(unsigned n=0;n<node.child_count;++n){
            const auto& edge=node.children[n];
            if(edge.blocked||!edge.next->floor)continue;
            leaf=false;
            path[depth]=edge.next;
            const auto segment=difference(edge.next->position,node.position);
            const auto squared=(static_cast<long double>(segment[1])*segment[1]+
                static_cast<long double>(segment[0])*segment[0])+static_cast<long double>(segment[2])*segment[2];
            choose(*edge.next,static_cast<float>(cost+squared),depth+1,path);
        }
        if(!leaf||!depth)return;
        const auto distance=static_cast<float>(square(difference(node.position,nodes_[1].position)));
        const auto difference=distance-best_distance_;
        if(!negative_word(difference)&&!(difference<.0001f&&cost<best_cost_))return;
        for(unsigned n=0;n<depth;++n)best_[n]=*path[n];
        best_distance_=distance;
        best_cost_=cost;
        selected_=depth;
    }
};

inline std::array<DetourSide,2> ellipsoid_detour_sides(const ContactVolume& volume,
        const HumanVector& origin,const HumanVector& local_origin,const HumanVector& direction,
        const HumanVector& lateral,float expansion){ //0056DE90 and the containing axis transform.
    constexpr std::array<HumanVector,4> corners{{{{1,0,1,0}},{{1,0,-1,0}},{{-1,0,1,0}},{{-1,0,-1,0}}}};
    std::array<DetourSide,2> sides{};
    std::array<std::array<float,4>,2> orders{};
    for(auto corner:corners){
        const auto offset=difference(corner,local_origin);
        const auto branch=negative_word(static_cast<float>(dot(offset,lateral)))?1u:0u;
        const auto order=static_cast<float>(dot(offset,direction));
        auto& side=sides[branch];
        auto slot=side.count++;
        while(slot&&order<=orders[branch][slot-1]){
            side.points[slot]=side.points[slot-1];
            orders[branch][slot]=orders[branch][slot-1];
            --slot;
        }
        orders[branch][slot]=order;
        for(unsigned i=0;i!=3;++i)corner[i]*=expansion;
        auto point=volume.unproject(corner);
        for(unsigned i=0;i!=3;++i)point[i]+=origin[i];
        point[3]=origin[3];
        side.points[slot]=point;
    }
    return sides;
}
struct DetourContact {
    HumanVector lateral;
    float time;
    unsigned intersection;
};
struct DetourRoots {float exit_time=100000000.f,entry_time=100000000.f;};
inline std::optional<DetourContact> detour_contact(const HumanVector& origin,
        const HumanVector& direction,float radius_squared,float nearest,
        DetourRoots& roots,bool ellipsoid){ //0056C230's line/volume intersection.
    const float length=static_cast<float>(square(direction));
    const float projection=static_cast<float>(dot(origin,direction));
    const float twice=projection*2;
    if(!negative_word(twice)||!(length>0))return std::nullopt;
    const float gap=static_cast<float>(square(origin)-radius_squared);
    const bool inside=negative_word(gap);
    const bool calculate_roots=!ellipsoid||!inside;
    HumanVector lateral{};
    if(calculate_roots){
        const auto ratio=static_cast<long double>(projection)/length;
        for(unsigned i=0;i!=3;++i)lateral[i]=static_cast<float>(origin[i]-ratio*direction[i]);
    }
    const float lateral_square=static_cast<float>(square(lateral));
    auto time=inside?0.f:nearest;
    if(calculate_roots){
        if(lateral_square>radius_squared)return std::nullopt;
        const float discriminant=static_cast<float>(static_cast<long double>(twice)*twice-4.L*length*gap);
        if(negative_word(discriminant))return std::nullopt;
        const auto root=std::sqrt(static_cast<long double>(discriminant));
        roots.exit_time=static_cast<float>((root-twice)/(2.L*length));
        roots.entry_time=static_cast<float>((-twice-root)/(2.L*length));
        for(const auto candidate:{roots.exit_time,roots.entry_time})
            if(!negative_word(candidate)&&candidate<1&&candidate<time)time=candidate;
    }
    if(!(time<nearest))return std::nullopt;
    if(lateral_square<=epsilon){
        lateral={{direction[2],0,-direction[0],0}};
        const auto inverse=fast_inverse_root(ellipsoid&&calculate_roots?2*length:length);
        for(unsigned i=0;i!=3;++i)lateral[i]*=inverse;
    }else{
        const auto inverse=fast_inverse_root(lateral_square);
        for(unsigned i=0;i!=3;++i)lateral[i]*=inverse;
    }
    return DetourContact{lateral,time,roots.exit_time>1||roots.entry_time>1?2u:1u};
}

} //namespace hd2::game::human_scene_movement
#endif
