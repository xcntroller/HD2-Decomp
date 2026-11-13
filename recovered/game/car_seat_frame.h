#ifndef HD2_GAME_CAR_SEAT_FRAME_H
#define HD2_GAME_CAR_SEAT_FRAME_H
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <exception>

namespace hd2::game {
//004D9E10, Actor90. Borrow the SAME authored seat and occupant model. Native
//seat38 is a NAME index (Actor94), not this frame. Lookup may temporarily
//unlink the occupant; link/update finishes before returning even a NULL pick.
template<class Car,class Frames,class World>
auto car_seat_camera_frame(Car& car,std::int32_t seat,Frames& frames,World& world){
    using Frame=typename Frames::Frame;
    if(!car.geometry||!car.loading)throw std::logic_error("Car seat frame requires actual geometry/loading owners");
    const auto& seats=car.geometry->auxiliary().seats();
    if(seat<0||static_cast<std::size_t>(seat)>=seats.size())return Frame{};
    const auto flags=car.loading->reaction_seat_flags(seat);
    const auto occupant=[&]{
        if(static_cast<std::size_t>(seat)>=car.occupants.size())throw std::logic_error("Car seat frame lost actual occupant array");
        return car.occupants[static_cast<std::size_t>(seat)];
    };
    const auto key=[&]{return world.human_view_offset_enabled()||world.human_car_seat_hides_cursor(car,seat)?"cameraFPV*":"camera3RD*";};
    if(!(flags&0x10u)&&seat!=0&&!world.human_pose_actor_has_control(&car,static_cast<std::uint32_t>(seat))){
        const auto* human=occupant();if(!human)throw std::logic_error("Car passenger camera requires actual occupant");
        const auto* name=key();return frames.actor_find(human->base().frame(),name,0x10020);
    }
    const auto marker=[&]{
        const auto value=seats[static_cast<std::size_t>(seat)].frame;
        if(!value)throw std::logic_error("Car seat frame requires actual authored marker");
        return value;
    };
    const auto* human=occupant();const auto model=human?human->base().frame():Frame{};
    if(human&&!model)throw std::logic_error("Car seat frame lost occupant model");
    if(model)frames.human_seat_link(model,Frame{});
    struct Restore {
        Frames& frames;const decltype(occupant)& current_occupant;const decltype(marker)& current_marker;bool active=true;
        void finish(){if(const auto* human=current_occupant()){
            frames.human_seat_link(human->base().frame(),current_marker());
            //Native reloads the occupant/model after the link call as well.
            const auto* current=current_occupant();if(!current)throw std::logic_error("Car camera relink lost actual occupant");
            frames.update_scene_object(current->base().frame());
        }active=false;}
        ~Restore(){if(active)try{finish();}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
    } restore{frames,occupant,marker};
    const auto* name=key();auto result=frames.actor_find(marker(),name,0x10020);
    if(!result){
        name=key();const auto parent=frames.actor_parent(marker());
        if(!parent)throw std::logic_error("Car seat camera fallback requires actual marker parent");
        result=frames.actor_find(parent,name,0x10020);
    }
    restore.finish();return result;
}
}
#endif
