#ifndef HD2_GAME_COLLISION_DEBUG_LINES_H
#define HD2_GAME_COLLISION_DEBUG_LINES_H
#include "game/human_joint_math.h"
#include <memory>
namespace hd2::game {
//SAME process buffer008CBC18..0097B898, cursor00844168 and count0097B898.
//Native BSS supplies zero UV; resets retain vertex storage. Startup owns this
//alongside the same shared multisphere budget, not an unused per-Dog line list.
class CollisionDebugLines {
public:
    struct Vertex{std::array<float,3> position{};std::uint32_t color=0;std::array<float,2> uv{};};
    static_assert(sizeof(Vertex)==24,"Native collision line vertex stride");
    static constexpr unsigned capacity=15000;
    CollisionDebugLines():vertices_(std::make_unique<std::array<Vertex,capacity*2>>()){}
    void append(const HumanVector& first,const HumanVector& second,std::uint32_t first_color,std::uint32_t second_color){ //0071F700.
        if(count_>=capacity)return;
        ++count_;auto& a=vertices_->at(cursor_++);
        for(unsigned n=0;n!=3;++n)a.position[n]=first[n];
        a.color=first_color|0xff000000u;auto& b=vertices_->at(cursor_++);
        for(unsigned n=0;n!=3;++n)b.position[n]=second[n];
        b.color=second_color?(second_color|0xff000000u):a.color;
    }
    void reset(){count_=cursor_=0;} //0071F8A0 absent frozen entry, no credit.
    template<class Frames>void draw(Frames& frames){ //0071F850 absent frozen entry, no credit.
        if(!count_)return;
        frames.collision_debug_begin();frames.collision_debug_matrix(identity_);
        frames.collision_debug_draw(count_,vertices_->data());
    }
    unsigned count()const{return count_;}unsigned cursor()const{return cursor_;}
    const auto& vertices()const{return *vertices_;}
private:
    std::unique_ptr<std::array<Vertex,capacity*2>> vertices_;
    unsigned count_=0,cursor_=0;const HumanMatrix identity_=human_identity(); //Native permanent00816310 identity.
};
}
#endif
