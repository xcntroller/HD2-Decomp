#ifndef HD2_GAME_EFFECTS_WEATHER_UPDATE_H
#define HD2_GAME_EFFECTS_WEATHER_UPDATE_H
#include "game/fowl_bird_random.h"
#include "game/human_joint_owners.h"

namespace hd2::game {
//00487240 weather tail. Same process BSS counters, CRT stream, collision tree,
//Players collected frames and module17 emitters. Engine owns rain rendering and
//the scene+130/+134 blocker list; client owns these splash particles.
inline long double effects_weather_square_distance(const HumanVector& a,const HumanVector& b){
    const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
    return (z*z+y*y)+x*x;
}
inline bool effects_weather_in_box(const HumanVector& point,const HumanVector& low,const HumanVector& high){
    for(unsigned axis=0;axis!=3;++axis)if(!(low[axis]<=point[axis]&&point[axis]<high[axis]))return false;
    return true;
}
template<class Frame>struct EffectsWeatherBlocker{Frame frame;HumanVector centre;float radius_squared;};
template<class Process,class Frames,class World>
void update_loading_weather(std::uint32_t elapsed,Process& process,Frames& frames,World& world){
    if(!process.flag24c)return;
    process.weather_elapsed+=elapsed;
    if(fowl_signed_word(process.weather_elapsed)<100)return;
    const auto position=[&](auto frame){
        if(!frame)throw std::runtime_error("Weather requires its actual camera/Player marker frame");
        const auto matrix=frames.actor_world_matrix(frame);return HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    };
    const auto camera=position(frames.active_game_camera()); //One snapshot for ALL accumulated100ms splash periods.
    while(fowl_signed_word(process.weather_elapsed)>=100){
        const auto density=static_cast<std::uint32_t>(static_cast<std::int64_t>(static_cast<long double>(process.weather_density)*.10000000149011612f));
        for(std::uint32_t remaining=density;remaining;--remaining){
            auto start=camera;start[1]=static_cast<float>(static_cast<long double>(camera[1])+100.f);
            const float range=static_cast<float>(static_cast<long double>(process.weather_radius)*2);
            start[2]=static_cast<float>((fowl_random(world,range)-process.weather_radius)+start[2]); //Native CRT draws Z BEFORE X.
            start[0]=static_cast<float>((fowl_random(world,range)-process.weather_radius)+start[0]);
            if(const auto hit=world.weather_ground_ray(start,HumanVector{{0,-200,0,0}})){
                const bool ordinary=(hit->flags&0x10000u)==0;auto point=hit->position;
                if(!ordinary)point[1]=static_cast<float>(static_cast<long double>(point[1])+.019999999552965164f);
                (void)world.emit_human_particle(ordinary?0x2cu:0x2du,point,HumanVector{{0,1,0,0}},typename Frames::Frame{},ordinary,false);
            }
        }
        if(!--process.weather_cadence){
            process.weather_cadence=3;
            const auto root=frames.camera_root_sector();const auto fresh_camera=position(frames.active_game_camera());
            const auto epoch=frames.human_scene_epoch();
            const auto count=frames.weather_blocker_count();const auto blockers=frames.weather_blocker_array(); //Count BEFORE pointer, once per cadence.
            for(auto n=world.weather_player_count();n;){
                const auto index=static_cast<std::int32_t>(--n);auto& human=world.weather_player(index);
                if(world.weather_player_sector(human)!=root)continue;
                const auto model=human.base().frame();
                if(effects_weather_square_distance(position(model),fresh_camera)>100.f)continue;
                if(epoch!=frames.weather_frame_epoch(model))continue;
                const auto head=position(human.head_marker);bool blocked=false;
                for(auto b=count;b;){
                    const auto blocker=frames.weather_blocker(blockers,--b);
                    if(effects_weather_square_distance(head,blocker.centre)<blocker.radius_squared&&frames.weather_blocker_contains(blocker.frame,head)){
                        blocked=true;break;
                    }
                }
                if(blocked)continue;
                const auto marker=world.weather_player_random_frame(index);auto point=position(marker);
                point[1]=static_cast<float>(static_cast<long double>(point[1])+.10000000149011612f);
                (void)world.emit_human_particle(0x2c,point,HumanVector{{0,1,0,0}},marker,true,false);
            }
        }
        process.weather_elapsed-=100; //Normal tail ONLY; failures preserve native accumulated period.
    }
}
} //namespace hd2::game
#endif
