#ifndef HD2_GAME_CAR_AXES_H
#define HD2_GAME_CAR_AXES_H

#include "game/car_wheel_geometry.h"
#include "game/car_geometry_math.h"
#include <cstdio>
#include <limits>

namespace hd2::game {

//004E6DE0's visual axle records and00500FD0's scale/direction application.
//These render-model connections differ from the strip vertex connection lists
//at570/580 used by physics. Neither one may stand in for the other.
template<class Frames>
class CarAxes {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Wheel=typename CarWheelGeometry<Frames>::Wheel;
    struct Axis {Vector offset{};Frame frame{};std::size_t axle=0;bool left=false,both=false,single=false;};
    void discover(Frames& frames,Frame model,std::size_t axles) {
        if(!both_.empty()||!sides_.empty())throw std::logic_error("Car axis discovery repeated");
        if(axles>12)throw std::runtime_error("Car axis table exceeds parameter columns");
        const auto name=frames.frame_name(model);auto candidates=frames.car_parts(model,"AXIS*",0x10001);
        const auto side_offset=name.size()+5,number_offset=name.size()+6;
        for(std::size_t i=candidates.size();i;--i) {
            auto frame=candidates[i-1];const auto text=frames.frame_name(frame);const auto axle=number(text,number_offset);
            if(axle>=axles||text.size()<=side_offset||text[side_offset]!='0')continue;
            both_.push_back({offset(frames.car_user_data(frame)),frame,axle,false,true,true});
            candidates.erase(candidates.begin()+std::ptrdiff_t(i-1));
        }
        std::array<std::array<std::size_t,12>,2> count{};
        for(auto entry=candidates.rbegin();entry!=candidates.rend();++entry) {
            const auto text=frames.frame_name(*entry);const auto axle=number(text,number_offset);
            if(axle>=axles||text.size()<=side_offset||(text[side_offset]!='L'&&text[side_offset]!='R'))continue;
            const bool left=text[side_offset]=='L';auto value=offset(frames.car_user_data(*entry));
            if(!left)value[0]=-value[0];
            sides_.push_back({value,*entry,axle,left,false,false});++count[left?0:1][axle];
        }
        for(auto& axis:sides_)axis.single=count[axis.left?0:1][axis.axle]==1;
    }
    void apply(Frames& frames,const std::vector<Wheel>& wheels,std::uint32_t flags,std::uint32_t previous) {
        if((flags&1)!=(previous&1)) {
            for(auto* list:{&sides_,&both_})for(auto axis=list->rbegin();axis!=list->rend();++axis)
                frames.object_visible(axis->frame,(flags&1)!=0);
        }
        if(!(flags&1))return;
        auto wheel_position=[&](std::size_t index) {
            const auto& wheel=wheels.at(index);
            return frames.car_transform(frames.car_position(wheel.frame),frames.car_local_matrix(wheel.parent));
        };
        for(auto axis=sides_.rbegin();axis!=sides_.rend();++axis) {
            const auto position=wheel_position(axis->axle*2+(axis->left?0:1));
            const auto origin=frames.car_position(axis->frame);Vector direction{};
            for(std::size_t i=0;i!=3;++i)direction[i]=position[i]+axis->offset[i]-origin[i];
            frames.object_scale(axis->frame,{1,1,length(direction)});
            frames.car_direction(axis->frame,normalize(direction));
        }
        for(auto axis=both_.rbegin();axis!=both_.rend();++axis) {
            const auto left=wheel_position(axis->axle*2),right=wheel_position(axis->axle*2+1);
            Vector position{},delta{},direction{};
            const auto left_origin=frames.car_position(wheels.at(axis->axle*2).parent);
            const auto right_origin=frames.car_position(wheels.at(axis->axle*2+1).parent);
            for(std::size_t i=0;i!=3;++i) {
                position[i]=right[i]+axis->offset[i]*(i==0?-1.f:1.f);
                delta[i]=left[i]+axis->offset[i]-position[i];
                direction[i]=left_origin[i]-right_origin[i];
            }
            frames.object_position(axis->frame,{position[0],position[1],position[2]});
            frames.object_scale(axis->frame,{1,1,length(delta)});
            frames.car_direction(axis->frame,normalize(direction));
        }
    }
    const std::vector<Axis>& both() const {return both_;}
    const std::vector<Axis>& sides() const {return sides_;}
private:
    static constexpr float epsilon=9.99999993922529e-09f;
    static float squared(const Vector& value) {return value[0]*value[0]+value[1]*value[1]+value[2]*value[2];}
    static float length(const Vector& value) {
        const auto sum=squared(value);
        if(!std::isfinite(sum))throw std::runtime_error("Non-finite Car axis length");
        if(sum<epsilon)return 0;
        return std::abs(sum-1.f)<epsilon?1.f:std::sqrt(sum);
    }
    static Vector normalize(Vector value) {
        return car_normalize(value);
    }
    static std::size_t number(const std::string& name,std::size_t offset) {
        if(offset>=name.size())return std::numeric_limits<std::size_t>::max();
        char* end=nullptr;const auto value=std::strtol(name.c_str()+offset,&end,10);
        if(value<0)return std::numeric_limits<std::size_t>::max();
        return static_cast<std::size_t>(end==name.c_str()+offset?0:value);
    }
    static Vector offset(const std::string& text) {
        Vector value{};const auto bracket=text.find('[');
        if(text.size()>2&&bracket!=std::string::npos) {
            std::sscanf(text.c_str()+bracket+1," %f %f %f",&value[0],&value[1],&value[2]);
            for(std::size_t i=0;i!=3;++i) {if(!std::isfinite(value[i]))throw std::runtime_error("Non-finite Car axis offset");value[i]*=.01f;}
        }
        return value;
    }
    std::vector<Axis> both_,sides_;
};

} //namespace hd2::game
#endif
