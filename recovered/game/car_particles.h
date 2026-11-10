#ifndef HD2_GAME_CAR_PARTICLES_H
#define HD2_GAME_CAR_PARTICLES_H

#include "game/car_contact_callbacks.h"
#include <string>
#include <string_view>

namespace hd2::game {

//0089BC40's per-frame pool entries use real native visual references. The
//240-byte particle table at0097B89C remains serialized, separate from this pool.
struct CarParticleDefinition {
    std::array<unsigned char,0xf0> bytes{};
    std::uint32_t word(unsigned offset) const {
        if(offset>bytes.size()-4)throw std::out_of_range("Particle definition word");
        std::uint32_t result;std::memcpy(&result,bytes.data()+offset,4);return result;
    }
    float scalar(unsigned offset) const {const auto value=word(offset);float result;std::memcpy(&result,&value,4);return result;}
    std::string text(unsigned offset,unsigned limit) const {
        if(offset>bytes.size()||limit>bytes.size()-offset)throw std::out_of_range("Particle definition text");
        const auto begin=reinterpret_cast<const char*>(bytes.data()+offset);
        const auto end=static_cast<const char*>(std::memchr(begin,0,limit));
        if(!end)throw std::runtime_error("Unterminated particle definition name");
        return std::string(begin,end);
    }
    CarContactVector vector(unsigned offset) const {
        return {{scalar(offset),scalar(offset+4),scalar(offset+8),scalar(offset+12)}};
    }
};
template<class Frame> struct CarParticleState {
    CarContactVector vector0{},vector10{};
    Frame visual{};
    std::uint32_t definition=0;
    bool active=false,pending=false;
};
template<class Frame>bool retire_car_particle(std::vector<CarParticleState<Frame>>& pool,std::int32_t index){ //004ECFC0.
    if(index<0||static_cast<std::size_t>(index)>=pool.size())return false;
    pool[static_cast<std::size_t>(index)].pending=true;return true;
}
struct CarParticleRegion {
    CarContactVector bounds;
    std::array<std::uint32_t,9> controls{{1,1,1,0,2000,0,0,0,8}};
};

//004D5C87..004D6094 /004D6357..004D6811 preserve the same property order.
//The new-texture callback alone sets property34 from table+AC.
template<class Backend,class Frame>
void configure_car_particle(Backend& backend,CarParticleState<Frame>& state,
        const CarParticleDefinition& definition,bool new_texture) {
    auto set=[&](unsigned property,std::uint32_t value){backend.car_particle_property(state.visual,property,value);};
    set(0x14,0);set(0x13,definition.word(0x98)&0x107);
    for(unsigned p=0;p<6;++p)set(p,definition.word(0x70+p*4));
    set(0x1a,definition.word(0xd4));set(0xb,definition.word(0xa4));set(0xc,definition.word(0xa8));
    if(new_texture)set(0x22,definition.word(0xac));
    set(6,definition.word(0x88));set(7,static_cast<std::uint32_t>(car_contact_integer(definition.scalar(0x8c))));
    for(unsigned p=0;p<3;++p)set(0xd+p,definition.word(0xb0+p*4));
    set(0x15,1);
    //Original XYZ assignments leave each native vector's fourth lane unwritten.
    for(unsigned p=0;p<3;++p)state.vector0[p]=definition.scalar(0xb0+p*4);
    for(unsigned p=0;p<3;++p)set(0x10+p,definition.word(0xbc+p*4));
    set(0x16,1);
    for(unsigned p=0;p<3;++p)state.vector10[p]=definition.scalar(0xbc+p*4);
    set(0x18,definition.word(0xc8));set(0x19,definition.word(0xcc));
    set(8,definition.word(0x90));set(9,definition.word(0x94));set(0xa,definition.word(0xa0));
    backend.car_particle_vector_property(state.visual,0x1d,definition.vector(0x40));
    backend.car_particle_vector_property(state.visual,0x1e,definition.vector(0x50));
}

template<class Backend,class Frame>
void select_car_particle_region(Backend& backend,Frame visual,const CarContactVector& selected) {
    //1001BAF0 assigns emitter+204; it selects a region, without deleting it.
    //Original visits all matches backwards, leaving the lowest matching index.
    for(auto region=backend.car_particle_region_count(visual);region!=0;) {
        const auto current=backend.car_particle_region(visual,--region);
        if(current[0]==selected[0]&&current[1]==selected[1]&&current[2]==selected[2]&&current[3]==selected[3])
            backend.select_car_particle_region(visual,region);
    }
}

template<class Backend,class Frame>
bool initialize_car_particle(Backend& backend,CarParticleState<Frame>& state,
        const std::vector<CarParticleDefinition>& definitions,std::uint32_t index) { //004D5B10,6BC.
    if(index>=definitions.size())return false;
    const auto& selected=definitions[index];
    const auto secondary=selected.text(0x20,0x20);
    const auto primary=selected.text(0,0x20);
    //Graphics+3C(primary, optional secondary, 2/4, texture-out); acquired texture
    //is released after final update and automatically on native endpoint failure.
    auto texture=backend.load_car_particle_texture(primary,secondary.size()>=4?secondary:std::string{},secondary.size()>=4?4u:2u);
    if(!texture)return false;
    if(!state.visual)throw std::runtime_error("Car particle has no actual visual");
    state.definition=index;
    backend.car_particle_texture(state.visual,texture.get());
    backend.car_particle_name(state.visual,"Car_particle");
    backend.link_scene_object(state.visual,Frame{});
    backend.car_particle_unit_scale(state.visual);
    backend.car_direction(state.visual,CarContactVector{{0,0,1,0}});
    backend.update_scene_object(state.visual);
    backend.reset_car_particle(state.visual);
    configure_car_particle(backend,state,selected,true);
    for(auto region=backend.car_particle_region_count(state.visual);region!=0;)
        backend.clear_car_particle_region(state.visual,--region); //+C8 removes records; +C0 only selects a region.
    for(auto n=definitions.size();n!=0;) {
        const auto& candidate=definitions[--n];
        if(candidate.text(0,0x20)==primary)
            backend.add_car_particle_region(state.visual,CarParticleRegion{candidate.vector(0x60)},-1);
    }
    select_car_particle_region(backend,state.visual,selected.vector(0x60));
    backend.update_scene_object(state.visual);
    return true;
}

template<class Backend,class Frame>
std::int32_t reuse_car_particle(Backend& backend,std::vector<CarParticleState<Frame>>& pool,
        const std::vector<CarParticleDefinition>& definitions,std::uint32_t slot,std::uint32_t index) { //004D62C0,6C0.
    if(index>=definitions.size())return -1;
    auto& state=pool.at(slot);if(!state.visual)throw std::runtime_error("Car particle pool slot has no actual visual");
    const auto& selected=definitions[index];
    if(backend.car_particle_texture_name(state.visual)!=selected.text(0,0x20))return -1;
    state.definition=index;
    configure_car_particle(backend,state,selected,false);
    select_car_particle_region(backend,state.visual,selected.vector(0x60));
    backend.update_scene_object(state.visual);
    state.active=true;state.pending=false;
    backend.activate_car_particle(state.visual,true);
    if(slot>0x7fffffff)throw std::runtime_error("Car particle slot exceeds original signed result");
    return static_cast<std::int32_t>(slot);
}

} //namespace hd2::game
#endif
