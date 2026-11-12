#ifndef HD2_GAME_HUD_COMPASS_LOADING_H
#define HD2_GAME_HUD_COMPASS_LOADING_H
#include "game/hud_sound_direction.h"
#include <array>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace hd2::game {
//0089BB48/4C are process state, shared across HUD lifetimes. Bind publishes0/1
//before the final compass update; close leaves these process values intact.
struct HudCompassShared {float field48=0,field4c=0;std::uint32_t field54=0;};
//00493F80/00494140, ring00493B30/00493BC0 and marker retirement00493B10.
//Actual marker loading004936C0/00493D10 and removal00493F40/00493AF0;
//Frame presentation uses actual per-Human queries and shared process randomness;
//it never drives Human/camera simulation or owns a second roster.
template<class Frames>
class HudCompassLoading {
public:
    using Frame=typename Frames::Frame;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Marker {
        FrameOwner frame;const void* human;float radius,amount;
        void retire(){if(amount!=0)amount=0;} //00493AF0; no frame call or release.
    };
    struct Ring {
        Frame us{},en{};std::uint8_t allocator8=0;
        ~Ring(){while(!markers.empty())markers.pop_back();} //Each00493B10 releases its actual frame then frees record, reverse order.
    private:
        friend class HudCompassLoading;
        std::vector<std::unique_ptr<Marker>> markers;
    };
    HudCompassLoading(Frames& frames,HudCompassShared& shared):frames_(frames),shared_(shared) {}
    ~HudCompassLoading(){close();}
    void bind(Frame scene) {
        if(ring_)throw std::logic_error("HUD compass rebound before teardown");
        if(!scene)throw std::runtime_error("HUD compass requires actual scene");
        compass_=frames_.hud_find(scene,"kompas",0xffff);cover_=frames_.hud_find(scene,"kmpscvr",0xffff);arrow_=frames_.hud_find(scene,"strelka",0xffff);
        ring_=std::make_unique<Ring>(); //00493B30: null allocator/owner state and two optional artwork borrowers.
        ring_->allocator8=static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(scene)); //Native copies low BYTE of scene stack argument into dummy allocator storage.
        ring_->us=frames_.hud_find(scene,"kul_us",0xffff);ring_->en=frames_.hud_find(scene,"kul_en",0xffff);
        if(ring_->us)frames_.object_visible(ring_->us,false);
        if(ring_->en)frames_.object_visible(ring_->en,false);
        fields60_.fill(0);
        for(unsigned index=0;index!=8;++index)for(unsigned kind=0;kind!=2;++kind) {
            require(compass_);std::array<char,32> name{};std::snprintf(name.data(),name.size(),kind?"kmps_rd%02u":"kmps_bl%02u",index);
            auto& frame=markers_[index*2+kind];frame=frames_.actor_find(compass_,name.data(),0xffff);require(frame);frames_.object_visible(frame,false);
        }
        fields0_.fill(0);shared_.field4c=1;shared_.field48=0;frames_.update_scene_object(compass_);
    }
    void close()noexcept {ring_.reset();compass_=Frame{};} //00494140 clears ring then root only.
    void show(bool visible){if(compass_)frames_.object_visible(compass_,visible);} //00494170, null root tolerated, no update.
    template<class Human>void add_human(Human* human,std::uint32_t mode,std::uint32_t difficulty) { //00493D10→004936C0.
        if(!human||(mode&7u)!=7u||difficulty==4||!ring_||!ring_->us||!ring_->en)return;
        const auto source=human->properties.field19c==2?ring_->en:ring_->us;
        auto frame=frames_.create_hud_inventory_visual(); //Actual OBJ_ factory; same frame release, not a model cache.
        if(!frame.get())throw std::runtime_error("HUD compass marker factory returned null");
        frames_.hud_inventory_duplicate(frame.get(),source);frames_.object_visible(frame.get(),true);
        frames_.human_equipment_link(frame.get(),frames_.object_parent(source),1);
        const auto position=frames_.actor_position(source);frames_.car_suspension_position(frame.get(),position); //Native local B0 and flag80 guard.
        const auto square=(static_cast<long double>(position[2])*position[2]+static_cast<long double>(position[1])*position[1])+static_cast<long double>(position[0])*position[0];
        const auto stored=static_cast<float>(square);std::int32_t bits;std::memcpy(&bits,&stored,4);
        const float radius=bits<0x322bcc77?0.f:std::abs(square-1)>=9.99999993922529e-09f?static_cast<float>(std::sqrt(static_cast<long double>(stored))):1.f;
        frames_.object_rotation(frame.get(),frames_.actor_rotation(source));frames_.update_scene_object(frame.get());
        frames_.effects_frame_fade_alpha(frame.get(),0.f); //Actual visual virtualA8, native argument0 bits.
        ring_->markers.push_back(std::make_unique<Marker>(Marker{std::move(frame),human,radius,1.f})); //Native appends without dedup.
    }
    template<class Human>void remove_human(Human* human) { //00493F40; reverse first match, does NOT erase/release.
        if(!human||!ring_)return;
        for(auto n=ring_->markers.size();n;){auto& marker=*ring_->markers[--n];if(marker.human==human){marker.retire();return;}}
    }
    std::size_t marker_count()const{return ring_?ring_->markers.size():0;}
    const Marker& marker(std::size_t index)const{if(!ring_)throw std::logic_error("HUD compass marker before actual ring");return *ring_->markers.at(index);}
    template<class Direction,class Players>void sound_direction(const Direction& direction,std::uint32_t kind,Players& players) { //004947E0.
        if(!players.current_player())return; //0045D1D0, checked BEFORE reading input.
        fields60_[hud_sound_direction_sector(direction[0],direction[2])*2+(kind!=0)]=500;
    }
    template<class Queries>void update(std::uint32_t elapsed,Queries& queries){ //00494190, independent of root visibility.
        update_heading(queries); //00494590 BEFORE sway00494360.
        const float sway=static_cast<float>(update_sway(elapsed,queries));
        const auto step=static_cast<long double>(signed_word(elapsed))*.01f;
        auto difference=static_cast<long double>(scalar(0))-scalar(1);difference=wrap(difference);
        auto velocity=step*std::sin(difference)+scalar(3);scalar(3,static_cast<float>(velocity));
        if(velocity>0){velocity-=step*.03f;scalar(3,static_cast<float>(velocity));if(velocity<0)scalar(3,0);}
        else if(velocity<0){velocity+=step*.03f;scalar(3,static_cast<float>(velocity));if(velocity>0)scalar(3,0);}
        auto speed=scalar(3);if(!(speed<.35f))speed=.35f;else if(!(speed>-.35f))speed=-.35f;scalar(3,speed);
        auto heading=static_cast<long double>(speed)*step+scalar(1);scalar(1,static_cast<float>(heading));
        if(heading>pi){heading-=tau;scalar(1,static_cast<float>(heading));}
        if(heading< -pi)heading+=tau;scalar(1,static_cast<float>(heading));
        require(cover_);frames_.first_person_flash_rotation(cover_,static_cast<float>(-heading));frames_.update_scene_object(cover_);
        require(arrow_);frames_.first_person_flash_rotation(arrow_,-sway);frames_.update_scene_object(arrow_);
        update_sound(elapsed); //00494750 BEFORE ring00493C60, including hidden sectors.
        if(ring_){auto* current=queries.current_player();
            for(auto n=ring_->markers.size();n;){--n;if(!update_marker(*ring_->markers[n],elapsed,current,queries)){
                //00493C60 destroys first, then swap-pops the reverse-selected slot.
                ring_->markers[n].reset();if(n+1!=ring_->markers.size())ring_->markers[n]=std::move(ring_->markers.back());ring_->markers.pop_back();
            }}
        }
    }
    void discard_borrowers()noexcept{compass_=cover_=arrow_=Frame{};markers_.fill(Frame{});}
    Frame compass()const{return compass_;}Frame cover()const{return cover_;}Frame arrow()const{return arrow_;}
    const auto& ring()const{return ring_;}const auto& markers()const{return markers_;}const auto& fields60()const{return fields60_;}const auto& fields0()const{return fields0_;}
private:
    static constexpr float pi=3.1415927410125732421875f,tau=6.283185482025146484375f;
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    float scalar(unsigned n)const{float value;std::memcpy(&value,&fields0_[n],4);return value;}
    void scalar(unsigned n,float value){std::memcpy(&fields0_[n],&value,4);}
    static long double wrap(long double value){if(value>pi)value-=tau;if(value< -pi)value+=tau;return value;}
    template<class Queries>void update_heading(Queries& queries){ //00494590, original horizontal normalization and engine AngleTo.
        auto* human=queries.current_player();if(!human)return;
        auto direction=queries.target_direction(*human);direction[1]=0;
        const auto extended=static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0];
        const float square=static_cast<float>(extended);
        if(!(std::abs(extended-1)<9.99999993922529e-09f)){
            if(!(square<9.99999993922529e-09f)){const auto inverse=1/std::sqrt(static_cast<long double>(square));for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(direction[n]*inverse);}
            else if(direction[0]!=0)direction[0]=direction[0]<0?-1.f:1.f;
            else if(direction[2]!=0)direction[2]=direction[2]<0?-1.f:1.f;
            else direction[1]=1;
        }
        const auto angle=frames_.car_vector_angle(direction,{{0,0,1,0}});scalar(0,angle);
        if((static_cast<long double>(direction[2])+direction[1])*0.f+direction[0]<0)scalar(0,static_cast<float>(static_cast<long double>(tau)-angle));
        auto value=static_cast<long double>(scalar(0));if(value>pi)value-=tau;scalar(0,static_cast<float>(value));if(value< -pi)value+=tau;scalar(0,static_cast<float>(value));
    }
    template<class Queries>long double update_sway(std::uint32_t elapsed,Queries& queries){ //00494360, process54 cadence survives HUD rebinding.
        if(scalar(2)>0){scalar(2,static_cast<float>(static_cast<long double>(scalar(2))-static_cast<long double>(signed_word(elapsed))*shared_.field4c));if(!(scalar(2)>0))scalar(2,0);}
        auto* human=queries.current_player();if(human&&queries.moving(*human)){
            const auto flags=queries.movement_flags(*human);const bool side=queries.movement_side(*human);
            if(side!=flag_a0_){flag_a0_=side;
                if(shared_.field54)--shared_.field54;
                else{
                    scalar(2,flags&0x10000u?.2f:flags&0x4000u?.1f:flags&0x2000u?.05f:.025f);
                    shared_.field4c=flags&0x10000u?.00018f:flags&0x4000u?.00015f:flags&0x2000u?.0001f:.00005f;
                    shared_.field54=queries.random_word()%4u+3u;
                    scalar(0,static_cast<float>(static_cast<long double>(scalar(0))-(static_cast<long double>(frames_.volume_random_unit())*2.5+1.5)*(flag_a0_?1.f:-1.f)*scalar(2)));
                }
            }
        }
        return std::sin(static_cast<long double>(queries.time())*.03f)*scalar(2)+std::sin(static_cast<long double>(scalar(1))*2)*.10000000149011612;
    }
    void update_sound(std::uint32_t elapsed){ //00494750, signed wrapped countdown; alpha uses OLD countdown.
        for(unsigned n=16;n;){--n;const auto frame=markers_[n];require(frame);auto& time=fields60_[n];
            if(time){if(!frames_.hud_frame_visible(frame))frames_.object_visible(frame,true);
                frames_.effects_frame_fade_alpha(frame,static_cast<float>(static_cast<long double>(signed_word(time))*.002f));
                time-=elapsed;if(signed_word(time)<0)time=0;
            }else frames_.object_visible(frame,false);
            frames_.update_scene_object(frame);
        }
    }
    template<class Human,class Queries>bool update_marker(Marker& marker,std::uint32_t elapsed,Human* current,Queries& queries){ //00493860.
        const auto frame=marker.frame.get();require(frame);float alpha=frames_.effects_human_fade_amount(frame);
        Human* human=nullptr;
        if(marker.amount!=0){human=queries.resolve_human(marker.human);
            if(queries.health(*human)<1||queries.suppressed(*human)||queries.unavailable(*human))marker.retire();
        }
        const auto step=static_cast<long double>(signed_word(elapsed))*.001f;
        if(marker.amount==0){const auto value=static_cast<long double>(alpha)-step;alpha=static_cast<float>(value);if(value<=0)return false;frames_.effects_frame_fade_alpha(frame,alpha);}
        else if(alpha<1){const auto value=static_cast<long double>(alpha)+step;alpha=static_cast<float>(value);if(value>1)alpha=1;frames_.effects_frame_fade_alpha(frame,alpha);}
        if(!current||current==marker.human){frames_.object_visible(frame,false);return true;}
        if(!human)human=queries.resolve_human(marker.human);
        if(!frames_.hud_frame_visible(frame))frames_.object_visible(frame,true);
        const auto first=queries.current_direction(*current);const auto second=queries.current_direction(*current); //Original reads Z, then X afresh.
        std::array<float,2> forward{{second[0],first[2]}};normalize_horizontal(forward);
        const auto origin=frames_.actor_world_matrix(queries.model(*current));const auto target=frames_.actor_world_matrix(queries.model(*human));
        std::array<float,2> delta{{static_cast<float>(static_cast<long double>(target[12])-origin[12]),static_cast<float>(static_cast<long double>(target[14])-origin[14])}};normalize_horizontal(delta);
        const auto angle=wrap(std::atan2(static_cast<long double>(forward[1]),static_cast<long double>(forward[0]))-std::atan2(static_cast<long double>(delta[1]),static_cast<long double>(delta[0])));
        auto position=frames_.actor_position(frame);position[0]=static_cast<float>(std::sin(angle)*marker.radius);position[1]=static_cast<float>(std::cos(angle)*marker.radius);
        frames_.car_suspension_position(frame,position);frames_.update_scene_object(frame);return true;
    }
    static void normalize_horizontal(std::array<float,2>& vector){const auto length=std::sqrt(static_cast<long double>(vector[0])*vector[0]+static_cast<long double>(vector[1])*vector[1]);
        if(length>9.99999993922529e-09f){const auto inverse=1/length;for(auto& value:vector)value=static_cast<float>(value*inverse);}
    }
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD compass artwork is missing");}
    Frames& frames_;HudCompassShared& shared_;Frame compass_{},cover_{},arrow_{};
    std::unique_ptr<Ring> ring_;std::array<Frame,16> markers_{};std::array<std::uint32_t,16> fields60_{};std::array<std::uint32_t,4> fields0_{};bool flag_a0_=false;
};
} //namespace hd2::game
#endif
