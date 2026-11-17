#ifndef HD2_GAME_SHOT_VISUAL_FRAMES_H
#define HD2_GAME_SHOT_VISUAL_FRAMES_H
#include "game/shot_motion_frames.h"
#include "game/human_ai_shot_direction.h"
namespace hd2::game::shot_visual {
using shot_motion::stored;
inline long double square_zyx(const HumanVector& v){return (static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];}
template<class Words>HumanVector interpolate(Words& w,std::uint32_t& elapsed){
    const auto prior=w.template get<std::uint32_t>(0x50);
    if(human_frame_signed_word(prior+elapsed)>100)elapsed=100u-prior;
    const auto next=prior+elapsed;w.set(0x50,next);HumanVector delta{};
    const auto factor=static_cast<long double>(human_frame_signed_word(next))*.009999999776482582f;
    const auto displacement=w.template get<HumanVector>(0x40);
    for(unsigned a=0;a!=3;++a)delta[a]=stored(displacement[a]*factor);
    return delta;
}
template<class Record,class Frames>void place(Record& record,const HumanVector& delta,Frames& frames){
    auto point=record.words.template get<HumanVector>(0x30);
    for(unsigned a=0;a!=3;++a)point[a]=stored(static_cast<long double>(delta[a])+point[a]);
    frames.car_sound_head(record.frame(),point);
}
//0044B400. Flare emitter004458B0 toggles its emission bit; deleting the
//instance here would lose already emitted points and change retirement order.
template<class Record,class Frames,class World>void bullet(Record& record,std::uint32_t elapsed,Frames& frames,World& world){
    auto& w=record.words;if(!record.frame())return;
    const auto displacement=w.template get<HumanVector>(0x40);
    if(square_zyx(displacement)<1.0000000168623835e-16f||w.template get<std::uint8_t>(0xb8))return;
    auto direction=interpolate(w,elapsed);
    const auto factor=static_cast<long double>(human_frame_signed_word(elapsed))*.009999999776482582f;
    std::array<long double,3> step{};for(unsigned a=0;a!=3;++a)step[a]=displacement[a]*factor;
    const float square=stored((step[2]*step[2]+step[1]*step[1])+step[0]*step[0]);std::int32_t bits;std::memcpy(&bits,&square,4);
    const float length=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-09f?1.f:stored(std::sqrt(static_cast<long double>(square)));
    const float distance=stored(static_cast<long double>(length)+w.template get<float>(0x54));w.set(0x54,distance);
    if(distance<30.f){frames.object_visible(record.frame(),false);frames.update_scene_object(record.frame());return;}
    if(w.template get<std::uint8_t>(0xb0)&&record.frame()){
        const auto flare=frames.actor_find(record.frame(),"flare",0xffffu);
        if(distance<=100.f){
            if(w.template get<std::int32_t>(0xb4)==-1)w.set(0xb4,world.emit_shot_flare_particle(record.frame()));
            if(flare){
                long double amount=frames.shot_flare_intensity(flare);
                if(amount<1.f){amount+=static_cast<long double>(human_frame_signed_word(elapsed+elapsed))*.0010000000474974513f;if(amount>1.f)amount=1.f;}
                frames.shot_flare_intensity(flare,stored(amount));frames.update_scene_object(flare);
            }
        }else if(flare){
            const auto amount=static_cast<long double>(frames.shot_flare_intensity(flare))-stored(human_frame_signed_word(elapsed))*.0005000000237487257f;
            if(amount<=0){
                const auto particle=w.template get<std::int32_t>(0xb4);w.set(0xb8,std::uint8_t{1});
                if(particle!=-1){world.toggle_shot_particle_emission(particle);if((world.mode_flags()&7u)==7u&&(world.mode_flags()&1u))world.shot_flare_game_flag();}
                if(record.frame()){frames.shot_surface_scale(record.frame(),0,0,0);frames.update_scene_object(record.frame());}return;
            }
            frames.shot_flare_intensity(flare,stored(amount));frames.update_scene_object(flare);
        }
    }
    place(record,direction,frames);normalize_human_ai_shot(direction,square_zyx(direction));
    frames.shot_visual_direction(record.frame(),direction);frames.shot_surface_scale(record.frame(),1,1,length);
    frames.object_visible(record.frame(),true);frames.update_scene_object(record.frame());
}
//00449030; ordinary00451670 initializes90=false. The positive camera branch
//still requires its actual command9 owner, and88 publishes only after it returns.
template<class Record,class Frames,class World>void capsule(Record& record,std::uint32_t elapsed,Frames& frames,World& world){
    auto& w=record.words;if(!record.frame()||square_zyx(w.template get<HumanVector>(0x40))<1.0000000168623835e-16f)return;
    w.set(0x80,w.template get<std::uint32_t>(0x80)+elapsed);
    auto direction=interpolate(w,elapsed);place(record,direction,frames);normalize_human_ai_shot(direction,square_zyx(direction));
    frames.shot_visual_direction(record.frame(),direction);frames.object_visible(record.frame(),true);frames.update_scene_object(record.frame());
    if(w.template get<std::uint8_t>(0x90)&&!w.template get<std::uint8_t>(0x88)&&record.source&&world.shot_source_is_player(record.source)&&w.template get<std::int32_t>(0x80)>300){
        world.begin_capsule_camera(record.frame());w.set(0x88,std::uint8_t{1});
    }
}
} //namespace hd2::game::shot_visual
#endif
