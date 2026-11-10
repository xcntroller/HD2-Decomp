#ifndef HD2_GAME_CAR_LOADING_SEATS_H
#define HD2_GAME_CAR_LOADING_SEATS_H

#include "game/car_scene_actor.h"
#include "game/car_seat_camera.h"
#include "frontend/cleanup.h"
#include <cstdio>

namespace hd2::game {

//004D16A0's reverse metadata/view/camera pass. The captured Car models have
//no100 gun-control seat flag; that positive producer remains explicit.
template<class Frames>class CarLoadingSeats {
public:
    using Actor=CarSceneActor<Frames>;using Camera=CarSeatCamera<Frames,Actor>;
    struct Seat {std::uint32_t flags=0;std::int32_t name=-1;std::optional<Camera> camera;};
    CarLoadingSeats(Frames& frames,Actor& actor):frames_(frames),actor_(actor){}
    void initialize(){
        const auto& seats=actor_.geometry.value().auxiliary().seats();
        if(seats.size()>actor_.fields24b4.size())throw std::runtime_error("Car seats exceed the actual ten control slots");
        if(active_)throw std::logic_error("Car loading seats initialized twice");
        active_=true;
        seats_.resize(seats.size());
        for(auto n=seats.size();n;){const auto index=--n;auto& seat=seats_[index];
            const auto marker=seats[index].frame;if(!marker)throw std::runtime_error("Car seat requires its authored marker");
            const auto text=frames_.car_user_data(marker);
            if(text.compare(0,4,"SEAT")==0)for(std::size_t offset=4;offset<text.size()&&text[offset]!=','&&text[offset]!=' ';++offset)
                seat.flags|=1u<<((static_cast<unsigned char>(text[offset])-0x30u)&31u);
            seat.name=car_attribute_number(text,"name");
            if(seat.flags&0x100u)throw std::runtime_error("Car gun-control seat requires its actual mounted control owner");
            view(index,false);
            const auto camera=frames_.car_find(marker,"camera3RD*",0x10020);
            if(camera&&!(seat.flags&0x120u))seat.camera.emplace(frames_,actor_,camera);
        }
    }
    void view(std::size_t index,bool visible){ //004DA090, actual C4 slot.
        if(index>=actor_.fields24b4.size())throw std::out_of_range("Car view control index");
        if(actor_.fields24b4[index])throw std::runtime_error("Car view requires its actual mounted control receiver");
        char name[32];std::snprintf(name,sizeof(name),"VIEW%02u",static_cast<unsigned>(index));
        if(const auto frame=frames_.car_find(actor_.base().frame(),name,0x10001))frames_.object_visible(frame,visible);
    }
    void restore_camera(std::size_t index){if(auto& camera=seats_.at(index).camera)camera->restore();}
    void release_cameras(){
        frontend::CleanupErrors errors;
        for(auto& seat:seats_)if(seat.camera){errors.attempt([&]{seat.camera->restore();});seat.camera.reset();}
        active_=false;errors.rethrow();
    }
    template<class Queries>void update(Queries& queries,const typename Frames::Vector& direction,const typename Frames::Vector& velocity,
            float speed,const Actor* current_attachment,CarSeatCameraShared& shared){
        for(auto& seat:seats_)if(seat.camera)seat.camera->update(queries,direction,velocity,speed,current_attachment,shared);
    }
    auto& seats(){return seats_;}const auto& seats()const{return seats_;}
private:
    Frames& frames_;Actor& actor_;std::vector<Seat> seats_;bool active_=false;
};

} //namespace hd2::game
#endif
