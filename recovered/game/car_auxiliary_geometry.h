#ifndef HD2_GAME_CAR_AUXILIARY_GEOMETRY_H
#define HD2_GAME_CAR_AUXILIARY_GEOMETRY_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hd2::game {

//004E2E30's exhaust/special-collision/head, door/seat and CENTRE records.
//Frames are borrowed from the retained model. Removed markers are converted to
//values first; native indices replace pointers into reallocating door storage.
template<class Frames>
class CarAuxiliaryGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Matrix=typename Frames::Matrix;
    using Bounds=typename Frames::Bounds;
    using Rotation=decltype(std::declval<Frames&>().car_rotation(std::declval<Frame>()));
    static constexpr std::size_t no_door=std::numeric_limits<std::size_t>::max();
    struct Exhaust {Vector position{},direction{};std::int32_t type=-1;};
    struct Ballast {Vector position{},direction{};};
    struct SpecialCollision {
        Bounds bounds{};
        Matrix inverse_reference{};
        Vector position{};
        Frame reference{};
        std::uint32_t flags=0x40;
        bool enabled=true;
    };
    struct Window {
        Frame frame{};
        Vector position{};
        float x=0,y=0,lower_y=0,radius=0;
        std::uint16_t part=0;
    };
    struct Door {
        Rotation rotation{};
        Vector position{};
        Frame frame{};
        float first_limit=1.25f,second_limit=1.4500000476837158f;
        std::uint8_t flags=0;
        std::uint16_t part=0xffff;
        Window window{};
    };
    struct Seat {
        Vector position{},direction{};
        Frame frame{};
        std::size_t door=no_door;
    };

    void extract_markers(Frames& frames,Frame model,Frame body) {
        auto exhausts=frames.car_parts(model,"EXHAUST*",0x10021);
        for(auto it=exhausts.rbegin();it!=exhausts.rend();++it) {
            const auto matrix=frames.car_local_matrix(body);
            Exhaust record;
            record.position=frames.car_transform(frames.car_position(*it),matrix);
            record.direction=frames.car_transform(frames.car_forward(*it),matrix);
            exhausts_.push_back(record);
            frames.remove_car_part(model,*it);
        }
        for(auto frame:frames.car_parts(model,"SPECCOLL*",0x10021)) {
            SpecialCollision record;
            const auto bounds=frames.car_dummy_bounds(frame);
            record.bounds.minimum=frames.car_transform(bounds.minimum,frames.car_local_matrix(frame));
            record.bounds.maximum=frames.car_transform(bounds.maximum,frames.car_local_matrix(frame));
            record.inverse_reference=identity();
            const auto parent=frames.object_parent(frame);
            if(!parent)throw std::runtime_error("Car special collision marker has no parent");
            if(parent!=model) {
                record.reference=parent;
                record.inverse_reference=frames.car_inverse_matrix(frames.car_world_matrix(parent));
                record.position=frames.car_position(frame);
            }
            special_.push_back(record);
            frames.remove_car_part(model,frame);
        }
        head_=extract_position(frames,model,body,"HEAD");
    }

    void discover_doors(Frames& frames,Frame model) {
        const auto offset=frames.frame_name(model).size()+5;
        auto candidates=frames.car_parts(model,"DOOR*",0x10001);
        candidates.erase(std::remove_if(candidates.begin(),candidates.end(),[&](Frame frame) {
            const auto name=frames.frame_name(frame);
            return name.size()<=offset||name[offset]<'0'||name[offset]>'9';
        }),candidates.end());
        order(frames,candidates,offset);
        for(auto frame:candidates) {
            const auto parent=frames.object_parent(frame);
            if(!parent)throw std::runtime_error("Car door has no parent");
            Door record;
            record.position=frames.car_transform(frames.car_position(frame),frames.car_local_matrix(parent));
            record.frame=frame;record.rotation=frames.car_rotation(frame);
            if(frames.car_forward(frame)[2]<0)record.flags|=0xc;
            const auto user=frames.car_user_data(frame);
            const auto bracket=user.find('[');
            if(bracket!=std::string::npos)for(auto i=bracket+1;i<user.size();++i)
                if(user[i]=='I'&&(user[i-1]=='['||user[i-1]==',')) {record.flags|=4;break;}
            if(auto window=frames.car_find(frame,"WINDOW*",0x10001)) {
                record.window.frame=window;record.window.position=frames.car_position(window);
                record.window.x=record.window.position[0];record.window.y=record.window.position[1];
                record.window.lower_y=record.window.y-.5f; //0080F518.
                record.window.radius=.30000001192092896f;record.window.part=0xffff;
            }
            doors_.push_back(record);
        }
    }

    void discover_seats(Frames& frames,Frame model,std::uint32_t flags) {
        auto candidates=frames.car_parts(model,"SEAT*",0x10020);
        if(candidates.empty()&&!(flags&2))throw std::runtime_error("Car requires seat markers");
        order(frames,candidates,frames.frame_name(model).size()+5);
        for(auto frame:candidates) {
            const auto parent=frames.object_parent(frame);
            if(!parent)throw std::runtime_error("Car seat has no parent");
            const auto matrix=frames.car_local_matrix(parent);
            Seat record;
            record.position=frames.car_transform(frames.car_position(frame),matrix);
            record.direction=frames.car_transform(frames.car_forward(frame),matrix);
            record.frame=frame;
            if(seats_.size()<doors_.size())record.door=seats_.size();
            seats_.push_back(record);
        }
    }

    void extract_ballast(Frames& frames,Frame model,Frame body) {
        auto candidates=frames.car_parts(body,"BALLAST*",0x10020);
        const auto offset=frames.frame_name(model).size()+8;
        for(std::size_t i=1;i<candidates.size();++i) {
            const auto value=number(frames,candidates[i],offset);
            for(std::size_t j=0;j<i;++j)if(value<number(frames,candidates[j],offset)) {
                //004E4267–004E42B7 inserts a copy at the CURRENT slot, writes
                //the final element to the next slot, then erases the last one.
                //It does not insert at j. Preserve that unusual permutation.
                const auto frame=candidates[i];
                candidates.insert(candidates.begin()+static_cast<std::ptrdiff_t>(i),frame);
                candidates[i+1]=candidates.back();candidates.pop_back();break;
            }
        }
        for(auto frame:candidates) {
            const auto parent=frames.object_parent(frame);
            if(!parent)throw std::runtime_error("Car ballast marker has no parent");
            const auto matrix=frames.car_local_matrix(parent);
            ballast_.push_back({frames.car_transform(frames.car_position(frame),matrix),
                                frames.car_transform(frames.car_forward(frame),matrix)});
            frames.remove_car_part(model,frame);
        }
    }

    //Called after axis setup, before wheel positions/strips and deformables.
    void extract_centre(Frames& frames,Frame model,Frame body) {
        const auto centre=frames.car_part(model,"CENTRE",0x20);
        if(!centre)throw std::runtime_error("Car requires a CENTRE collision marker");
        const auto bounds=frames.car_dummy_bounds(centre);
        collision_.minimum=frames.car_transform(bounds.minimum,frames.car_local_matrix(centre));
        collision_.maximum=frames.car_transform(bounds.maximum,frames.car_local_matrix(centre));
        collision_.minimum=frames.car_transform(collision_.minimum,frames.car_local_matrix(body));
        collision_.maximum=frames.car_transform(collision_.maximum,frames.car_local_matrix(body));
        frames.remove_car_part(model,centre);frames.update_scene_object(model);
        const auto model_bounds=frames.car_bounds(model);
        const auto scale=frames.car_scale(model);
        for(std::size_t i=0;i!=3;++i) {
            dimensions_[i]=model_bounds.maximum[i]-model_bounds.minimum[i];
            scaled_dimensions_[i]=dimensions_[i]*scale[i];
            origin_[i]=-model_bounds.minimum[i];scaled_origin_[i]=origin_[i]*scale[i];
        }
        steering_wheel_=frames.car_part(model,"SWHEEL",1);
        if(steering_wheel_)steering_direction_=frames.car_forward(steering_wheel_);
        smoke_=extract_position(frames,model,body,"SMOKE_MOTOR");
    }

    template<class Parts> void bind_parts(const Parts& parts) {
        for(auto seat=seats_.rbegin();seat!=seats_.rend();++seat) {
            if(seat->door==no_door)continue;
            auto& door=doors_.at(seat->door);
            auto bind=[&](Frame frame,std::uint16_t& destination) {
                if(!frame)return;
                for(auto i=parts.size();i;--i)if(parts[i-1].frame==frame) {
                    if(i-1>=0xffff)throw std::runtime_error("Car door/window part index overflow");
                    destination=static_cast<std::uint16_t>(i-1);break;
                }
            };
            bind(door.window.frame,door.window.part);bind(door.frame,door.part);
        }
    }
    const std::vector<Exhaust>& exhausts() const {return exhausts_;}
    const std::vector<Ballast>& ballast() const {return ballast_;}
    const std::vector<SpecialCollision>& special() const {return special_;}
    const std::vector<Door>& doors() const {return doors_;}
    const std::vector<Seat>& seats() const {return seats_;}
    const Bounds& collision_bounds() const {return collision_;}
    const Vector& head() const {return head_;}
    const Vector& smoke() const {return smoke_;}
    const Vector& dimensions() const {return dimensions_;}
    const Vector& scaled_dimensions() const {return scaled_dimensions_;}
    const Vector& origin() const {return origin_;}
    const Vector& scaled_origin() const {return scaled_origin_;}
    Frame steering_wheel() const {return steering_wheel_;}
    const Vector& steering_direction() const {return steering_direction_;}
private:
    static Matrix identity() {Matrix value{};for(std::size_t i=0;i!=4;++i)value[i*5]=1;return value;}
    static Vector extract_position(Frames& frames,Frame model,Frame body,const char* pattern) {
        const auto frame=frames.car_part(model,pattern,0x20);
        if(!frame)return {};
        const auto position=frames.car_transform(frames.car_position(frame),frames.car_local_matrix(body));
        frames.remove_car_part(model,frame);return position;
    }
    static long number(Frames& frames,Frame frame,std::size_t offset) {
        const auto name=frames.frame_name(frame);
        return offset<name.size()?std::strtol(name.c_str()+offset,nullptr,10):0;
    }
    static void order(Frames& frames,std::vector<Frame>& values,std::size_t offset) {
        //Original swaps a later matching suffix into place and leaves gaps
        //unchanged. Sorting or rejecting missing suffixes changes that behavior.
        for(std::size_t i=0;i<values.size();++i) {
            if(number(frames,values[i],offset)==static_cast<long>(i))continue;
            for(auto j=i+1;j<values.size();++j)if(number(frames,values[j],offset)==static_cast<long>(i)) {
                std::swap(values[i],values[j]);break;
            }
        }
    }
    std::vector<Exhaust> exhausts_;
    std::vector<Ballast> ballast_;
    std::vector<SpecialCollision> special_;
    std::vector<Door> doors_;
    std::vector<Seat> seats_;
    Bounds collision_{};
    Vector head_{},smoke_{},dimensions_{},scaled_dimensions_{},origin_{},scaled_origin_{},steering_direction_{};
    Frame steering_wheel_{};
};

} //namespace hd2::game
#endif
