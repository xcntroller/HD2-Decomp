#ifndef HD2_GAME_CAR_SKID_MODELS_H
#define HD2_GAME_CAR_SKID_MODELS_H

#include "game/car_wheel_runtime.h"
#include "game/car_definition_tables.h"

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
    struct Pair {Owner first,second;bool active=false,continuation=false,second_visible=false;}; //Native bytes8/9/A.
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
    void update(std::size_t index,CarWheelRuntime<Frames>& wheel,const CarMaterialRegistry& materials){ //004FCBD0.
        const auto flags=wheel.flags;
        if((flags&0x400u)||!(flags&8u)||!materials.responses().at(materials.indices().at(wheel.surface_id)).flag2d||
                !wheel.previous_query_identity||(wheel.previous_query_flags&0x10000u)||wheel.scalars.get(0x21c)<0){
            retire(index,wheel.skid_cursor);return;
        }
        auto& pairs=wheels_.at(index);const auto cursor=wheel.skid_cursor;if(cursor>=pairs.size())return;
        auto& pair=pairs[cursor];const auto& normal=wheel.vectors.at(0x80);const auto& velocity=wheel.vectors.at(0xa0);
        const auto contact=add(wheel.vectors.at(0x50),scale(normal,.05000000074505806f));
        const auto width=wheel.scalars.get(0x214);
        if(!pair.active)frames_.object_visible(pair.first.get(),true);
        if(!pair.active){
            pair.active=true;wheel.vectors[0x120]=contact;
            frames_.car_suspension_position(pair.first.get(),contact);frames_.car_projector_scale(pair.first.get(),{{width,1,.01f,0}});
            frames_.update_scene_object(pair.first.get());pair.continuation=true;wheel.vectors[0x110]=normalize(velocity);return;
        }
        auto start=wheel.vectors.at(0x120),delta=subtract(contact,start);auto length=magnitude(delta);
        if(length<.05000000074505806f)return;
        if(length>5.f){retire(index,wheel.skid_cursor);return;}
        auto direction=normalize(delta);
        if(dot(velocity,wheel.vectors.at(0x110))<0){retire(index,wheel.skid_cursor);update(index,wheel,materials);return;}
        if(!pair.continuation){
            auto side=cross(direction,normal);const auto& left=wheel.vectors.at(0xf0);const auto& right=wheel.vectors.at(0x100);
            const auto edge=subtract(left,right);const auto edge_length=magnitude(edge);const auto previous=normalize(edge);
            const bool positive=dot(direction,previous)>=0;
            start=positive?subtract(left,scale(side,width*.5f)):add(right,scale(side,width*.5f));
            auto corner_direction=add(side,previous);if(positive)corner_direction=scale(corner_direction,-1);
            const auto other=positive?subtract(left,scale(side,width)):add(right,scale(side,width));
            const auto half=magnitude(subtract(positive?right:left,other))*.5f;
            const auto corner_length=static_cast<float>(std::sqrt(static_cast<long double>(edge_length)*edge_length-static_cast<long double>(half)*half));
            auto& corner=pairs[cursor?cursor-1:pairs.size()-1];
            if(!corner.second_visible){frames_.object_visible(corner.second.get(),true);corner.second_visible=true;}
            frames_.car_body_pose(corner.second.get(),positive?left:right,corner_direction,normal);
            frames_.car_projector_scale(corner.second.get(),{{half+half,1,corner_length,0}});frames_.update_scene_object(corner.second.get());
            delta=subtract(contact,start);length=magnitude(delta);direction=normalize(delta);
        }
        if(length<=.30000001192092896f){
            frames_.car_body_pose(pair.first.get(),start,direction,normal);
            frames_.car_projector_scale(pair.first.get(),{{width,1,length,0}});frames_.update_scene_object(pair.first.get());return;
        }
        auto side=scale(normalize(cross(direction,normal)),width*.5f);
        wheel.vectors[0xf0]=side;wheel.vectors[0x100]=scale(side,-1);
        frames_.car_body_pose(pair.first.get(),start,direction,normal);
        frames_.car_projector_scale(pair.first.get(),{{width,1,.3f,0}});frames_.update_scene_object(pair.first.get());
        pair.active=false;pair.continuation=false;
        wheel.skid_cursor=cursor+1;if(wheel.skid_cursor>=pairs.size())wheel.skid_cursor=0;
        start=add(start,scale(direction,.3f));
        wheel.vectors[0xf0]=add(start,wheel.vectors.at(0xf0));wheel.vectors[0x100]=add(start,wheel.vectors.at(0x100));wheel.vectors[0x120]=start;
        auto& next=pairs[wheel.skid_cursor];
        frames_.car_suspension_position(next.first.get(),start);frames_.car_projector_scale(next.first.get(),{{width,1,.01f,0}});frames_.update_scene_object(next.first.get());
        if(!next.active)frames_.object_visible(next.first.get(),true);
        next.active=true;next.continuation=false;wheel.vectors[0x110]=normalize(velocity);
        update(index,wheel,materials);
    }
    void clear_wheel(std::size_t index){
        auto& wheel=wheels_.at(index);
        while(!wheel.empty()){wheel.back().first.reset();wheel.back().second.reset();wheel.pop_back();}
    }
    void hide_first(){ //004E95B0 bit2: ascending wheels, reverse pairs, first visual only.
        for(auto& pairs:wheels_)for(auto n=pairs.size();n;){auto& pair=pairs[--n];pair.active=false;frames_.object_visible(pair.first.get(),false);}
    }
    void clear(){ //004EC5C0: ascending wheels, reverse pairs, first then second release.
        for(std::size_t n=0;n<wheels_.size();++n)clear_wheel(n);
        initialized_=false;
    }
    const auto& wheels()const{return wheels_;}
private:
    using Vector=typename Frames::Vector;
    static Vector add(Vector a,const Vector& b){for(unsigned n=0;n<3;++n)a[n]+=b[n];return a;}
    static Vector subtract(Vector a,const Vector& b){for(unsigned n=0;n<3;++n)a[n]-=b[n];return a;}
    static Vector scale(Vector a,float b){for(unsigned n=0;n<3;++n)a[n]*=b;return a;}
    static long double dot(const Vector& a,const Vector& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[2])*b[2])+static_cast<long double>(a[1])*b[1];}
    static Vector cross(const Vector& a,const Vector& b){Vector c{};for(unsigned n=0;n<3;++n)c[n]=static_cast<float>(static_cast<long double>(a[(n+1)%3])*b[(n+2)%3]-static_cast<long double>(a[(n+2)%3])*b[(n+1)%3]);return c;}
    static float square(const Vector& a){return static_cast<float>((static_cast<long double>(a[2])*a[2]+static_cast<long double>(a[1])*a[1])+static_cast<long double>(a[0])*a[0]);}
    static float magnitude(const Vector& a){const auto s=square(a);std::int32_t bits;std::memcpy(&bits,&s,4);return bits<0x322bcc77?0.f:std::abs(static_cast<long double>(s)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(s)));}
    static Vector normalize(Vector a){const auto s=square(a);if(std::abs(static_cast<long double>(s)-1)>=9.99999993922529e-09f){
        if(s>=9.99999993922529e-09f){const auto f=1/std::sqrt(static_cast<long double>(s));for(unsigned n=0;n<3;++n)a[n]=static_cast<float>(f*a[n]);}
        else{const auto n=a[0]!=0?0u:a[2]!=0?2u:1u;a[n]=a[n]<0?-1.f:1.f;}}return a;}
    Frames& frames_;std::vector<std::vector<Pair>> wheels_;bool initialized_=false;
};
} //namespace hd2::game
#endif
