#ifndef HD2_GAME_HUMAN_FORMATION_H
#define HD2_GAME_HUMAN_FORMATION_H

#include "game/human_joint_owners.h"
#include "game/mission_navigation.h"
#include "game/human_formation_layouts.h"
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

//00596D60→005971E0: mode9 is the actual Players subgroup constructor mode.
inline constexpr std::array<HumanVector,6> human_formation_subgroup_positions{{
    {{0,0,0,0}},{{-1,0,-1,0}},{{1,0,-1,0}},{{-2,0,-2,0}},{{2,0,-2,0}},{{-3,0,-3,0}}
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
    bool consume_member_position(Human* human,HumanVector& position,std::uint16_t* point=nullptr){ //00598320.
        if(!human||!leader_||!slots_)return false;
        std::int32_t index=0;if(!contains(human,&index))return false;
        auto& slot=slots_->at(static_cast<std::size_t>(index));
        position=slot.vector(0x20); //Outputs are written even when changed88 is false.
        if(point)*point=slot.words.template get<std::uint16_t>(0x7e);
        if(!slot.words.template get<std::uint8_t>(0x88))return false;
        slot.words.set(0x88,std::uint8_t{0});return true;
    }
    void request_member_position(Human* human,const HumanVector& position){ //00599040, reset point/changed only on membership.
        std::int32_t index=0;if(!contains(human,&index))return;
        auto& s=slot(static_cast<unsigned>(index));s.vector(0x20,position);
        s.words.set(0x7e,std::uint16_t{0xffff});s.words.set(0x88,std::uint8_t{1});
    }
    void update_member_route(unsigned index){advance_member_route(index);} //0059AF30, same cache and point guards.
    void select_loading_mode(){select_mode(3);}
    void select_subgroup_mode(){select_mode(9);}
    void select_mode(std::int32_t requested){ //00597420, SAME slots; cycle12 and close13 policies.
        changed_=true;
        if(requested==13&&slots_){slots_.reset();mode_=13;return;}
        const auto mode=requested==12?(mode_+1>10?1:mode_+1):requested;
        if(mode<0||mode>=static_cast<std::int32_t>(human_formation_layouts.size()))throw std::runtime_error("Formation mode exceeds original static data");
        if(!slots_)slots_=std::make_unique<std::array<Slot,6>>();
        mode_=mode;
        const auto& layout=human_formation_layouts[static_cast<std::size_t>(mode_)];
        for(unsigned n=6;n;){--n;auto& s=(*slots_)[n];s.vector(0,layout[n]);s.vector(0x50,normalize_formation_vector(layout[6+n]));}
        rebuild();
    }
    void request_navigation(std::uint16_t point){reset_navigation(point);} //00594A50, same actual order fields.
    void subgroup_policy(){subgroup_=true;} //004608A0's actual formation8C=1, before add/leader/mode9.
    template<class Runtime>void advance(std::uint32_t elapsed,Runtime& runtime){ //0059BA70.
        if(!leader_||!slots_)return;
        navigation_state_=navigation_id_==0xffff?1:2;
        smooth_positions(elapsed); //Always first, including suppressed ordinary or attached branches.
        if(navigation_state_==1){
            if(leader_->field160){const auto kind=leader_->field160->base().type();
                if(kind==4||kind==20){advance_attached(runtime);return;}}
            if(!runtime.formation_direction_gate(*leader_))advance_ordinary(runtime);
        }else if(navigation_state_==2)advance_navigation();
    }
    template<class Runtime>void advance_attached(Runtime& runtime){advance_attached_actor(leader_->field160,runtime);} //005957D0(NULL).
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
    void reset_navigation(std::uint16_t value){navigation_id_=value;navigation_stage_=-1;} //00594A50.
    static HumanVector difference(const HumanVector& a,const HumanVector& b){
        HumanVector result{};for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(static_cast<long double>(a[axis])-b[axis]);return result;
    }
    static long double square(const HumanVector& v,unsigned order=0){
        const auto x=static_cast<long double>(v[0])*v[0],y=static_cast<long double>(v[1])*v[1],z=static_cast<long double>(v[2])*v[2];
        return order==1?(z+y)+x:order==2?(y+z)+x:order==3?(x+z)+y:(x+y)+z;
    }
    static float magnitude(const HumanVector& v,unsigned order=0){
        const float stored=static_cast<float>(square(v,order));std::int32_t bits;std::memcpy(&bits,&stored,4);
        if(!std::isfinite(stored))throw std::runtime_error("Non-finite Human formation frame magnitude");
        return bits<0x322bcc77?0.f:std::abs(static_cast<long double>(stored)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(stored)));
    }
    static HumanVector normalize(HumanVector v,unsigned order=0){
        const auto extended=square(v,order);const auto stored=static_cast<float>(extended);
        if(!std::isfinite(extended)||!std::isfinite(stored))throw std::runtime_error("Non-finite Human formation frame direction");
        if(std::abs(extended-1)>=9.99999993922529e-09f){
            if(stored>=9.99999993922529e-09f){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
                for(unsigned axis=0;axis!=3;++axis)v[axis]=static_cast<float>(v[axis]*inverse);
            }else {const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.f:1.f;}
        }
        return v;
    }
    void smooth_positions(std::uint32_t elapsed){ //0059BB60, ascending all six records, signed time.
        std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
        const float speed=static_cast<float>(static_cast<long double>(signed_elapsed)*.004000000189989805f);
        for(unsigned n=0;n!=6;++n){auto& s=slot(n);const auto current=s.vector(0x20);auto old=s.vector(0x30);
            const float dx=static_cast<float>(static_cast<long double>(current[0])-old[0]);
            float dy=0,dz=static_cast<float>(static_cast<long double>(current[2])-old[2]);
            const float stored=static_cast<float>(static_cast<long double>(dz)*dz+static_cast<long double>(dx)*dx);
            std::int32_t bits;std::memcpy(&bits,&stored,4);
            if(!std::isfinite(stored))throw std::runtime_error("Non-finite formation smoothing");
            const float length=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(stored)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(stored)));
            long double x=dx;
            if(std::abs(static_cast<long double>(stored)-1)>=9.99999993922529e-09f){
                if(stored>=9.99999993922529e-09f){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));const auto rounded=static_cast<float>(inverse);
                    x*=inverse;dy=static_cast<float>(static_cast<long double>(rounded)*0.f);dz=static_cast<float>(static_cast<long double>(rounded)*dz);
                }else if(x!=0)x=x<0?-1:1;else if(dz!=0)dz=dz<0?-1.f:1.f;else dy=1;
            }
            float step=length<=speed?length:speed;if(length>5.f)step=static_cast<float>(static_cast<long double>(length)-5.f);
            const float zstep=static_cast<float>(static_cast<long double>(dz)*step);
            old[0]=static_cast<float>(x*step+old[0]);old[1]=static_cast<float>(static_cast<long double>(step)*dy+old[1]);
            old[2]=static_cast<float>(static_cast<long double>(zstep)+old[2]);s.vector(0x30,old);
        }
    }
    void separate_members(){ //00598550, reverse records and reverse first close peer; project after displacement.
        for(unsigned n=6;n;){--n;auto& s=slot(n);
            if(s.member==leader_){(void)project(n);continue;}
            if(!s.member||s.words.template get<std::uint8_t>(0x7c))continue;
            for(unsigned other=6;other;){--other;const auto& peer=slot(other);
                if(!peer.member||peer.member==s.member)continue;
                auto delta=difference(s.vector(0x20),peer.vector(0x20));
                if(magnitude(delta)>=1.2000000476837158f)continue;
                delta=normalize(delta);auto position=peer.vector(0x20);
                for(unsigned axis=0;axis!=3;++axis){
                    if(axis)delta[axis]=static_cast<float>(static_cast<long double>(delta[axis])*1.2000000476837158f);
                    position[axis]=axis?static_cast<float>(static_cast<long double>(delta[axis])+position[axis]):static_cast<float>(static_cast<long double>(delta[axis])*1.2000000476837158f+position[axis]);
                }
                position[3]=s.vector(0x20)[3]; //Native unwritten temporary W: preserve guard.
                s.vector(0x20,position);s.words.set(0x88,std::uint8_t{1});s.words.set(0x7c,std::uint8_t{1});break;
            }
            (void)project(n);
        }
    }
    void avoid_leader(unsigned index,float amount){ //00598810, actual second FLOAT stack argument.
        if(!leader_)return;
        auto& s=slot(index);if(s.member==leader_)return;
        if(!s.member)throw std::logic_error("Formation leader avoidance requires actual member");
        auto velocity=leader_->activation.vector1b0;auto other=s.member->activation.vector1b0;
        if(magnitude(other,2)>0&&magnitude(velocity,2)>0){
            velocity=normalize(velocity,2);other=normalize(other,2);
            const auto dot=(static_cast<long double>(velocity[0])*other[0]+static_cast<long double>(velocity[1])*other[1])+static_cast<long double>(velocity[2])*other[2];
            if(dot>.10000000149011612f)return;
        }else velocity=normalize(velocity,2);
        const auto world=frames_.actor_world_matrix(leader_->base().frame());
        const auto old=s.vector(0x20);const std::array<long double,3> delta{{static_cast<long double>(old[0])-world[12],static_cast<long double>(old[1])-world[13],static_cast<long double>(old[2])-world[14]}};
        const auto fresh=leader_->activation.vector1b0;
        const HumanVector cross{{static_cast<float>(fresh[1]*delta[2]-fresh[2]*delta[1]),
            static_cast<float>(fresh[2]*delta[0]-delta[2]*fresh[0]),static_cast<float>(delta[1]*fresh[0]-fresh[1]*delta[0]),0}};
        long double x=static_cast<long double>(fresh[2])*cross[1]-static_cast<long double>(fresh[1])*cross[2];
        float y=static_cast<float>(static_cast<long double>(cross[2])*fresh[0]-static_cast<long double>(fresh[2])*cross[0]);
        auto extended_z=static_cast<long double>(fresh[1])*cross[0]-static_cast<long double>(cross[1])*fresh[0];float z=static_cast<float>(extended_z);
        const auto normalize_away=[&]{ //598DAA/598EEB FST Z, X remains in x87 through both passes.
            const auto length=(extended_z*z+static_cast<long double>(y)*y)+x*x;const auto stored=static_cast<float>(length);
            if(!std::isfinite(length)||!std::isfinite(stored))throw std::runtime_error("Non-finite formation avoidance direction");
            if(std::abs(length-1)>=9.99999993922529e-09f){
                if(stored>=9.99999993922529e-09f){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;
                    y=static_cast<float>(y*inverse);z=static_cast<float>(z*inverse);
                }else if(x!=0)x=x<0?-1:1;else if(z!=0)z=z<0?-1.f:1.f;else y=y<0?-1.f:1.f;
            }
        };
        normalize_away();x+=static_cast<long double>(velocity[0])*.15000000596046448f;
        const float vy=static_cast<float>(static_cast<long double>(velocity[1])*.15000000596046448f),vz=static_cast<float>(static_cast<long double>(velocity[2])*.15000000596046448f);
        y=static_cast<float>(static_cast<long double>(vy)+y);extended_z=static_cast<long double>(vz)+z;z=static_cast<float>(extended_z);normalize_away();
        auto position=s.vector(0x20);position[0]=static_cast<float>(x*amount+position[0]);position[1]=static_cast<float>(static_cast<long double>(y)*amount+position[1]);
        const float zstep=static_cast<float>(static_cast<long double>(z)*amount);position[2]=static_cast<float>(static_cast<long double>(zstep)+position[2]);
        s.vector(0x20,position);s.words.set(0x88,std::uint8_t{1});s.words.set(0x7c,std::uint8_t{1});
    }
    template<class Runtime>void advance_ordinary(Runtime& runtime){ //005990C0.
        auto world=frames_.actor_world_matrix(leader_->base().frame());const HumanVector position{{world[12],world[13],world[14],world[15]}};
        auto velocity=leader_->activation.vector1b0;std::int32_t leader_index=0;(void)contains(leader_,&leader_index);
        auto planar=velocity;planar[1]=0;
        if(magnitude(planar)==0){if(!changed_)return;velocity=last_direction_;velocity[1]=0;
            if(magnitude(velocity)==0)velocity=runtime.formation_target_direction(*leader_);}
        velocity[1]=0;velocity=normalize(velocity,3);
        world=frames_.actor_world_matrix(leader_->base().frame());frames_.formation_matrix_direction(world,velocity);last_direction_=velocity;
        for(unsigned n=6;n;){--n;auto& s=slot(n);if(!s.member)continue;
            std::int32_t health;const auto bits=s.member->properties.stats_ec[0];std::memcpy(&health,&bits,4);if(health<=0)continue;
            auto ideal=frames_.formation_transform(s.vector(0x10),world);
            const auto* fresh=frames_.actor_world_matrix_pointer(leader_->base().frame());
            auto direction=frames_.formation_transform(s.vector(0x60),world);
            direction=normalize(difference(direction,{{(*fresh)[12],(*fresh)[13],(*fresh)[14],(*fresh)[15]}}),1);direction[3]=s.vector(0x40)[3];s.vector(0x40,direction);
            if(!runtime.human_is_player(*s.member))project_ordinary_route(position,ideal);
            const auto old=s.vector(0x20);const auto distance=magnitude(difference(old,position));s.words.set(0x7c,std::uint8_t{0});
            if(distance<1.2000000476837158f){auto ahead=position;
                for(unsigned axis=0;axis!=3;++axis){const float step=static_cast<float>(static_cast<long double>(velocity[axis])*.0010000000474974513f);ahead[axis]=static_cast<float>(static_cast<long double>(step)+ahead[axis]);}
                if(magnitude(difference(old,ahead))<distance)avoid_leader(n,1.2000000476837158f);
            }
            bool publish=changed_;
            if(!publish){auto delta=difference(old,ideal);
                for(unsigned axis=0;axis!=3;++axis){const float step=static_cast<float>(static_cast<long double>(velocity[axis])*.009999999776482582f);delta[axis]=static_cast<float>(static_cast<long double>(step)+delta[axis]);}
                publish=magnitude(delta)<distance;
            }
            if(publish){ideal[3]=old[3];s.vector(0x20,ideal);s.words.set(0x88,std::uint8_t{1});}
            if(square(leader_->activation.vector1b0,1)!=0&&!subgroup_)advance_member_route(n);
        }
        separate_members();const auto point=slot(0).words.template get<std::uint16_t>(0x7e);if(point!=0xffff)(void)navigation_.record(point);
        changed_=false;
    }
    void project_ordinary_route(const HumanVector& leader,HumanVector& ideal){ //005990C0's temporary route, mask0.
        const auto distance=magnitude(difference(ideal,leader),3);
        const auto first=navigation_.nearest_volume({{leader[0],leader[1],leader[2]}},5,-1);
        const auto last=navigation_.nearest_volume({{ideal[0],ideal[1],ideal[2]}},5,-1);
        std::vector<std::uint16_t> route;if(first==0xffff||last==0xffff||!navigation_.find_route(first,last,0,route))return;
        if(!route.empty()&&route.front()==first)route.erase(route.begin());
        float accumulated=0;auto previous=leader;
        for(auto point:route){const auto* record=navigation_.record(point);if(!record)return;
            const auto p=navigation_.position(point);const HumanVector current{{p[0],p[1],p[2],0}};
            accumulated=static_cast<float>(static_cast<long double>(magnitude(difference(previous,current),1))+accumulated);
            const float radius=static_cast<unsigned char>(record->prefix[0x13])*.0625f;
            if(distance<=static_cast<long double>(radius)+accumulated){
                if(magnitude(difference(current,ideal),1)>radius){
                    const auto delta=normalize(difference(leader,current),3);
                    const float offset=static_cast<long double>(accumulated)-radius<=distance?static_cast<float>(static_cast<long double>(accumulated)-distance):radius;
                    for(unsigned axis=0;axis!=3;++axis){const float step=static_cast<float>(static_cast<long double>(delta[axis])*offset);ideal[axis]=static_cast<float>(static_cast<long double>(step)+current[axis]);}
                }
                return;
            }
            previous=current;
        }
    }
    void advance_member_route(unsigned index){ //0059AF30, same navigation and slot90 cached route owner.
        if(!index)return;
        auto& s=slot(index);if(!s.member)return;
        const auto member=s.words.template get<std::uint16_t>(0x7e),leader=slot(0).words.template get<std::uint16_t>(0x7e);
        if(member==0xffff||leader==0xffff)return;
        if(s.words.template get<std::uint16_t>(0x9c)==0xffff||slot(0).words.template get<std::uint16_t>(0x9c)==0xffff||
           s.words.template get<std::uint16_t>(0x9c)!=member||slot(0).words.template get<std::uint16_t>(0x9c)!=leader){
            s.words.set(0x9c,member);slot(0).words.set(0x9c,leader);s.indices.clear();
            if(!navigation_.find_route(leader,member,0x200,s.indices))return;
        }
        float remaining=s.words.template get<float>(0x78);
        const auto point=[&](std::uint16_t index){const auto* record=navigation_.record(index);if(!record)throw std::runtime_error("Formation cached route references an absent actual navigation point");
            const auto p=navigation_.position(index);return HumanVector{{p[0],p[1],p[2],0}};};
        if(s.indices.size()>1){const auto next=point(s.indices[1]);remaining=static_cast<float>(static_cast<long double>(remaining)-magnitude(difference(slot(0).vector(0x20),next),2));
            if(remaining<0){(void)point(s.indices[1]);return;}}
        if(remaining<0)return;
        for(std::size_t n=2;n+1<s.indices.size();++n){const auto previous=point(s.indices[n-1]);const auto next=point(s.indices[n]);
            const float after=static_cast<float>(static_cast<long double>(remaining)-magnitude(difference(next,previous),2));
            if(after>=0){remaining=after;continue;}
            const auto current=point(s.indices[n]);auto direction=difference(n==s.indices.size()-2?s.vector(0x20):point(s.indices[n+1]),current);
            direction=normalize(direction,2);auto position=s.vector(0x20);
            for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(direction[axis])*remaining+current[axis]);
            s.vector(0x20,position);s.words.set(0x88,std::uint8_t{1});remaining=after;break;
        }
        if(s.indices.size()>1){const auto penultimate=point(s.indices[s.indices.size()-2]);
            if(static_cast<long double>(remaining)-magnitude(difference(s.vector(0x20),penultimate),2)<0)(void)point(s.indices[1]);}
    }
    template<class Actor,class Runtime>void assign_attached_actor(Actor* actor,Runtime& runtime){ //005957D0(nonNULL).
        const auto count=runtime.formation_seat_count(*actor);
        std::vector<HumanVector> positions,directions;std::vector<std::int32_t> seats;
        for(auto n=count;n;){--n;HumanVector position{},direction{};
            if(!runtime.formation_seat_pose(*actor,n,position,direction))continue;
            positions.push_back(position);directions.push_back(direction);seats.push_back(static_cast<std::int32_t>(n));
        }
        if(seats.empty())return;
        std::size_t selected=0;
        for(unsigned n=0;n!=6;++n){auto& s=slot(n);
            if(s.words.template get<std::int32_t>(0x84)!=-1||!s.member||s.member==leader_)continue;
            while(selected<count&&runtime.formation_seat_occupied(*actor,selected))++selected;
            if(selected==count){
                if(!count)throw std::runtime_error("Attached formation exhausted an empty actual seat owner");
                --selected;std::size_t candidate=count;
                while(candidate){--candidate;if(!runtime.formation_seat_occupied(*actor,candidate))break;}
                selected=candidate;HumanVector position{},direction{};
                const bool available=runtime.formation_seat_pose(*actor,selected,position,direction);
                //Original type20 fallback returns on successful paired query; Car returns on failed query.
                if(actor->base().type()==4?!available:actor->base().type()==20&&available)return;
                for(unsigned rest=n;rest!=6;++rest){auto& target=slot(rest);target.vector(0x20,position);target.vector(0x40,direction);target.vector(0x30,position);
                    target.words.set(0x88,std::uint8_t{1});(void)project(rest);}
                return;
            }
            if(s.member->field160){const auto kind=s.member->field160->base().type();if(kind==4||kind==20||kind==21||kind==34)continue;}
            std::size_t nearest=0;float best=std::numeric_limits<float>::max();
            for(auto candidate=positions.size();candidate;){--candidate;const auto distance=static_cast<float>(square(difference(positions[candidate],s.member->base().world_position())));
                if(distance<best){best=distance;nearest=candidate;}}
            s.vector(0x20,positions[nearest]);s.vector(0x40,directions[nearest]);s.vector(0x30,s.vector(0x20));
            s.words.set(0x80,static_cast<std::uint32_t>(selected));s.words.set(0x84,actor->base().id10());s.words.set(0x88,std::uint8_t{1});
            (void)project(n);++selected;
        }
    }
    template<class Actor,class Runtime>void advance_attached_actor(Actor* actor,Runtime& runtime){
        for(unsigned n=0;n!=6;++n)slot(n).words.set(0x84,std::int32_t{-1}); //Before actor family dispatch, including absent marker paths.
        if(!actor)throw std::logic_error("Attached formation has no actual leader attachment");
        if(actor->base().type()==4){assign_attached_actor(actor,runtime);return;}
        if(actor->base().type()!=20)return;
        assign_attached_actor(actor,runtime);
        const auto& linked=runtime.formation_linked_actors(*actor);
        for(auto n=linked.size();n;){auto* child=linked[--n];if(child!=actor)assign_attached_actor(child,runtime);}
    }
    void advance_navigation(){ //0059A2E0, actual84 point and reverse first link of kind1/2.
        const auto* record=navigation_.record(navigation_id_);if(!record)throw std::runtime_error("Formation navigation84 references no actual record");
        const auto count=record->flags&0x3fu;
        const auto* links=static_cast<const unsigned char*>(record->payload);std::uint16_t neighbour=0xffff;
        if(count&&!links)throw std::runtime_error("Formation navigation link payload is absent");
        for(auto n=count;n;){--n;std::uint16_t candidate;std::memcpy(&candidate,links+n*4,2);const auto* next=navigation_.record(candidate);
            if(!next)throw std::runtime_error("Formation navigation link names absent record");
            if(next->byte16==1||next->byte16==2){neighbour=candidate;break;}}
        if(neighbour==0xffff)throw std::runtime_error("Formation navigation branch has no original required neighbour"); //Native NULL dereference rejected.
        const auto first=navigation_.position(navigation_id_),last=navigation_.position(neighbour);
        const HumanVector a{{first[0],first[1],first[2],0}},b{{last[0],last[1],last[2],0}};
        const auto forward=normalize(difference(b,a),3);
        const HumanVector side{{static_cast<float>(static_cast<long double>(forward[1])*0-forward[2]),
            static_cast<float>(static_cast<long double>(forward[2])*0-static_cast<long double>(forward[0])*0),
            static_cast<float>(static_cast<long double>(forward[0])-static_cast<long double>(forward[1])*0),0}};
        for(unsigned n=6;n;){--n;auto& s=slot(n);if(!s.member||s.member==leader_)continue;
            auto position=s.vector(0x20),direction=s.vector(0x40);bool write=false;
            if(n==1||n==2){const auto& base=navigation_stage_<(n==1?0:1)?a:b;
                for(unsigned axis=0;axis!=3;++axis)position[axis]=n==1?static_cast<float>(static_cast<long double>(side[axis])+base[axis]):static_cast<float>(static_cast<long double>(base[axis])-side[axis]);
                direction=forward;if(navigation_stage_<(n==1?0:1))for(unsigned axis=0;axis!=3;++axis)direction[axis]=-direction[axis];write=true;
            }else if(n==3){for(unsigned axis=0;axis!=3;++axis)position[axis]=navigation_stage_<2?static_cast<float>((static_cast<long double>(a[axis])-forward[axis])+side[axis]):static_cast<float>(static_cast<long double>(forward[axis])+b[axis]);direction=forward;write=true;}
            if(write){position[3]=s.vector(0x20)[3];direction=normalize(direction,1);direction[3]=s.vector(0x40)[3];s.vector(0x20,position);s.vector(0x40,direction);}
            s.words.set(0x88,std::uint8_t{1}); //Also records4/5 and0, as in original.
        }
        separate_members();
        if(navigation_stage_!=-1){for(unsigned n=6;n;){--n;auto& s=slot(n);if(!s.member||s.member==leader_||navigation_stage_!=static_cast<std::int32_t>(n)-1)continue;
                if(square(s.member->activation.vector1b0,1)<.009999999776482582f&&square(difference(s.member->base().world_position(),s.vector(0x20)))<.5f)++navigation_stage_;
                return;}
        }else {navigation_stage_=0;for(unsigned n=6;n;){--n;auto& s=slot(n);if(!s.member||s.member==leader_)continue;
                if(square(s.member->activation.vector1b0,1)>.009999999776482582f||square(difference(s.member->base().world_position(),s.vector(0x20)),2)>=.5f){navigation_stage_=-1;break;}}}
    }
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
    std::unique_ptr<std::array<Slot,6>> slots_;Human* leader_=nullptr;std::int32_t mode_=3,count_=0,navigation_state_=0;
    std::uint16_t navigation_id_=0xffff;bool changed_=false,subgroup_=false;HumanVector last_direction_{};std::int32_t navigation_stage_=-1; //005949C0/00594A50 native88, separate from state80.
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
