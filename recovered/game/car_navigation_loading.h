#ifndef HD2_GAME_CAR_NAVIGATION_LOADING_H
#define HD2_GAME_CAR_NAVIGATION_LOADING_H

#include "game/mission_navigation.h"
#include "game/car_geometry_math.h"
#include "game/detached_camera.h"

namespace hd2::game {

//004DB060/004DAB60's loading marks and004DAC80/004DAFE0's moving marks.
//Borrows the SAME decoded navigation table and real Car model/BODY. Table
//storage must outlive this owner. No actor-local copy of navigation flags.
//The other004DAB60 counter-mark producer is not supplied by this owner.
template<class Frames>class CarNavigationLoading {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarNavigationLoading(Frames& frames,MissionNavigation& navigation)
        :frames_(frames),navigation_(navigation){}
    ~CarNavigationLoading(){close();}
    CarNavigationLoading(const CarNavigationLoading&)=delete;
    CarNavigationLoading& operator=(const CarNavigationLoading&)=delete;
    void close()noexcept{
        clear_moving();
        for(auto row=loading_.rbegin();row!=loading_.rend();++row)row->record->flags&=0xfdffu;
        loading_.clear();
    }
    void load(Frame model,Frame body,std::uint32_t mode,float speed,const Vector& velocity){ //004DB060.
        close();
        if(!model||!body)throw std::runtime_error("Car navigation requires actual model and BODY");
        const auto world=frames_.actor_world_matrix(model);
        const auto start=navigation_.nearest_volume({{world[12],world[13],world[14]}},5.f,-1.f);
        if(start!=0xffffu){
            std::vector<std::uint16_t> points;
            navigation_.append_reachable(start,15.f,points);
            points.push_back(start); //Native explicitly appends even though the walk already includes it.
            const auto inverse=frames_.car_inverse_matrix(frames_.actor_world_matrix(body));
            for(auto point=points.rbegin();point!=points.rend();++point){
                const auto position=navigation_.position(*point);
                //Native computes this transform but applies NO bounds/inside test.
                (void)frames_.car_transform(Vector{{position[0],position[1],position[2],0}},inverse);
                auto* record=navigation_.record(*point);
                if(!record)throw std::runtime_error("Car navigation references an unavailable point");
                loading_.push_back({*point,record});record->flags|=0x200u;
            }
        }
        refresh_moving(model,mode,speed,velocity);
    }
    void refresh_moving(Frame model,std::uint32_t mode,float speed,const Vector& velocity){ //004DAC80.
        if((mode&7u)!=7u)return;
        clear_moving(); //004DAFE0 precedes the speed gate.
        if(!(speed>=.10000000149011612f))return;
        if(!model)throw std::runtime_error("Car moving navigation requires its actual model");
        const auto world=frames_.actor_world_matrix(model);
        const auto start=navigation_.nearest_volume({{world[12],world[13],world[14]}},5.f,-1.f);
        std::vector<std::uint16_t> points;navigation_.append_reachable(start,16.f,points);
        const auto direction=car_normalize(velocity);
        const HumanVector first{{world[12],world[13],world[14],0}};
        HumanVector last{};
        for(unsigned axis=0;axis!=3;++axis){
            const float displacement=direction[axis]*10.f;
            last[axis]=displacement+first[axis];
        }
        for(auto point=points.rbegin();point!=points.rend();++point){
            const auto position=navigation_.position(*point);float parameter;
            if(camera_segment_square({{position[0],position[1],position[2],0}},first,last,parameter)<4.f){
                auto* record=navigation_.record(*point);
                if(!record)throw std::runtime_error("Car moving navigation references an unavailable point");
                moving_.push_back(record);record->flags|=0x2000u;
            }
        }
    }
    std::size_t loading_marks()const{return loading_.size();}
    void observe_loading_marks()const{ //004D3336..335D: reverse game54/0051D5A0; result unused.
        for(auto row=loading_.rbegin();row!=loading_.rend();++row)(void)navigation_.record(row->identity);
    }
    std::size_t moving_marks()const{return moving_.size();}
private:
    void clear_moving()noexcept{
        for(auto row=moving_.rbegin();row!=moving_.rend();++row)(*row)->flags&=0xdfffu;
        moving_.clear();
    }
    Frames& frames_;MissionNavigation& navigation_;
    struct LoadingMark{std::uint16_t identity;MissionTableRecord* record;};
    std::vector<LoadingMark> loading_;std::vector<MissionTableRecord*> moving_;
};
} //namespace hd2::game
#endif
