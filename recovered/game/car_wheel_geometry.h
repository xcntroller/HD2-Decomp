#ifndef HD2_GAME_CAR_WHEEL_GEOMETRY_H
#define HD2_GAME_CAR_WHEEL_GEOMETRY_H

#include "game/car_physics.h"
#include <cstdlib>
#include <limits>
#include <string_view>

namespace hd2::game {

//004F9BE0 and004E2E30 wheel discovery/measurement/indexing. The remaining Car
//constructor owns axis connections, strips, deformable parts and callbacks.
//This owner supplies actual derived geometry for those consumers; it does not
//publish a constructed/ready actor on its own.
template<class Frames>
class CarWheelGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    struct Wheel {
        Frame frame, parent;
        std::string name;
        float radius, contact_radius, width, diagonal, contact_diagonal, angle, local_x;
        Vector local_position, position;
        std::uint32_t flags;
        float strip_offset=0;
    };
    void discover(Frames& frames,Frame model,std::uint32_t car_flags) {
        if(!wheels_.empty())throw std::logic_error("Car wheel discovery repeated");
        const auto name=frames.frame_name(model);
        const auto candidates=frames.car_parts(model,"WH*",0x10021);
        if(!(car_flags&2)&&(candidates.size()<4||candidates.size()%2))
            throw std::runtime_error("Car requires at least two paired wheel axles");
        if(candidates.size()>24||candidates.size()%2)
            throw std::runtime_error("Unsupported Car wheel count");
        std::vector<Wheel> discovered;
        for(auto frame:candidates) {
            const auto wheel_name=frames.frame_name(frame);
            const auto number=name.size()+4; //model name + '.WH' + side.
            if(number>=wheel_name.size()||wheel_name[number]<'0'||wheel_name[number]>'9')
                throw std::runtime_error("Car wheel has no numeric axle suffix");
            const auto parent=frames.object_parent(frame);
            if(!parent)throw std::runtime_error("Car wheel has no original parent");
            const auto parent_name=frames.frame_name(parent);
            if(parent_name.compare(0,name.size()+5,name+".DWHR")&&
               parent_name.compare(0,name.size()+5,name+".DWHL"))
                throw std::runtime_error("Car wheel parent is not DWHR/DWHL");
            const auto bounds=frames.car_bounds(frame);
            const auto radius=std::abs(bounds.maximum[1]-bounds.minimum[1])*.5f;
            auto width=std::abs(bounds.maximum[0]-bounds.minimum[0]);
            const auto user=frames.car_user_data(parent);
            const auto bracket=user.find('[');
            if(user.size()>2&&bracket!=std::string::npos) {
                //Original sscanf("%f") leaves the existing width unchanged on
                //failure, then multiplies it by.01 even when conversion failed.
                const auto value=user.c_str()+bracket+1;
                char* end=nullptr;const auto parsed=std::strtof(value,&end);
                if(end!=value)width=parsed;
                width*=.01f;
            }
            const auto contact_radius=radius*.800000011920929f;
            const auto diagonal=std::sqrt(radius*radius+width*width);
            const auto contact_diagonal=std::sqrt(contact_radius*contact_radius+width*width);
            if(!std::isfinite(radius)||radius<=0||!std::isfinite(width)||!std::isfinite(diagonal)||diagonal<=0)
                throw std::runtime_error("Invalid Car wheel geometry");
            discovered.push_back({frame,parent,wheel_name,radius,contact_radius,width,diagonal,contact_diagonal,
                                  std::acos(radius/diagonal),frames.car_position(frame)[0],{},{},0});
        }
        std::vector<Wheel> ordered;
        ordered.reserve(discovered.size());
        //Original searches its linked list for L/R at each axle index. Reject a
        //missing pair explicitly instead of propagating its null dereference.
        for(std::size_t axle=0;axle<discovered.size()/2;++axle)for(const char side:{'L','R'}) {
            const auto found=std::find_if(discovered.begin(),discovered.end(),[&](const Wheel& wheel) {
                const auto side_offset=name.size()+3;
                const auto wheel_side=wheel.name.at(side_offset);
                return (wheel_side==side||wheel_side==char(side+('a'-'A')))&&suffix(wheel.name,side_offset+1)==axle;
            });
            if(found==discovered.end())throw std::runtime_error("Car wheel axle pair is missing");
            ordered.push_back(*found);
        }
        //004F9BE0 appends nativeF60's linked list in enumeration order;
        //F64 is the separate axle-sorted index.004EC5C0 visits both orders.
        for(const auto& wheel:discovered){
            const auto found=std::find_if(ordered.begin(),ordered.end(),[&](const Wheel& value){return value.frame==wheel.frame;});
            linked_order_.push_back(static_cast<std::size_t>(found-ordered.begin()));
        }
        wheels_=std::move(ordered);
    }
    void initialize_positions(Frames& frames,Frame model) {
        const auto inverse=frames.car_inverse_matrix(frames.car_world_matrix(model));
        for(auto& wheel:wheels_) {
            wheel.local_position=frames.car_position(wheel.parent);
            if(frames.object_parent(wheel.parent)==model)wheel.position=wheel.local_position;
            else {
                const auto world=frames.car_world_matrix(wheel.parent);
                wheel.position=frames.car_transform(Vector{world[12],world[13],world[14],world[15]},inverse);
            }
            wheel.flags=frames.frame_kind(wheel.frame)==6?0x400u:0u;
        }
    }
    const std::vector<Wheel>& wheels() const {return wheels_;}
    std::vector<Wheel>& wheels() {return wheels_;}
    const std::vector<std::size_t>& linked_order()const{return linked_order_;}
    std::vector<CarWheelPhysics> physics_wheels() const {
        std::vector<CarWheelPhysics> result;
        for(const auto& wheel:wheels_)result.push_back({wheel.radius,wheel.flags,-1,-1,{}});
        return result;
    }
private:
    static std::size_t suffix(const std::string& name,std::size_t offset) {
        std::size_t value=0;
        for(;offset<name.size()&&name[offset]>='0'&&name[offset]<='9';++offset) {
            const auto digit=std::size_t(name[offset]-'0');
            if(value>(std::numeric_limits<std::size_t>::max()-digit)/10)
                throw std::runtime_error("Car wheel axle suffix overflow");
            value=value*10+digit;
        }
        return value;
    }
    std::vector<Wheel> wheels_;
    std::vector<std::size_t> linked_order_;
};

} //namespace hd2::game
#endif
