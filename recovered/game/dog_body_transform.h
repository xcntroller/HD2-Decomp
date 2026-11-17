#ifndef HD2_GAME_DOG_BODY_TRANSFORM_H
#define HD2_GAME_DOG_BODY_TRANSFORM_H
#include "game/dog_body_actions.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_animation_requests.h"
namespace hd2::game {
//Position/interpolation portion of actual Body=Actor+90. flags3E0 is borrowed
//from the SAME Body action owner; no mirrored movement flags or actor pose.
class DogBodyTransform {
public:
    using Vector=std::array<float,4>;
    explicit DogBodyTransform(DogBodyActions& actions):actions_(actions){}
    std::optional<Vector> previous390,reference3a0,position3b0;
    Vector displacement3d0{};bool requested3c0=false;
    std::int32_t interval38c=-1;
    void initialize_known_position(const Vector& value){ //005C7D30 stores390/3B0, NOT3A0.
        previous390=position3b0=value;for(unsigned n=0;n!=3;++n)displacement3d0[n]=0;requested3c0=false;interval38c=-1;
    }
    void initialize_bootstrap_position(const Vector& cached_model_position){
        //Composite005C7D30→005C0F50:3E0=0,390=3B0=cached model XYZ.
        //First distance is exactly0. Either reference-selection branch writes
        //that SAME position (the model world matrix is already cached and no
        //intervening native call can invalidate it). The old unwritten3A0
        //therefore cannot affect any defined output; no cold value is invented.
        if(actions_.flags3e0)throw std::logic_error("Dog bootstrap position requires actual zero3E0 producer");
        for(unsigned n=0;n!=3;++n)if(!std::isfinite(cached_model_position[n]))throw std::runtime_error("Dog bootstrap requires actual finite model coordinates");
        initialize_known_position(cached_model_position);requested3c0=true;reference3a0=cached_model_position;
    }
    template<class Actor,class Frames>void queue_position(const Vector& value,Actor& actor,Frames& frames){ //005C0D10 snapshots before clearing bit2.
        const auto copy=value;actions_.flags3e0&=~2u;request_position(copy,actor,frames);
    }
    template<class Actor,class Frames>void request_position(const Vector& value,Actor& actor,Frames& frames){ //005C0F50, one vector pointer.
        requested3c0=true;position3b0=value;
        const auto& previous=required(previous390,"Dog position requires actual Body390 producer");
        for(unsigned n=0;n!=3;++n)displacement3d0[n]=static_cast<float>(static_cast<long double>((*position3b0)[n])-previous[n]);
        const double first=static_cast<double>(length(stored_square_xyz(displacement3d0)));
        previous390=*position3b0;
        const auto& old_reference=required(reference3a0,"Dog position requires actual Body3A0 reference producer");
        const auto second=length(difference_square(*position3b0,old_reference));
        if(!(std::abs(static_cast<long double>(first)-second)>2.0)){
            const auto matrix=frames.actor_world_matrix(actor.base().frame());reference3a0=Vector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        }else reference3a0=*previous390;
        const auto& target=required(position3b0,"Dog position lost actual Body3B0 producer");const auto& origin=required(reference3a0,"Dog position lost actual Body3A0 producer");Vector direction{};
        for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(static_cast<long double>(target[n])-origin[n]);
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
        if(!actions_.flags3e0){
            const float distance=static_cast<float>(first);interval38c=-1;
            for(unsigned n=0;n!=3;++n)displacement3d0[n]=static_cast<float>(static_cast<long double>(direction[n])*distance);
            return;
        }
        const double distance=static_cast<double>(length(difference_square(target,origin)));
        const auto denominator=first==0?9.999999747378752e-06: first;
        interval38c=human_animation_time_integer(static_cast<long double>(distance)/denominator*100.0+1.0);
        if(interval38c>166){interval38c=166;for(unsigned n=0;n!=3;++n)displacement3d0[n]=static_cast<float>(static_cast<long double>(target[n])-origin[n]);}
        else for(unsigned n=0;n!=3;++n)displacement3d0[n]=static_cast<float>(static_cast<long double>(direction[n])*denominator);
    }
    template<class Actor,class Frames>void request_direction(const Vector& value,Actor& actor,Frames& frames){ //005C0D60; X remains in x87 through comparison.
        const auto snapshot=value;const auto matrix=frames.actor_world_matrix(actor.base().frame());
        long double x=matrix[8];float y=matrix[9],z=matrix[10];
        const auto square=(static_cast<long double>(y)*y+static_cast<long double>(z)*z)+x*x;const float stored=static_cast<float>(square);
        if(!(std::abs(square-1)<9.99999993922529e-09f)){
            if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;y=static_cast<float>(y*inverse);z=static_cast<float>(z*inverse);}
            else if(x!=0)x=x<0?-1:1;
            else if(z!=0)z=z<0?-1.f:1.f;
            else y=y<0?-1.f:1.f;
        }
        const float dy=static_cast<float>(static_cast<long double>(y)-value[1]),dz=static_cast<float>(static_cast<long double>(z)-value[2]);
        if(!(std::abs(x-value[0])<.0010000000474974513)||!(std::abs(static_cast<long double>(dy))<.0010000000474974513)||!(std::abs(static_cast<long double>(dz))<.0010000000474974513)){
            frames.human_reaction_model_direction(actor.base().frame(),snapshot);
            frames.update_scene_object(actor.base().frame());
        }
    }
private:
    template<class T>static const T& required(const std::optional<T>& value,const char* message){if(!value)throw std::runtime_error(message);return *value;}
    static float stored_square_xyz(const Vector& v){return static_cast<float>((static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2]);}
    static float difference_square(const Vector& a,const Vector& b){const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];return static_cast<float>((z*z+y*y)+x*x);}
    static long double length(float square){std::uint32_t bits;std::memcpy(&bits,&square,4);return dog_order_signed(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));}
    DogBodyActions& actions_;
};
}
#endif
