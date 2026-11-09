#ifndef HD2_HOST_GAME_CAR_CALLBACK_FRAMES_H
#define HD2_HOST_GAME_CAR_CALLBACK_FRAMES_H

#include "host/game_actor_services.h"
#include "game/car_callback_services.h"

namespace hd2::host {

//Shared actual frame/identity/time endpoints for Car callbacks. The containing
//runtime must still implement every live actor/item/script/effect/network and
//particle service inherited as pure virtual; this bridge cannot be instantiated
//as a successful stand-in for those owners.
class GameCarCallbackFrames : public game::CarCallbackServices<GameActorServices> {
public:
    GameCarCallbackFrames(GameActorServices& frames,const std::uint32_t& mode,const std::int32_t& local,
            const std::vector<std::int32_t>& tyres,std::function<std::uint32_t()> clock)
        : frames_(frames),mode_(mode),local_(local),tyres_(tyres),clock_(std::move(clock)) {
        if(!clock_)throw std::logic_error("Car callbacks require the actual control clock");
    }
    const std::uint32_t& car_mode_flags() const override {return mode_;}
    const std::int32_t& car_local_actor_id() const override {return local_;}
    const std::vector<std::int32_t>& car_tyre_ids() const override {return tyres_;}
    std::uint32_t car_clock() override {return clock_();}
    Actor car_actor_from_frame(Frame frame) override {
        const auto* data=frames_.find_actor_frame_data(frame);
        return data?reinterpret_cast<Actor>(data->words[0]):nullptr;
    }
    std::uint32_t actor_type(Actor actor) override {return frames_.actor_type(&require_actor(actor).base());}
    std::int32_t actor_id(Actor actor) override {return frames_.actor_id(&require_actor(actor).base());}
    Vector actor_world_position(Actor actor) override {
        const auto frame=require_actor(actor).base().frame();
        if(!frame)throw std::logic_error("Collision actor has no actual frame");
        const auto world=frames_.car_world_matrix(frame);return {{world[12],world[13],world[14],world[15]}};
    }
    void car_transform_in_place(Vector& value,const Matrix& matrix) override {frames_.car_transform_in_place(value,matrix);}
    float car_vector_angle(const Vector& a,const Vector& b) override {return frames_.car_vector_angle(a,b);}
    bool car_frame_visible(Frame frame) override {return frames_.car_frame_visible(frame);}
    std::uint32_t frame_kind(Frame frame) override {return frames_.frame_kind(frame);}
    Frame object_parent(Frame frame) override {return frames_.object_parent(frame);}
    void object_visible(Frame frame,bool value) override {frames_.object_visible(frame,value);}
    void update_scene_object(Frame frame) override {frames_.update_scene_object(frame);}
    void link_scene_object(Frame frame,Frame parent) override {frames_.link_scene_object(frame,parent);}
    Matrix car_world_matrix(Frame frame) override {return frames_.car_world_matrix(frame);}
    const game::ActorFrameData* find_actor_frame_data(Frame frame) override {return frames_.find_actor_frame_data(frame);}
    void car_direction(Frame frame,const Vector& direction) override {frames_.car_direction(frame,direction);}
    TextureOwner load_car_particle_texture(const std::string& primary,const std::string& secondary,unsigned flags) override {
        struct OwnedTexture final : Texture {
            explicit OwnedTexture(GameCarFrames::ResourceOwner value) : owner(std::move(value)) {}
            Frame get() const override {return owner.get();}
            GameCarFrames::ResourceOwner owner;
        };
        auto value=frames_.load_car_particle_texture(primary,secondary,flags);
        if(!value)return TextureOwner{};
        return TextureOwner(std::make_unique<OwnedTexture>(std::move(value)));
    }
    void car_particle_texture(Frame frame,Frame texture) override {frames_.car_particle_texture(frame,texture);}
    void car_particle_name(Frame frame,const char* name) override {frames_.car_particle_name(frame,name);}
    void car_particle_unit_scale(Frame frame) override {frames_.car_particle_unit_scale(frame);}
    void reset_car_particle(Frame frame) override {frames_.reset_car_particle(frame);}
    void car_particle_property(Frame frame,unsigned index,std::uint32_t value) override {frames_.car_particle_property(frame,index,value);}
    void car_particle_vector_property(Frame frame,unsigned index,const Vector& value) override {frames_.car_particle_vector_property(frame,index,value);}
    std::size_t car_particle_region_count(Frame frame) override {return frames_.car_particle_region_count(frame);}
    Vector car_particle_region(Frame frame,std::size_t index) override {return frames_.car_particle_region(frame,index);}
    void select_car_particle_region(Frame frame,std::size_t index) override {frames_.select_car_particle_region(frame,index);}
    void clear_car_particle_region(Frame frame,std::size_t index) override {frames_.clear_car_particle_region(frame,index);}
    void add_car_particle_region(Frame frame,const game::CarParticleRegion& region,std::int32_t index) override {frames_.add_car_particle_region(frame,region,index);}
    std::string car_particle_texture_name(Frame frame) override {return frames_.car_particle_texture_name(frame);}
    void activate_car_particle(Frame frame,bool active) override {frames_.activate_car_particle(frame,active);}

protected:
    GameActorServices& frames_;
private:
    static game::SceneActor<GameActorServices>& require_actor(Actor actor) {
        if(!actor)throw std::logic_error("Car callback requires an actual native actor");return *actor;
    }
    const std::uint32_t& mode_;
    const std::int32_t& local_;
    const std::vector<std::int32_t>& tyres_;
    std::function<std::uint32_t()> clock_;
};

} //namespace hd2::host
#endif
