#ifndef HD2_GAME_HUMAN_PLAYER_COVER_MOVE_H
#define HD2_GAME_HUMAN_PLAYER_COVER_MOVE_H
#include "game/human_active_ai_retreat.h"
#include <functional>
namespace hd2::game {
//004595F0 writes XYZ into real BSS89ABD0; fourth lanes remain zero.
inline constexpr std::array<HumanVector,5> player_cover_move_offsets{{
    {{-10,0,15,0}},{{10,0,15,0}},{{4,0,3,0}},{{-10,0,-6,0}},{{10,0,-6,0}}
}};
//005245C0 and CRT005244C0: first low set bit, zero uses row0. Diagonals
//are deliberately not normalized until the containing00459740 call.
inline HumanVector player_cover_octant(std::uint8_t mask){
    constexpr std::array<HumanVector,8> table{{{{0,0,1,0}},{{1,0,1,0}},{{1,0,0,0}},{{1,0,-1,0}},{{0,0,-1,0}},{{-1,0,-1,0}},{{-1,0,0,0}},{{-1,0,1,0}}}};
    for(unsigned n=0;n<8;++n)if(mask&(1u<<n))return table[n];
    return table[0];
}
//00537A40: the near test compares an extended sum against a DOUBLE .04.
//Its refusal still emits speech; no task state is reset on that return.
template<class AI,class Frames,class World>void start_active_player_cover_move(AI& ai,const HumanVector& destination,const HumanVector& direction,bool immediate,std::uint32_t posture,Frames& frames,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    const auto own=human_ai_cover::actor_position(frames,*ai.human());validate();
    const auto x=static_cast<long double>(destination[0])-own[0],y=static_cast<long double>(destination[1])-own[1],z=static_cast<long double>(destination[2])-own[2];
    if((z*z+x*x)+y*y<.04){world.queue_active_ai_speech(ai,immediate?0x69u:0x68u,false,300u,false);validate();return;}
    auto& state=ai.state();state.set(0x190,0u);ai.initialize_task(nullptr,41u,destination,direction,true,.5f);validate();
    state.set(0x1c0,.5f);state.set(0x204,std::uint8_t{1});state.set(0x209,std::uint8_t{1});state.set(0x188,std::uint8_t(immediate));
    state.set(0x160,direction);state.set(0x218,posture);ai.pending_first_mode(immediate?9u:8u);validate();
    notify_human_active_ai_empty_orders(ai,world);validate();select_active_group_move_posture(ai,frames,world);validate();
}
namespace player_cover_detail {
inline float radius(const MissionTableRecord& point){return static_cast<unsigned char>(point.prefix[0x13])*.0625f;}
inline bool has_cover(const MissionTableRecord& point){return (point.reserved17[0]|point.reserved17[1]|point.reserved17[2])!=0;}
inline bool dark(const MissionTableRecord& point){float shadowed,plain;return MissionNavigation::light_values(point,0,shadowed,plain)&&shadowed<.2f;}
inline HumanVector adjusted(const MissionTableRecord& point,const HumanVector& direction,bool spill){
    auto result=human_ai_cover::position(point);const auto r=radius(point);
    for(unsigned n=0;n<3;++n){auto product=static_cast<long double>(direction[n])*r;if(spill)product=static_cast<float>(product);result[n]=static_cast<float>(result[n]+product);}return result;
}
inline long double extended_distance(const HumanVector& a,const HumanVector& b){const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];return (x*x+y*y)+z*z;}
//Fallback keeps its X difference extended through the inverse multiplication.
inline HumanVector fallback_look(const HumanVector& target,const HumanVector& own){
    const auto x=static_cast<long double>(target[0])-own[0];auto result=human_ai_cover::difference(target,own);
    const auto square=(x*x+static_cast<long double>(result[1])*result[1])+static_cast<long double>(result[2])*result[2];
    if(std::abs(square-1)<9.99999993922529e-09f)return result;
    const float stored=static_cast<float>(square);
    if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));result[0]=static_cast<float>(x*inverse);for(unsigned n=1;n<3;++n)result[n]=static_cast<float>(result[n]*inverse);}
    else {const auto axis=x!=0?0u:result[2]!=0?2u:1u;result[axis]=(axis==0?x:result[axis])<0?-1.f:1.f;}
    return result;
}
}
//00459740: actual first navigation table, Scene sector, geometry ray and
//current Human B0 borrowers. Local scratch owns no actors or navigation points.
template<class Command,class Frames,class World>void dispatch_player_cover_move(Command& command,bool immediate,Frames& frames,World& world){
    using namespace player_cover_detail;
    auto* selected=world.active_selected_player();auto recipients=world.player_command_recipients(command,4u);if(recipients.empty())return;
    if(!selected)throw std::runtime_error("Players cover move reaches native NULL selected Human");
    if(recipients.size()>player_cover_move_offsets.size())throw std::runtime_error("Players cover move exceeds native five-slot storage");
    const auto selected_live=world.active_event_actor_guard(*selected);
    std::vector<std::function<void()>> loans;for(auto* human:recipients){if(!human)throw std::runtime_error("Players cover move reaches native NULL recipient");loans.emplace_back(world.active_event_actor_guard(*human));}
    selected_live();const auto own=human_ai_cover::actor_position(frames,*selected);selected_live();
    auto direction=command.position();direction[1]=0;normalize_human_ai_shot(direction,static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2]);
    auto matrix=human_identity();frames.formation_matrix_direction(matrix,direction);
    auto centre=own;for(unsigned n=0;n<3;++n){const float product=static_cast<float>(static_cast<long double>(direction[n])*2.f);centre[n]=static_cast<float>(static_cast<long double>(own[n])+product);matrix[12+n]=centre[n];}
    const auto count=recipients.size();std::array<HumanVector,5> ideals{};
    for(auto n=count;n;){--n;ideals[n]=frames.formation_transform(player_cover_move_offsets[n],matrix);}
    selected_live();const auto sector=frames.door_sector(human_ai_cover::actor_position(frames,*selected));selected_live();
    auto& navigation=world.active_task_navigation();const auto* reference=navigation.record(navigation.nearest_volume({{centre[0],centre[1],centre[2]}},10.f,-1.f));
    struct Slot {const MissionTableRecord* point=nullptr;float distance=10000.f;};std::array<Slot,5> slots{};
    std::vector<std::uint16_t> nearby;std::vector<const MissionTableRecord*> extras;
    navigation.append_spatial_cover({{centre[0],centre[1],centre[2]}},20.f,nearby);
    const auto octant=human_ai_cover::direction(direction);
    for(auto n=nearby.size();n;){const auto* point=navigation.record(nearby[--n]);if(!point)throw std::runtime_error("Players cover candidate lost native navigation point");
        const bool unlit=dark(*point);if(!unlit&&(!has_cover(*point)||!(human_ai_cover::blocked(*point,octant,7u)&1u)))continue;
        const auto position=human_ai_cover::position(*point);if(frames.door_sector(position)!=sector)continue;
        if(!unlit){auto start=position;start[1]=static_cast<float>(static_cast<long double>(start[1])+.3f);auto travel=direction;const auto r=radius(*point);
            travel[0]=static_cast<float>(static_cast<long double>(direction[0])*r+static_cast<long double>(direction[0])*.7f);
            for(unsigned k=1;k<3;++k){const float a=static_cast<float>(static_cast<long double>(direction[k])*r),b=static_cast<float>(static_cast<long double>(direction[k])*.7f);travel[k]=static_cast<float>(static_cast<long double>(a)+b);}
            const auto hit=world.human_geometry_ray(start,travel,0xffffffffu,0x10000u,true);if(!hit||hit->normal[1]>.6f)continue;
        }
        const auto metric=adjusted(*point,direction,false);
        if(!reference)throw std::runtime_error("Players cover height test reaches native NULL nearest point");
        const auto height=static_cast<long double>(human_ai_cover::position(*reference)[1])-metric[1];if(height>2.f||height< -2.f)continue;
        std::size_t best=count;float distance=10000.f;
        for(auto i=count;i;){--i;const auto square=extended_distance(ideals[i],metric);if(square<distance){best=i;distance=static_cast<float>(square);}}
        if(best==count)throw std::runtime_error("Players cover assignment reaches native invalid nearest slot");
        auto& slot=slots[best];if(!slot.point){slot={point,distance};}
        else if(!(slot.distance>distance))extras.push_back(point);
        else {extras.push_back(slot.point);slot={point,distance};}
    }
    std::vector<HumanVector> chosen;
    for(auto n=count;n;){const auto* point=slots[--n].point;if(point)chosen.push_back(adjusted(*point,direction,true));}
    if(chosen.size()<=static_cast<float>(count)*.5f){for(auto& slot:slots)slot.point=nullptr;}
    else {
        for(auto n=chosen.size();n;){const auto& position=chosen[--n];for(auto i=extras.size();i;){--i;const auto delta=human_ai_cover::difference(human_ai_cover::position(*extras[i]),position);if(human_ai_cover::square(delta)<4.f)extras.erase(extras.begin()+i);}}
        if(!extras.empty())for(auto n=count;n;){const auto slot=--n;if(slots[slot].point)continue;std::size_t best=extras.size();float distance=10000.f;
            for(auto i=extras.size();i;){--i;const auto delta=human_ai_cover::difference(ideals[slot],human_ai_cover::position(*extras[i]));const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[1])*delta[1];if(square<distance){best=i;distance=static_cast<float>(square);}}
            if(best==extras.size())throw std::runtime_error("Players extra cover reaches native invalid nearest candidate");
            slots[slot]={extras[best],distance};extras.erase(extras.begin()+best);if(extras.empty())break;
        }
        for(auto n=count;n;){const auto i=--n;if(slots[i].point)continue;const auto& ideal=ideals[i];const auto* point=navigation.record(navigation.nearest_volume({{ideal[0],ideal[1],ideal[2]}},10.f,-1.f));
            if(!point)throw std::runtime_error("Players cover fallback reaches native NULL nearest point");
            if(frames.door_sector(human_ai_cover::position(*point))==sector)slots[i].point=point;
        }
    }
    auto look_at=centre;for(unsigned n=0;n<3;++n){auto product=static_cast<long double>(direction[n])*30.f;if(n==0)product=static_cast<float>(product);look_at[n]=static_cast<float>(centre[n]+product);}
    for(std::size_t slot=0;slot<count;++slot){const auto* point=slots[slot].point;if(!point)continue;const auto position=human_ai_cover::position(*point);std::size_t best=recipients.size();float distance=10000.f;
        for(auto n=recipients.size();n;){--n;loans[n]();const auto current=human_ai_cover::actor_position(frames,*recipients[n]);loans[n]();const auto square=extended_distance(position,current);if(square<distance){best=n;distance=static_cast<float>(square);}}
        if(best==recipients.size())throw std::runtime_error("Players cover move reaches native invalid nearest Human");
        const auto destination=adjusted(*point,direction,false);HumanVector look{};const auto low=point->reserved17[1];
        if(!(low&octant))look=human_ai_cover::difference(look_at,destination);
        else {const auto x=static_cast<long double>(destination[0])-centre[0],y=static_cast<long double>(destination[1])-centre[1],z=static_cast<long double>(destination[2])-centre[2];const float side=static_cast<float>((x*direction[2]+z*(-static_cast<long double>(direction[0])))+y*0.f);auto bit=octant;
            do {bit=side>0?(bit==1?0x80:bit>>1):(bit==0x80?1:bit*2);}while((low&bit)&&bit!=octant);look=player_cover_octant(bit);
        }
        normalize_human_ai_shot(look,human_ai_cover::square_xyz(look));
        bool unlit=dark(*point);if(has_cover(*point)&&(human_ai_cover::blocked(*point,octant,7u)&1u))unlit=false;
        loans[best]();start_active_player_cover_move(world.active_command_state(*recipients[best]),destination,look,immediate,unlit?1u:2u,frames,world);loans[best]();
        recipients.erase(recipients.begin()+best);loans.erase(loans.begin()+best);
    }
    for(auto n=recipients.size();n;){--n;loans[n]();const auto position=human_ai_cover::actor_position(frames,*recipients[n]);loans[n]();const auto look=fallback_look(look_at,position);
        auto& ai=world.active_command_state(*recipients[n]);const auto draw=world.human_frame_random_word();loans[n]();start_active_player_cover_move(ai,position,look,immediate,draw%3?4u:2u,frames,world);loans[n]();
    }
}
}
#endif
