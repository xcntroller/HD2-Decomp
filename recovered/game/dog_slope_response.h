#ifndef HD2_GAME_DOG_SLOPE_RESPONSE_H
#define HD2_GAME_DOG_SLOPE_RESPONSE_H
#include "game/collision_motion.h"
#include "game/dog_body_orders.h"
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
//005C43D0. The separate local-normal producer is required explicitly; neither
//an absent normal nor a decompiler's scratch name is replaced with zero.
template<class Dog,class Hit>
void apply_dog_slope_response(Dog& dog,CollisionMotion<Hit>& request,bool hit,bool normal_present,
        const HumanVector* normal,const HumanVector& anchor,const HumanVector& movement,std::int32_t elapsed){
    auto& b=dog.body;long double fraction=0;
    if(hit&&normal_present&&dog_order_signed(dog.body_actions.current364)<5&&
       !(std::abs(static_cast<long double>(request.position[1])-request.centers[0][1])>1.)&&
       !(b.field250>static_cast<long double>(9.8100004196167f)*2)){
        if(!normal)throw std::runtime_error("Dog slope response requires actual separate normal producer");
        long double x=(*normal)[0],z=(*normal)[2];float y=0;
        const float vertical=static_cast<float>((x+z)*0.f+(*normal)[1]);
        HumanVector direction{{request.displacement[0],0,request.displacement[2],0}};
        normalize_human_ai_shot(direction,static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0]);
        const auto square=z*z+x*x;const float stored_square=static_cast<float>(square);
        if(!(std::abs(square-1)<9.99999993922529e-09f)){
            if(!(stored_square<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored_square));x*=inverse;y=static_cast<float>(0.f*inverse);z*=inverse;}
            else if(x!=0)x=x<0?-1:1;
            else if(z!=0)z=z<0?-1:1;
            else y=1;
        }
        const float dot=static_cast<float>((static_cast<long double>(direction[2])*z+x*direction[0])+static_cast<long double>(direction[1])*y);
        if(vertical>.5f||dot>.6000000238418579f||dot<-.6000000238418579f){
            if(!(vertical>.949999988079071f)){
                const float time=static_cast<float>(elapsed);
                const float slope=static_cast<float>((1000.L/time)*(static_cast<long double>(request.position[1])-anchor[1]));
                const float planar=static_cast<float>(static_cast<long double>(movement[2])*movement[2]+static_cast<long double>(movement[0])*movement[0]);
                std::uint32_t bits;std::memcpy(&bits,&planar,4);
                const auto length=dog_order_signed(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(planar)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(planar));
                const auto extended_speed=length*1000.f/time;const float speed=static_cast<float>(extended_speed);
                const auto adjusted=extended_speed>0&&speed<1?static_cast<long double>(slope)/speed:static_cast<long double>(slope);
                if(vertical>0&&adjusted>.20000000298023224f&&b.field1f8==-10000.099609375f){
                    fraction=(1.L-vertical)*(dot<0?-static_cast<long double>(dot):static_cast<long double>(dot))*2.0999999046325684f;
                    if(fraction>1||vertical<=.550000011920929f)fraction=1;
                }
            }
        }else fraction=b.field1fc;
    }
    if(std::abs((static_cast<long double>(b.field1fc)-fraction)*100.f)/elapsed>.17000000178813934){
        const auto step=static_cast<long double>(elapsed)*.17000000178813934f*.009999999776482582f;
        fraction=fraction<b.field1fc?static_cast<long double>(b.field1fc)-step:static_cast<long double>(b.field1fc)+step;
    }
    b.field1fc=static_cast<float>(fraction); //Native retains the extended fraction for rate/position after this store.
    const auto extended_rate=1.L-.5f*fraction;float rate=static_cast<float>(extended_rate);
    if(extended_rate<.6499999761581421f)rate=.6499999761581421f;
    if(fraction!=0){
        if(!(fraction>=.8999999761581421f)){
            const auto position=request.position;
            const float dx=static_cast<float>((static_cast<long double>(position[0])-anchor[0])*fraction);
            const float dy=static_cast<float>((static_cast<long double>(position[1])-anchor[1])*fraction);
            const float dz=static_cast<float>(static_cast<long double>(position[2])-anchor[2]);
            request.position[0]=static_cast<float>(static_cast<long double>(position[0])-dx);
            request.position[1]=static_cast<float>(static_cast<long double>(position[1])-dy);
            request.position[2]=static_cast<float>(static_cast<long double>(position[2])-fraction*dz);
        }else request.position=anchor;
    }
    if(!dog.clip)throw std::runtime_error("Dog slope response requires actual clip owner at native rate call");
    dog.clip->rate(0,rate==1.f?1.f:rate);
}
}
#endif
