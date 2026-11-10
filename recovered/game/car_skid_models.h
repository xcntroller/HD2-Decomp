#ifndef HD2_GAME_CAR_SKID_MODELS_H
#define HD2_GAME_CAR_SKID_MODELS_H

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

//004EA780's real shlaif01/02 model pool, initialized after wheel state.
//Each wheel receives up to004E2900's678=50 retained frame pairs. The temporary
//models use the SAME category1 cache; factory absence skips that attempt and
//a failed model load ends all remaining pool loading as native.
template<class Frames>class CarSkidModels {
public:
    using Owner=decltype(std::declval<Frames&>().car_skid_frame(std::declval<typename Frames::Frame>()));
    struct Pair {Owner first,second;bool active=false;};
    explicit CarSkidModels(Frames& frames,std::size_t wheel_count):frames_(frames),wheels_(wheel_count){}
    ~CarSkidModels(){clear();}
    CarSkidModels(const CarSkidModels&)=delete;CarSkidModels& operator=(const CarSkidModels&)=delete;
    void load(std::uint32_t variant_flags,unsigned count=50){
        if(initialized_)throw std::logic_error("Car skid models initialized twice");
        initialized_=true;
        if(variant_flags&1u)return;
        for(auto wheel=wheels_.size();wheel;){
            auto& pairs=wheels_[--wheel];
            for(unsigned slot=0;slot<count;++slot){
                auto first=frames_.create_car_ancillary_model();auto second=frames_.create_car_ancillary_model();
                if(!first||!second)continue;
                const auto first_status=frames_.load_car_skid_model(first.get(),"shlaif01.i3d");
                const auto second_status=frames_.load_car_skid_model(second.get(),"shlaif02.i3d");
                if(first_status||second_status){first.reset();second.reset();return;}
                auto first_frame=frames_.car_skid_frame(first.get());
                frames_.car_link_skid_root(first_frame.get());frames_.object_visible(first_frame.get(),false);frames_.update_scene_object(first_frame.get());
                auto second_frame=frames_.car_skid_frame(second.get());
                frames_.car_link_skid_root(second_frame.get());frames_.object_visible(second_frame.get(),false);frames_.update_scene_object(second_frame.get());
                pairs.push_back({std::move(first_frame),std::move(second_frame),false});
                first.reset();second.reset();
            }
        }
    }
    void retire(std::size_t wheel,std::uint32_t& cursor){ //004FE610, no frame visibility/release calls.
        auto& pairs=wheels_.at(wheel);
        if(cursor>=pairs.size()||!pairs[cursor].active)return;
        pairs[cursor].active=false;++cursor;if(cursor>=pairs.size())cursor=0;
    }
    void clear_wheel(std::size_t index){
        auto& wheel=wheels_.at(index);
        while(!wheel.empty()){wheel.back().first.reset();wheel.back().second.reset();wheel.pop_back();}
    }
    void clear(){ //004EC5C0: ascending wheels, reverse pairs, first then second release.
        for(std::size_t n=0;n<wheels_.size();++n)clear_wheel(n);
        initialized_=false;
    }
    const auto& wheels()const{return wheels_;}
private:
    Frames& frames_;std::vector<std::vector<Pair>> wheels_;bool initialized_=false;
};
} //namespace hd2::game
#endif
