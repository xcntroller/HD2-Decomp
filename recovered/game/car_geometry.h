#ifndef HD2_GAME_CAR_GEOMETRY_H
#define HD2_GAME_CAR_GEOMETRY_H

#include "game/car_auxiliary_geometry.h"
#include "game/car_axes.h"
#include "game/car_collision_parts.h"
#include "game/car_default_parameters.h"
#include "game/car_deformable_parts.h"
#include "game/car_lighting.h"
#include "game/car_strip_connections.h"
#include <optional>

namespace hd2::game {

//Ordered ordinary runtime construction from004E2E30. This owns model geometry
//and the first004ED270 application, not004D10A0's actor callbacks/ancillary model
//or subsequent004D55C0 property application. No actor/world readiness follows
//from constructing this value alone. On failure, the containing loading stage
//must discard/reset the modified model/scene before trying another load.
template<class Frames>
class CarGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Matrix=typename Frames::Matrix;
    using ModelOwner=decltype(std::declval<Frames&>().retain_car_frame(std::declval<Frame>()));
    CarGeometry(Frames& frames,Frame model,const std::vector<std::int32_t>& tyre_ids,
                std::uint32_t car_flags=0,std::uint32_t visual_flags=0x8000dff9) {
        if(!model)throw std::runtime_error("Car construction requires a model");
        model_.emplace(frames.retain_car_frame(model));
        if(frames.frame_kind(model)!=9)throw std::runtime_error("Car construction requires model frame kind9");
        name_=frames.frame_name(model);resource_=frames.car_model_resource(model);
        initial_world_=frames.car_world_matrix(model);
        body_=frames.car_part(model,"BODY",0x21);
        if(!body_)throw std::runtime_error("Car model requires BODY");
        auxiliary_.extract_markers(frames,model,body_);
        collision_.discover(frames,model);collision_.extract(frames,model);
        auxiliary_.discover_doors(frames,model);
        auxiliary_.extract_ballast(frames,model,body_);
        auxiliary_.discover_seats(frames,model,car_flags);
        wheels_.discover(frames,model,car_flags);
        axes_.discover(frames,model,wheels_.wheels().size()/2);
        axes_.apply(frames,wheels_.wheels(),visual_flags,visual_flags);
        auxiliary_.extract_centre(frames,model,body_);
        wheels_.initialize_positions(frames,model);
        strips_.discover(frames,model,wheels_.wheels());
        deformables_.discover(frames,model);deformables_.initialize_visibility(frames);
        auxiliary_.bind_parts(deformables_.parts());
        lighting_.discover(frames,model,deformables_.parts());
        const auto bounds=auxiliary_.collision_bounds();
        CarPhysicsGeometry geometry{{bounds.minimum[0],bounds.minimum[1],bounds.minimum[2]},
                                    {bounds.maximum[0],bounds.maximum[1],bounds.maximum[2]},
                                    wheels_.wheels().size()/2,car_flags,wheels_.physics_wheels(),
                                    strips_.wheel_indices(false),strips_.wheel_indices(true),deformables_.physics_parts()};
        physics_.emplace(std::move(geometry));
        if(!physics_->apply(default_car_parameters(wheels_.wheels().size()/2),tyre_ids))
            throw std::runtime_error("Car default physics parameters rejected");
    }
    Frame model() const {return model_->get();}
    Frame body() const {return body_;}
    const std::string& name() const {return name_;}
    const std::string& resource() const {return resource_;}
    const Matrix& initial_world() const {return initial_world_;}
    const CarAuxiliaryGeometry<Frames>& auxiliary() const {return auxiliary_;}
    const CarCollisionParts<Frames>& collision() const {return collision_;}
    const CarWheelGeometry<Frames>& wheels() const {return wheels_;}
    const CarAxes<Frames>& axes() const {return axes_;}
    const CarStripConnections<Frames>& strips() const {return strips_;}
    const CarDeformableParts<Frames>& deformables() const {return deformables_;}
    CarDeformableParts<Frames>& deformables() {return deformables_;}
    const CarLighting<Frames>& lighting() const {return lighting_;}
    CarPhysics& physics() {return physics_.value();}
    const CarPhysics& physics() const {return physics_.value();}
private:
    std::optional<ModelOwner> model_; //Retained until all dependent owners die.
    Frame body_{};
    std::string name_,resource_;
    Matrix initial_world_{};
    CarAuxiliaryGeometry<Frames> auxiliary_;
    CarCollisionParts<Frames> collision_;
    CarWheelGeometry<Frames> wheels_;
    CarAxes<Frames> axes_;
    CarStripConnections<Frames> strips_;
    CarDeformableParts<Frames> deformables_;
    CarLighting<Frames> lighting_;
    std::optional<CarPhysics> physics_;
};

} //namespace hd2::game
#endif
