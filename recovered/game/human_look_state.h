#ifndef HD2_GAME_HUMAN_LOOK_STATE_H
#define HD2_GAME_HUMAN_LOOK_STATE_H

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//007AC780/007ACA30. These are the model's command-state writes and shared-CRT
//timing/scan selection, not an animation tick. Preserve the native overwrite
//of its temporary80000000/scan direction bits at the end of007AC780.
struct HumanLookState {
    using Vector=std::array<float,4>;
    std::uint32_t flags=0;
    Vector target{},baseline{},current{},scan{}; //6580,6590 output,65A0 start,65B0 finish.
    std::uint32_t time65c0=0,time65c4=0,time65c8=0,time65cc=0;
    float speed65d0=0,progress65d4=0;
    template<class Random>static std::uint32_t draw(Random& random) {
        const auto word=random();if(word>0x7fff)throw std::runtime_error("Human look requires shared CRT random");return word;
    }
    template<class Frames,class Random>void command(Frames& frames,Random& random,typename Frames::Frame model,typename Frames::Frame bone6,std::uint32_t requested,const Vector* value) {
        if(!model||!bone6)throw std::runtime_error("Human look requires the actual model and bone6");
        const auto bone=frames.actor_world_matrix(bone6);Vector direction{{bone[8],bone[9],bone[10],bone[11]}};
        if(requested&1) {
            if(!value){flags=0;return;}
            target=*value;current=model_direction(frames,model,direction);
            time65cc=(requested&0x80)?(draw(random)>>4)%500+700:0x7fffffff;
            time65c8=(draw(random)>>4)%100+300;time65c0=time65c8;speed65d0=2.5f;flags=requested;return;
        }
        if(!(requested&2)) {
            if(!requested&&(flags&1)) {
                requested=flags;flags|=0x80000000u;time65c0=time65c8;current=baseline;
            }
            flags=requested;return;
        }
        baseline=model_direction(frames,model,direction);
        if(!(flags&2))begin_scan(random); //Native reads PREVIOUS flags, then overwrites them below.
        flags=requested;
    }
    template<class Random>void begin_scan(Random& random) { //007ACA30.
        const auto word=draw(random);const auto shifted=word>>4;std::uint32_t angle;
        bool negative=false;
        if(flags&0x20){angle=shifted%20+((flags&0x80)?50:25);negative=true;}
        else if(flags&0x40)angle=shifted%20+((flags&0x80)?50:25);
        else {
            angle=(flags&0x80)?shifted%50+20:shifted%45+10;
            if(flags&0x20000000u)flags&=0xdfffffffu;
            else if((flags&0x40000000u)||word>0x3fff) {flags=(flags&0xbfffffffu)|0x20000000u;negative=true;}
            if(!negative)flags|=0x40000000u;
        }
        constexpr float degree=0.01745329238474369f; //0080F6E8.
        const auto radians=static_cast<long double>(angle)*degree;
        scan[0]=static_cast<float>((negative?-1:1)*std::sin(radians));scan[1]=0;scan[2]=static_cast<float>(std::cos(radians));
        if(flags&0x10) {
            time65c4=(draw(random)>>4)%2000+1000;time65cc=(draw(random)>>4)%1000+500;time65c8=(draw(random)>>4)%300+400;
        }else if(flags&8) {
            time65c4=(draw(random)>>4)%1000+2000;time65cc=(draw(random)>>4)%500+300;time65c8=(draw(random)>>4)%300+600;
        }else {
            time65c4=(draw(random)>>4)%2000+5000;time65cc=(draw(random)>>4)%1000+500;time65c8=(draw(random)>>4)%500+1000;
        }
        speed65d0=2.5f;current=baseline;time65c0=time65c8;
    }
private:
    template<class Frames>static Vector model_direction(Frames& frames,typename Frames::Frame model,const Vector& direction) {
        Vector result{};
        for(unsigned row=0;row!=3;++row) {
            const auto world=frames.actor_world_matrix(model);
            result[row]=static_cast<float>((static_cast<long double>(direction[0])*world[row*4]+static_cast<long double>(direction[2])*world[row*4+2])+static_cast<long double>(direction[1])*world[row*4+1]);
        }
        return result;
    }
};
} //namespace hd2::game
#endif
