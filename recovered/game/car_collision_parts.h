#ifndef HD2_GAME_CAR_COLLISION_PARTS_H
#define HD2_GAME_CAR_COLLISION_PARTS_H

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::game {

//Native owners for004F9E70/004FA0E0/004FA140/004FA190/004FA2E0.
//Frames are borrowed until extraction. Every geometry/engine operation is a
//mandatory backend call, and extraction clears discarded frame references.
//No4DS interpretation or process pointer relocation participates here.
template<class Frames>
class CarCollisionParts {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Bounds=typename Frames::Bounds;
    struct Point {
        Vector position;
        float radius;
        Frame frame;
    };
    struct Box {
        Bounds bounds;
        Vector position;
        float radius;
        bool type_b;
        Frame frame, reference;
        std::vector<Point> points;
        std::vector<std::unique_ptr<Box>> children;
    };
    void discover(Frames& frames, Frame model) {
        if(!roots_.empty())throw std::logic_error("Car collision discovery repeated");
        const auto prefix=frames.frame_name(model).size()+1;
        for(auto frame:frames.car_parts(model,"BODYCOL*",0x10020)) {
            const auto bounds=frames.car_dummy_bounds(frame);
            auto box=std::make_unique<Box>();
            box->bounds=bounds;box->position=frames.car_position(frame);
            box->radius=std::abs(bounds.maximum[1]-bounds.minimum[1])*.5f;
            const auto scale=frames.car_scale(frame);
            box->radius*=scale[1];
            for(std::size_t i=0;i!=3;++i) {
                box->bounds.minimum[i]*=scale[i];box->bounds.maximum[i]*=scale[i];
            }
            const auto name=frames.frame_name(frame);
            box->type_b=prefix+7<name.size()&&name[prefix+7]=='B';
            box->frame=frame;box->reference=Frame{};
            if(roots_.empty()||!insert(frames,roots_,box)) {
                if(frames.object_parent(frame)!=model)box->reference=frames.object_parent(frame);
                roots_.push_back(std::move(box));
            }
        }
        discover_points(frames,roots_,prefix);
    }
    void extract(Frames& frames, Frame model) {
        const auto inverse=frames.car_inverse_matrix(frames.car_world_matrix(model));
        extract_boxes(frames,model,inverse,roots_);
    }
    const std::vector<std::unique_ptr<Box>>& roots() const {return roots_;}
private:
    static bool insert(Frames& frames,std::vector<std::unique_ptr<Box>>& boxes,std::unique_ptr<Box>& incoming) {
        const auto parent=frames.object_parent(incoming->frame);
        for(auto& box:boxes) {
            if(parent==box->frame) {
                if(box->reference)incoming->reference=box->reference;
                box->children.push_back(std::move(incoming));return true;
            }
            if(insert(frames,box->children,incoming))return true;
        }
        return false;
    }
    static void discover_points(Frames& frames,std::vector<std::unique_ptr<Box>>& boxes,std::size_t prefix) {
        for(auto& box:boxes) {
            for(auto point:frames.car_parts(box->frame,"*",0x10020)) {
                if(frames.object_parent(point)!=box->frame)continue;
                const auto name=frames.frame_name(point);
                if(name.size()>=prefix+7&&name.compare(prefix,7,"BODYCOL")==0)continue;
                const auto bounds=frames.car_dummy_bounds(point);
                const auto radius=std::abs(bounds.maximum[1]-bounds.minimum[1])*.5f*frames.car_scale(point)[1];
                box->points.push_back({frames.car_position(point),radius,point});
            }
            discover_points(frames,box->children,prefix);
        }
    }
    template<class Matrix>
    static void extract_boxes(Frames& frames,Frame model,const Matrix& inverse,std::vector<std::unique_ptr<Box>>& boxes) {
        for(auto& box:boxes) {
            if(!box->frame)throw std::logic_error("Car collision extraction repeated");
            auto relative=[&](const Vector& position,const Matrix& matrix) {
                auto result=frames.car_transform(position,matrix);
                return frames.car_transform(result,box->reference?
                    frames.car_inverse_matrix(frames.car_world_matrix(box->reference)):inverse);
            };
            const auto world=frames.car_world_matrix(box->frame);
            for(auto& point:box->points) {
                point.position=relative(point.position,world);
                frames.remove_car_part(model,point.frame);point.frame=Frame{};
            }
            extract_boxes(frames,model,inverse,box->children);
            const auto parent=frames.object_parent(box->frame);
            if(!parent)throw std::runtime_error("Car collision box lost its original parent");
            box->position=relative(box->position,frames.car_world_matrix(parent));
            for(std::size_t i=0;i!=3;++i) {
                box->bounds.minimum[i]+=box->position[i];box->bounds.maximum[i]+=box->position[i];
            }
            frames.remove_car_part(model,box->frame);box->frame=Frame{};
        }
    }
    std::vector<std::unique_ptr<Box>> roots_;
};

} //namespace hd2::game
#endif
