#ifndef HD2_GAME_MISSION_PARTICLE_POINT_H
#define HD2_GAME_MISSION_PARTICLE_POINT_H
#include "game/effect_definitions.h"
#include "game/human_joint_owners.h"
#include "game/human_loading_stats.h"

namespace hd2::game {
inline float particle_real(std::uint32_t word){float value;std::memcpy(&value,&word,4);return value;}
inline std::int32_t particle_integer(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
inline std::int32_t particle_truncate(long double value){return particle_integer(human_loading_stat_word(value));}
inline long double particle_square(const std::array<std::uint32_t,3>& vector){
    const auto x=particle_real(vector[0]),y=particle_real(vector[1]),z=particle_real(vector[2]);
    return (static_cast<long double>(z)*z+static_cast<long double>(y)*y)+static_cast<long double>(x)*x;
}

//0043F500/0043F520/0043F730/0043FB40. Definition and instance matrices are
//borrowed; point allocation/pooling belongs to the SAME mission process.
//Cold zero guards cover native unwritten scalar/padding lanes only. The native
//inline emitter colour_keys table is actually the sprite's four-word UV table.
struct alignas(16) MissionParticlePoint {
    HumanMatrix matrix=human_identity(); //Native00..3F, translation30 and forward20.
    std::uint32_t texture_index=0,texture_count=0;
    std::int32_t texture_time=-1,remaining=0;
    float size=0,size_rate=0,angle=0,angle_rate=0,alpha=0,alpha_rate=0,speed=0,acceleration=0,gravity=0;
    HumanVector colour{};const EffectEmitterDefinition* definition=nullptr;
    std::uint32_t phase=0,flags=0,emitter_index=0;std::array<std::uint32_t,4> uv_cache{};
    static constexpr float tiny=1.0000000168623835e-16f,seconds=.0010000000474974513f;

    template<class Frames>bool initialize(const HumanMatrix& instance,const EffectEmitterDefinition& emitter,
            std::uint32_t elapsed,std::uint32_t index,Frames& frames){ //0043F520.
        emitter_index=index;definition=&emitter;matrix=human_identity();texture_count=static_cast<std::uint32_t>(emitter.colour_keys.size());
        if(particle_square(emitter.vector50)>=tiny||particle_square(emitter.vector60)>=tiny){
            for(unsigned axis=0;axis!=3;++axis){const auto offset=(static_cast<long double>(frames.volume_random_unit())-.5f)*particle_real(emitter.vector60[axis]);
                matrix[12+axis]=static_cast<float>(offset+offset+particle_real(emitter.vector50[axis]));}
        }
        flags=emitter.field44&1u;
        if(flags)for(unsigned axis=0;axis!=3;++axis)matrix[12+axis]+=instance[12+axis];
        phase=texture_index=0;texture_time=texture_count<2?-1:particle_integer(emitter.colour_fields[1]);
        angle=jitter(emitter.fields70[6],emitter.fields70[7],frames);
        size=jitter(emitter.fields70[0],emitter.fields70[1],frames);
        alpha=jitter(emitter.fields70[4],emitter.fields70[5],frames);
        if(!select_phase(instance,frames))return false;
        speed=jitter(emitter.fields70[2],emitter.fields70[3],frames);
        if(elapsed)advance(elapsed,frames);
        return true;
    }
    template<class Frames>bool select_phase(const HumanMatrix& instance,Frames& frames){ //0043F730.
        if(!definition)throw std::logic_error("Particle phase requires its actual emitter definition");
        if(phase>=definition->points.size())return false;
        const auto& point=definition->points[phase];
        HumanVector direction{{particle_real(point.position10[0]),particle_real(point.position10[1]),particle_real(point.position10[2]),0}};
        const auto length=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2];
        if(length<tiny)direction={{0,0,1,0}};
        if(particle_square(point.velocity20)>=tiny){
            long double z=0;
            for(unsigned axis=0;axis!=3;++axis){const auto offset=(static_cast<long double>(frames.volume_random_unit())-.5f)*particle_real(point.velocity20[axis]);
                const auto result=offset+offset+direction[axis];direction[axis]=static_cast<float>(result);if(axis==2)z=result;}
            const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+z*direction[2];
            normalize(direction,square);
        }
        if(!(flags&1u))direction=frames.collision_sweep_rotate(direction,instance);
        frames.particle_point_direction(matrix,direction);
        gravity=particle_real(point.fields30[2])*seconds;
        const auto draw=static_cast<long double>(frames.volume_random_unit())-.5f;
        remaining=particle_truncate(static_cast<long double>(particle_integer(point.fields30[1]))*(draw+draw)+particle_integer(point.fields30[0]));
        if(remaining<1)remaining=1;
        const auto target_size=jitter_extended(point.fields30[5],point.fields30[6],frames);
        size_rate=static_cast<float>(((static_cast<long double>(target_size)-size)/(static_cast<long double>(remaining)*seconds))*seconds);
        auto target_alpha=jitter_extended(point.fields78[0],point.fields78[1],frames);
        if(target_alpha>1)target_alpha=1;else if(target_alpha<0)target_alpha=0;
        alpha_rate=static_cast<float>(((target_alpha-alpha)/(static_cast<long double>(remaining)*seconds))*seconds);
        acceleration=static_cast<float>(jitter_extended(point.fields78[2],point.fields78[3],frames)*seconds);
        angle_rate=static_cast<float>(jitter_extended(point.fields30[3],point.fields30[4],frames)*seconds);
        for(unsigned axis=0;axis!=3;++axis)colour[axis]=particle_real(point.vector50[axis]);
        if(particle_square(point.vector60)>=tiny){ //ONE shared, uncentred draw, unlike position/direction jitter.
            const auto colour_draw=static_cast<long double>(frames.volume_random_unit());
            colour[0]=static_cast<float>(colour_draw*particle_real(point.vector60[0])+colour[0]);
            for(unsigned axis=1;axis!=3;++axis){const float offset=static_cast<float>(colour_draw*particle_real(point.vector60[axis]));colour[axis]+=offset;}
        }
        return true;
    }
    template<class Frames>void advance(std::uint32_t elapsed,Frames& frames){ //0043FB40, wrap then signed clamp.
        if(!definition)throw std::logic_error("Particle motion requires its actual emitter definition");
        remaining=particle_integer(static_cast<std::uint32_t>(remaining)-elapsed);
        if(remaining<0){elapsed+=static_cast<std::uint32_t>(remaining);remaining=0;}
        const float milliseconds=static_cast<float>(particle_integer(elapsed));
        HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        if(gravity!=0){direction[1]=static_cast<float>(static_cast<long double>(direction[1])-static_cast<long double>(milliseconds)*gravity);frames.particle_point_direction(matrix,direction);}
        if(acceleration!=0)speed=static_cast<float>(static_cast<long double>(milliseconds)*acceleration+speed);
        for(unsigned axis=0;axis!=3;++axis){const float delta=static_cast<float>(((static_cast<long double>(direction[axis])*speed)*milliseconds)*seconds);matrix[12+axis]+=delta;}
        if(angle_rate!=0){const auto result=static_cast<long double>(milliseconds)*angle_rate+angle;angle=static_cast<float>(result);
            constexpr float turn=6.2831854820251465f;if(result<0)angle=static_cast<float>(result+turn);else if(result>turn)angle=static_cast<float>(result-turn);}
        if(size_rate!=0)size=static_cast<float>(static_cast<long double>(milliseconds)*size_rate+size);
        if(alpha_rate!=0){const auto result=static_cast<long double>(milliseconds)*alpha_rate+alpha;alpha=static_cast<float>(result);
            if(result>1)alpha=1;else if(result<0)alpha=0;}
        if(texture_time>=0){texture_time=particle_integer(static_cast<std::uint32_t>(texture_time)+elapsed);
            if(particle_integer(definition->colour_fields[1])<texture_time){
                texture_time=0;++texture_index;if(texture_index==texture_count)texture_index=definition->colour_fields[0];
                uv_cache=definition->colour_keys.at(texture_index); //Actual authored invalid index remains a failure.
            }
        }
    }
private:
    template<class Frames>static long double jitter_extended(std::uint32_t base,std::uint32_t range,Frames& frames){
        const auto offset=(static_cast<long double>(frames.volume_random_unit())-.5f)*particle_real(range);return offset+offset+particle_real(base);
    }
    template<class Frames>static float jitter(std::uint32_t base,std::uint32_t range,Frames& frames){return static_cast<float>(jitter_extended(base,range,frames));}
    static void normalize(HumanVector& value,long double length){
        constexpr float epsilon=9.99999993922529e-09f;
        if(epsilon<=std::abs(length-1.L)){
            const float stored=static_cast<float>(length);
            if(stored>=epsilon){const float inverse=static_cast<float>(1.L/std::sqrt(static_cast<long double>(stored)));for(unsigned axis=0;axis!=3;++axis)value[axis]*=inverse;}
            else{const unsigned axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
    }
};
} //namespace hd2::game
#endif
