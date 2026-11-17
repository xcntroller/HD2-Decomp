#ifndef HD2_GAME_DOG_HEAD_LOOK_H
#define HD2_GAME_DOG_HEAD_LOOK_H
#include "game/human_ai_shot_direction.h"
#include "game/dog_damage.h"
#include "game/human_packet_commands.h"
#include "game/human_motion_velocity.h"
namespace hd2::game {
//Actor4E8..52C, separate from Body action/posture/stance. Actual005D4DB0
//produces the scalars; the current4F0 target is produced by005D82B0's tail.
class DogHeadLook {
public:
    bool flag4e8=false,active4e9=false,request504=false,finished505=false;
    float speed508=1;std::uint32_t field50c=1,mode520=0,parameter52c=0;
    std::int32_t timer524=0,wait528=-1;
    std::optional<float> angle500; //Produced by the first005D7A10 frame, not005D4DB0.
    HumanVector target510{};std::optional<HumanVector> current4f0;
    void initialize(){flag4e8=active4e9=request504=finished505=false;speed508=1;field50c=1;target510.fill(0);mode520=0;timer524=0;wait528=-1;parameter52c=0;initialized_=true;}
    template<class Random>std::uint32_t request(const HumanVector& target,std::uint32_t mode,std::uint32_t parameter,Random&& random){ //005D8070; incidental integer return is preserved.
        require_initialized();
        const auto selected=mode&0xffffffeeu;if(mode520==selected)return selected;
        mode520=selected;target510=target;const auto result=selected-2u;
        const auto speed=[&]{const auto value=dog_signed_word(random());speed508=static_cast<float>(static_cast<long double>(value%30)*.009999999776482582f+((mode>>4)&1u?1.f:.6000000238418579f));return static_cast<std::uint32_t>(value/30);};
        const auto open=[&]{active4e9=true;finished505=request504=false;};
        switch(selected){
        case 2:wait528=-1;open();return speed();
        case 4:request504=true;if(!parameter52c){wait528=0;return 0;}break;
        case 6:wait528=-1;open();timer524=dog_signed_word(parameter);return speed();
        case 8:{const auto draw=dog_signed_word(random());parameter52c=parameter;timer524=draw%1500;
            if(parameter>2u){wait528=0;open();return speed();}
            if(parameter==0)return 0;
            break;}
        default:return result;
        }
        const auto duration=parameter52c*2000u;
        const auto draw=static_cast<std::uint32_t>(random());
        if(!duration)throw std::runtime_error("Dog head look reaches native random-duration division fault");
        const auto value=draw%duration+parameter52c*1000u;wait528=dog_signed_word(value);return value;
    }
    template<class Queries>void advance(std::uint32_t elapsed,Queries& queries){ //005D82B0, native signed clock comparisons.
        require_initialized();
        if(!active4e9&&(mode520!=8||(parameter52c!=1&&parameter52c!=2)))return;
        const auto duration=[&]{if(!parameter52c)return std::int32_t{0};const auto draw=queries.random_word();const auto divisor=parameter52c*2000u;if(!divisor)throw std::runtime_error("Dog head look reaches native random-duration division fault");return dog_signed_word(draw%divisor+parameter52c*1000u);};
        if(mode520==6){
            if(finished505){timer524=dog_signed_word(static_cast<std::uint32_t>(timer524)-elapsed);if(timer524<=0){request504=true;wait528=duration();}}
        }else if(mode520==8){
            bool process=false;
            switch(parameter52c){
            case 0:wait528=-1;break;
            case 1:case 2:
                if(wait528>dog_signed_word(elapsed)){wait528=dog_signed_word(static_cast<std::uint32_t>(wait528)-elapsed);break;}
                if(wait528>0){wait528=0;if(speed508>.6000000238418579f)speed508*=.5f;}
                process=true;break;
            case 3:process=true;break;
            default:break;
            }
            if(process){
                if(wait528==0){
                    active4e9=true;wait528=-1;
                    auto direction=queries.model_forward();
                    normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
                    const auto first=dog_signed_word(queries.random_word());
                    float angle=static_cast<float>(static_cast<long double>(first%10)*.09090909361839294f+.2199999988079071f);
                    if(dog_signed_word(queries.random_word())%2!=0)angle=-angle;
                    queries.rotate_direction(direction,angle);
                    for(unsigned n=0;n!=3;++n)direction[n]*=20.f;
                    const auto position=queries.model_position();
                    for(unsigned n=0;n!=3;++n)target510[n]=static_cast<float>(static_cast<long double>(direction[n])+position[n]);
                    //The temporary fourth lane is native-unproduced padding.
                }
                if(finished505){
                    timer524=dog_signed_word(static_cast<std::uint32_t>(timer524)-elapsed);
                    if(timer524<=0){
                        timer524=dog_signed_word(human_packet_ftol(static_cast<long double>(queries.random_unit())*1000.f));
                        request504=true;finished505=false;
                        if(parameter52c==1||parameter52c==2)wait528=duration();
                    }
                }
            }
        }
        current4f0=target510;
    }
    template<class Queries>void present(std::int32_t elapsed,Queries& queries){ //005D7A10.
        require_initialized();if(!active4e9)return;
        const auto inverse=queries.parent_inverse();
        auto direction=queries.model_forward();
        const auto normalize=[](HumanVector& vector){normalize_human_ai_shot(vector,(static_cast<long double>(vector[1])*vector[1]+static_cast<long double>(vector[2])*vector[2])+static_cast<long double>(vector[0])*vector[0]);};
        normalize(direction);
        if(request504){
            HumanVector offset=direction;for(unsigned n=0;n!=3;++n)offset[n]*=10.f;
            const auto position=queries.head_position();auto value=position;
            for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(offset[n])+position[n]);
            current4f0=value;
        }
        const auto position=queries.head_position();
        if(!current4f0)throw std::runtime_error("Dog head presentation requires actual4F0 target producer");
        HumanVector toward{};for(unsigned n:{0u,2u})toward[n]=static_cast<float>(static_cast<long double>((*current4f0)[n])-position[n]);
        auto delta=human_motion_planar_angle(direction,toward);
        if(!flag4e8){
            auto current=queries.head_forward();normalize(current);angle500=0;
            delta=human_motion_planar_angle(current,toward);
        }else{
            if(!angle500)throw std::runtime_error("Dog head presentation requires actual500 yaw producer");
            delta-=*angle500;
        }
        const auto magnitude=std::abs(delta);long double rate=.0020000000949949026f;
        if(field50c==0)rate=.0028000001329928637f;
        else if(field50c==1||field50c==2){
            auto value=magnitude;
            if(value<.10000000149011612f)value=.10000000149011612f;
            else if(field50c==1&&value>3.1415927410125732f)value=3.1415927410125732f;
            rate=std::log(value)*.0020000000949949026+.006000000284984708;
        }
        const float step=static_cast<float>((delta>0?1.L:-1.L)*rate*speed508*elapsed);
        if(std::abs(static_cast<long double>(step))<magnitude)delta=step;
        const auto yaw=*angle500;
        if(delta*yaw>=0&&std::abs(delta+yaw)>.9707963466644287)delta=0;
        //Raw AND4100 treats unordered as within the completion threshold.
        const bool completed=!(std::abs(delta)>.003000000026077032);
        if(completed&&request504){
            active4e9=flag4e8=request504=false;queries.reset_animation();return;
        }
        if(completed)finished505=true;
        else angle500=static_cast<float>(delta+yaw);
        queries.rotate_direction(direction,*angle500);
        queries.apply_inverse(direction,inverse);
        queries.set_animation_direction(direction);
        flag4e8=true; //After successful engine publication.
    }
private:
    bool initialized_=false; //Source provenance, not a native readiness flag.
    void require_initialized()const{if(!initialized_)throw std::runtime_error("Dog head look requires actual005D4DB0 initialization");}
};
} //namespace hd2::game
#endif
