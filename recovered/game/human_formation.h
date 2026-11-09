#ifndef HD2_GAME_HUMAN_FORMATION_H
#define HD2_GAME_HUMAN_FORMATION_H

#include "game/human_joint_owners.h"
#include "game/mission_navigation.h"
#include <memory>
#include <vector>

namespace hd2::game {

//005965E0 fills8ABF50;005971E0 copies these12 vectors to8AB410 (mode3).
//Fourth lanes are genuine BSS zeros, unlike the constructor's stack-vector W.
inline constexpr std::array<HumanVector,6> human_formation_loading_positions{{
    {{0,0,0,0}},{{-1,0,-1,0}},{{1,0,-1,0}},{{-2,0,-2,0}},{{-1,0,-2.5f,0}},{{1,0,-2.5f,0}}
}};
inline constexpr std::array<HumanVector,6> human_formation_loading_directions{{
    {{0,0,1,0}},{{-.5f,0,1,0}},{{.5f,0,1,0}},{{-1,0,1,0}},{{1,0,1,0}},{{0,0,1,0}}
}};

//00597420/005978E0 store squared length before the epsilon comparison.
//00597C50/00597F80 keep that comparison extended, but reload FLOAT for sqrt.
inline HumanVector normalize_formation_vector(HumanVector v,bool actor_direction=false) {
    constexpr float epsilon=9.99999993922529e-09f;
    const auto x=static_cast<long double>(v[0])*v[0],y=static_cast<long double>(v[1])*v[1],z=static_cast<long double>(v[2])*v[2];
    const auto extended=actor_direction?(y+x)+z:(x+y)+z;
    const auto stored=static_cast<float>(extended);
    if(!std::isfinite(extended)||!std::isfinite(stored))throw std::runtime_error("Non-finite Human formation direction");
    if(std::abs((actor_direction?extended:static_cast<long double>(stored))-1)>=epsilon) {
        if(stored>=epsilon) {
            const auto scale=1/std::sqrt(static_cast<long double>(stored));
            for(unsigned axis=0;axis!=3;++axis)v[axis]=static_cast<float>(v[axis]*scale);
        }else {const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.f:1.f;}
    }
    return v;
}

//005949C0/00594A70: Human2C8's actual six-member owner. Frames delegates
//SetDir/vector transform to the real engine; it does not earn engine body credit.
//This covers mode3 loading and member/leader/cleanup behavior. Runtime formation
//modes/orders are separate producers; unknown scalar lanes are zero guards.
template<class Frames,class Human>class HumanFormation {
public:
    struct Slot {
        Slot(){words.set(0x84,std::int32_t{-1});words.set(0x9c,std::uint16_t{0xffff});words.set(0x7e,std::uint16_t{0xffff});} //00595370.
        HumanJointWords<0xa0> words;Human* member=nullptr;std::vector<std::uint16_t> indices; //Native70 and90/94/98, not DWORD pointers.
        HumanVector vector(unsigned offset)const{return words.template get<HumanVector>(offset);}
        void vector(unsigned offset,const HumanVector& v){words.set(offset,v);}
    };
    HumanFormation(Frames& frames,MissionNavigation& navigation):frames_(frames),navigation_(navigation),matrix_(human_identity()) {
        select_loading_mode();direction({{0,0,1,0}});scale({{2.5f,2.5f,2.5f,0}});changed_=false;
        //Native68/6C and the two temporary vector W lanes are unwritten; guards only.
    }
    ~HumanFormation(){close();} //00594A70→00597420(13), buffers reverse-retire with slots.
    HumanFormation(const HumanFormation&)=delete;HumanFormation& operator=(const HumanFormation&)=delete;
    void close()noexcept {if(slots_){changed_=true;slots_.reset();mode_=13;}}
    bool has_slots()const{return bool(slots_);}std::int32_t count()const{return count_;}
    Human* leader()const{return leader_;}std::int32_t mode()const{return mode_;}
    const HumanMatrix& matrix()const{return matrix_;}const auto& slots()const{return required_slots();}
    Slot& slot(unsigned n){return required_slots().at(n);}
    bool contains(Human* human,std::int32_t* index=nullptr)const { //00598290, reverse, including NULL.
        const auto& slots=required_slots();
        for(std::size_t n=slots.size();n;)if(slots[--n].member==human){if(index)*index=static_cast<std::int32_t>(n);return true;}
        return false;
    }
    void select_loading_mode() { //00597420(3), original allocation precedes all six writes.
        changed_=true;if(!slots_)slots_=std::make_unique<std::array<Slot,6>>();mode_=3;
        for(unsigned n=6;n;) {--n;auto& s=(*slots_)[n];s.vector(0,human_formation_loading_positions[n]);s.vector(0x50,normalize_formation_vector(human_formation_loading_directions[n]));}
        rebuild();
    }
    void direction(const HumanVector& value) {direction_=value;rebuild_matrix();rebuild();} //00597760.
    void scale(const HumanVector& value) {scale_=value;rebuild_matrix();rebuild();} //005977F0.
    bool add(Human* human) { //00597F80; publish member before matrix reads, first empty slot.
        if(contains(human))return false;
        for(auto& s:required_slots())if(!s.member) {
            if(!human)throw std::runtime_error("Formation insertion requires actual Human");
            s.member=human;auto m=frames_.actor_world_matrix(human->base().frame());s.vector(0x20,{{m[12],m[13],m[14],m[15]}});
            m=frames_.actor_world_matrix(human->base().frame());s.vector(0x40,normalize_formation_vector({{m[8],m[9],m[10],m[11]}},true));
            s.vector(0x30,s.vector(0x20));s.words.set(0x88,std::uint8_t{1});++count_;return true;
        }
        return false;
    }
    bool select_leader(Human* human) { //00597C50; slot0 swap is MEMBERS ONLY, not whole records.
        if(human==leader_)return true;
        reset_navigation(0xffff);
        if(human&&!contains(human)&&!add(human))return false;
        leader_=human;if(!human)return true;
        auto& slots=required_slots();const auto old=slots[0].member;slots[0].member=human;
        for(unsigned n=6;n;)if(slots[--n].member==human){slots[n].member=old;slots[0].member=human;break;}
        rebuild();
        auto leader_matrix=frames_.actor_world_matrix(human->base().frame());
        const auto forward=frames_.actor_world_matrix(human->base().frame());
        frames_.formation_matrix_direction(leader_matrix,normalize_formation_vector({{forward[8],forward[9],forward[10],forward[11]}},true));
        const auto position=frames_.actor_world_matrix(human->base().frame());
        //SetDir preserved translation until this explicit THREE-component write.
        for(unsigned axis=0;axis!=3;++axis)leader_matrix[12+axis]=position[12+axis];
        for(unsigned n=6;n;) {--n;slots[n].vector(0x30,frames_.formation_transform(slots[n].vector(0x10),leader_matrix));}
        slots[0].vector(0x20,{{position[12],position[13],position[14],position[15]}});
        slots[0].words.set(0x7e,std::uint16_t{0xffff});(void)project(0);return true;
    }
    bool remove(Human* human) { //005981C0, keep original leader-preserving member-only shifts.
        if(!contains(human))return false;
        if(human==leader_)select_leader(nullptr);
        auto& slots=required_slots();
        for(unsigned n=0;n!=6;++n)if(slots[n].member==human) {
            slots[n].member=nullptr;
            if(leader_&&n<5)for(unsigned j=n;j<5;++j) {
                if(slots[n].member==leader_||slots[j].member==leader_) {
                    if(slots[j+1].member==leader_)slots[j].member=slots.at(j+2).member;
                }else slots[j].member=slots[j+1].member;
            }
            slots[5].member=nullptr;--count_;return true;
        }
        return false;
    }
    bool project(unsigned index) { //0059B7B0, same actual navigation owner; failure leaves position intact.
        auto& s=slot(index);auto p=s.vector(0x20);
        const std::array<float,3> point{{p[0],p[1],p[2]}};
        auto nav=navigation_.nearest_volume(point,5,-1);s.words.set(0x7e,nav);
        if(nav==0xffff){nav=navigation_.nearest_floor_volume(point,5,-1);s.words.set(0x7e,nav);if(nav==0xffff)return false;}
        const auto* record=navigation_.record(nav);const auto position=navigation_.position(nav);
        p[1]=0;s.vector(0x20,p);
        //X subtraction stays extended while Z is stored to FLOAT before the sum.
        const auto x=static_cast<long double>(p[0])-position[0];
        auto z=static_cast<float>(static_cast<long double>(p[2])-position[2]);
        const auto square=(static_cast<long double>(z)*z+0)+x*x;
        const float radius=static_cast<unsigned char>(record->prefix[0x13])*.0625f;
        if(static_cast<long double>(radius)*radius<square) {
            const float stored=static_cast<float>(square);long double nx=x;float ny=0;
            if(std::abs(square-1)>=9.99999993922529e-09f) {
                if(stored>=9.99999993922529e-09f) {
                    const auto factor=1/std::sqrt(static_cast<long double>(stored));nx*=factor;ny=static_cast<float>(ny*factor);z=static_cast<float>(z*factor);
                }else if(nx!=0)nx=nx<0?-1:1;else if(z!=0)z=z<0?-1.f:1.f;else ny=1;
            }
            p[0]=static_cast<float>(nx*radius+position[0]);p[1]=static_cast<float>(static_cast<long double>(ny)*radius);
            p[2]=static_cast<float>(static_cast<long double>(z)*radius+position[2]);
            //Native2C is uninitialized stack W on this branch: preserve guard.
        }
        p[1]=position[1];s.words.set(0x88,std::uint8_t{1});
        if(leader_&&record->byte16==15) {
            const auto height=static_cast<float>(static_cast<long double>(position[1])+.5f);
            const auto leader_y=leader_->base().world_position()[1];
            p[1]=leader_y<=height?height:leader_y;
        }
        s.vector(0x20,p);return true;
    }
private:
    auto& required_slots(){if(!slots_)throw std::logic_error("Formation has no live member records");return *slots_;}
    const auto& required_slots()const{if(!slots_)throw std::logic_error("Formation has no live member records");return *slots_;}
    void reset_navigation(std::uint16_t value){navigation_id_=value;navigation_state_=-1;} //00594A50.
    void rebuild_matrix() {
        frames_.formation_matrix_direction(matrix_,direction_);
        for(unsigned row=0;row!=3;++row)for(unsigned axis=0;axis!=3;++axis)
            matrix_[row*4+axis]=static_cast<float>(static_cast<long double>(matrix_[row*4+axis])*scale_[row]);
    }
    void rebuild() { //005978E0 reverse records; stored lengths and native signed-bit tiny test.
        if(!slots_)return;
        for(unsigned n=6;n;) {--n;auto& s=(*slots_)[n];s.vector(0x10,frames_.formation_transform(s.vector(0),matrix_));
            s.vector(0x60,normalize_formation_vector(frames_.formation_transform(s.vector(0x50),matrix_)));
            const auto a=s.vector(0x10),b=(*slots_)[0].vector(0x10);
            const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
            const float square=static_cast<float>((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
            const float length=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
            s.words.set(0x78,length);
        }
    }
    Frames& frames_;MissionNavigation& navigation_;HumanMatrix matrix_;HumanVector direction_{},scale_{};
    std::unique_ptr<std::array<Slot,6>> slots_;Human* leader_=nullptr;std::int32_t mode_=3,count_=0,navigation_state_=-1;
    std::uint16_t navigation_id_=0xffff;bool changed_=false;
};

template<class Human>void clear_human_formation(Human& human);
template<class Human>void retire_human_formation(Human& human) { //005A9120: callback may recursively retire this SAME owner.
    for(unsigned n=6;n&&human.formation&&human.formation->has_slots();) {
        auto* member=human.formation->slots()[--n].member;
        if(member&&member!=&human)clear_human_formation(*member);
    }
    if(human.formation)human.formation.reset();
}
template<class Human>void clear_human_formation(Human& human) { //005A91A0(NULL), clear borrower LAST.
    auto* leader=human.formation_leader;if(!leader)return;
    if(leader->formation) {
        leader->formation->remove(&human);
        if(leader->formation->count()<2)retire_human_formation(*leader);
    }
    human.formation_leader=nullptr;
}
template<class Frames,class Human>
void set_human_formation(Frames& frames,MissionNavigation& navigation,const HumanVector& shared_scale,Human& human,Human* leader) { //005A91A0.
    if(leader==human.formation_leader)return;
    clear_human_formation(human);if(!leader)return;
    if(!leader->formation) {
        leader->formation=std::make_unique<HumanFormation<Frames,Human>>(frames,navigation);
        leader->formation->add(leader);leader->formation->add(&human);leader->formation->select_leader(leader);
        leader->formation->select_loading_mode();leader->formation->scale(shared_scale);
    }else leader->formation->add(&human);
    //Native publishes the follower borrower even if the six-slot add failed.
    human.formation_leader=leader;
}
} //namespace hd2::game
#endif
