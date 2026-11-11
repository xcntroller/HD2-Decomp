#ifndef HD2_GAME_HUMAN_ANIMATION_TRACKS_H
#define HD2_GAME_HUMAN_ANIMATION_TRACKS_H

#include "game/human_joint_owners.h"
#include "game/human_ai_shot_direction.h"
#include <cmath>
#include <limits>

namespace hd2::game {

template<class Words>void normalize_human_primary_animation(Words& words,unsigned base){ //00782EB0.
    auto id=words.template get<std::int32_t>(base),detail=words.template get<std::int32_t>(base+4);
    switch(id){case 26:case 32:detail=0;break;case 27:case 30:detail=1;break;case 31:case 33:detail=2;break;default:break;}
    words.set(base+4,detail);
    if(id>=45&&id<=65){
        if(detail==2&&id<63)id=id==46||id==56?64:63;
        else if(detail==1&&(id<55||id>62))id=id==46||id==64?56:55;
        else if(detail==0&&id>54)id=id==56||id==64?46:45;
        words.set(base,id);
    }
    words.set(base+12,std::uint8_t{1});
}

//DD0+260:007720C0/007723B0,007733D0/00773260 and007724E0's track owner.
//Frame/animation resources are real borrowed objects. Callback registration,
//player queries and the live0084E80C scale are mandatory containing services.
//This owner does not supply animation-marker sound playback or DD0 activation.
template<class Frames,class Callbacks>
class HumanAnimationTracks {
public:
    using Frame=typename Frames::Frame;
    struct Request {Frame animation{};float weight=1;std::uint32_t flags=0;float blend=0;bool separate=false;};
    using Requests=std::array<Request,4>;
    using Primary=std::array<std::int32_t,3>;
    using Secondary=std::array<std::int32_t,2>;
    HumanAnimationTracks(Frames& frames,Callbacks& callbacks):frames_(frames),callbacks_(callbacks) {}
    ~HumanAnimationTracks(){clear();}
    HumanAnimationTracks(const HumanAnimationTracks&)=delete;
    HumanAnimationTracks& operator=(const HumanAnimationTracks&)=delete;
    void initialize(Frame model) {
        if(!model)throw std::runtime_error("Human animation tracks require the actual model");
        if(started_&&model_!=model)throw std::runtime_error("Live Human animation model replacement is not reconstructed");
        model_=model;started_=true;
        try {
            for(auto offset:{0x358u,0x35cu,0x37cu,0x380u,0x398u,0x39cu})words_.set(offset,std::uint32_t{0});
            for(auto offset:{0x360u,0x384u,0x3a0u})words_.set(offset,std::uint8_t{0});
            const auto parent=frames_.object_parent(model);
            if(!parent)throw std::runtime_error("Human animation model has no actual parent");
            parent_marker_=frames_.actor_find(parent,"a",0xffff);
            for(unsigned group=0;group!=4;++group) {
                const auto base=group*0x2c;
                words_.set(base+0xc,std::int32_t{-1});words_.set(base+0x10,std::int32_t{-1});
                for(auto offset:{0x14u,0x2cu,0x30u})words_.set(base+offset,std::uint32_t{0});
                words_.set(base+0x34,std::uint8_t{0});
            }
            //Original initializes SEVEN metadata records while resetting eight
            //engine tracks. The unused eighth record is not read for its rates.
            for(unsigned slot=0;slot!=7;++slot) {
                const auto base=slot*0x24;
                for(auto offset:{0xbcu,0xc0u,0xccu})words_.set(base+offset,std::uint32_t{0});
                words_.set(base+0xc4,-10000.f);words_.set(base+0xc8,std::uint8_t{1});
                words_.set(base+0xd0,1.f);words_.set(base+0xd4,1.f);
                words_.set(base+0xd8,std::uint8_t{0});words_.set(base+0xdc,std::int32_t{-1});
            }
            for(unsigned slot=0;slot!=8;++slot)frames_.human_model_animation(model_,Frame{},slot,1.f,0);
            frames_.human_model_parameter_c4(model_,0);
            const auto marker=frames_.actor_find(model_,"a",0xffff);
            if(marker) {callbacks_.install_animation_events(marker,*this);marker_=marker;}
            for(auto offset:{0x230u,0x234u,0x228u,0x238u,0x240u,0x260u,0x264u,
                0x250u,0x254u,0x258u,0x270u,0x274u,0x278u,0x2c0u,0x2c4u,0x2c8u,0x330u})
                words_.set(offset,std::uint32_t{0});
            words_.set(0x224,std::int32_t{-1});
            for(auto offset:{0x23cu,0x25cu,0x27cu})words_.set(offset,std::uint32_t{0xffff});
            for(auto offset:{0x294u,0x2b4u,0x2f4u,0x328u,0x334u})words_.set(offset,std::int32_t{-10000});
            words_.set(0x32c,1.f);
            set_primary(0x1dc,{{206,0,1}});set_primary(0x200,{{206,0,1}}); //00782FA0 ignores its fourth stack argument.
            for(auto offset:{0x1ecu,0x210u}){words_.set(offset,std::int32_t{91});words_.set(offset+4,std::int32_t{0});}
            for(auto offset:{0x1f8u,0x21cu}){words_.set(offset,std::int32_t{75});words_.set(offset+4,std::int32_t{0});}
        }catch(...){clear();throw;}
    }
    void clear() noexcept { //007723B0, before CD0's separate shadow cleanup.
        if(!started_)return;
        attempt([&]{
            if(const auto marker=frames_.actor_find(model_,"a",0xffff))callbacks_.remove_animation_events(marker,*this);
        });
        for(unsigned slot=0;slot!=8;++slot)attempt([&]{frames_.human_model_animation(model_,Frame{},slot,1.f,0);});
        marker_=parent_marker_=model_=Frame{};started_=false;
    }
    void select(Requests& requests,const Primary& primary,const Secondary& secondary,const Secondary& third,bool force) {
        require_model();
        for(unsigned group=0;group!=3;++group) {
            auto& request=requests[group];if(!request.animation)continue;
            if(!group)words_.set(0x334,std::int32_t{-10000});
            const auto base=group*0x2c;const auto previous=integer(base+0xc);
            words_.set(base+0x14,std::int32_t{0});words_.set(base+0x10,std::int32_t{0});
            const unsigned lane=previous==0?1:previous==-1?1:0;
            words_.set(base+0xc,static_cast<std::int32_t>(lane));
            words_.set(base+0x30,frames_.human_clip_duration(request.animation));words_.set(base+0x34,std::uint8_t{0});
            const auto slot=group*2+lane;const auto meta=slot*0x24;
            if(previous==-1) {
                frames_.human_model_animation(model_,request.animation,slot,request.separate?0.f:request.weight,0);
                words_.set(meta+0xc4,0.f);
            }else {
                bool reuse=false;
                if(!group)reuse=integer(0x1dc)!=206&&get_primary(0x1dc)==primary&&real((1-lane)*0x24+0xc4)>0&&!force;
                else if(group==2)reuse=integer(0x1f8)!=75&&get_secondary(0x1f8)==third&&real((group*2+1-lane)*0x24+0xc4)>0;
                if(reuse) {
                    const auto old=(group*2+1-lane)*0x24;
                    words_.set(meta+0xc4,static_cast<float>((1.L-static_cast<long double>(real(old+0xc4))/real(old+0xc0))*request.blend));
                    request.flags=2;
                }else {
                    const auto weight=lane?0.f:request.separate?0.f:request.weight;
                    frames_.human_model_animation(model_,request.animation,slot,weight,0);words_.set(meta+0xc4,0.f);
                }
            }
            words_.set(meta+0xbc,request.weight);words_.set(meta+0xc0,request.blend);
            words_.set(meta+0xc8,static_cast<std::uint8_t>(request.separate));words_.set(meta+0xcc,std::uint32_t{0});
            words_.set(meta+0xd0,1.f);words_.set(meta+0xd4,1.f);
            if(previous!=-1)synchronize(requests,group);
        }
        if(primary[0]!=206&&requests[0].animation){set_primary(0x1dc,get_primary(0x200));set_primary(0x200,primary);}
        if(secondary[0]!=91&&requests[1].animation){set_secondary(0x1ec,get_secondary(0x210));set_secondary(0x210,secondary);}
        if(third[0]!=75&&requests[2].animation){set_secondary(0x1f8,get_secondary(0x21c));set_secondary(0x21c,third);}
    }
    void synchronize(const Requests& requests,unsigned group) { //00773260.
        if(group>=4)throw std::out_of_range("Human animation group");
        const auto lane=integer(group*0x2c+0xc);if(lane==-1)return;
        if(lane!=0&&lane!=1)throw std::runtime_error("Invalid Human animation lane");
        const auto current=group*2+static_cast<unsigned>(lane),other=group*2+static_cast<unsigned>(lane==0);
        const auto flags=requests[group].flags;
        if(!(real(other*0x24+0xc0)>0))return;
        if(flags&4u) {
            const auto time=frames_.human_model_time(model_,other);
            const auto clip=frames_.human_model_clip(model_,other);if(!clip)return;
            const auto remaining=wrap(frames_.human_clip_duration(clip),-static_cast<std::int64_t>(time));
            const auto next=frames_.human_model_clip(model_,current);if(!next)return;
            auto end=frames_.human_clip_duration(next);
            if(remaining>0){if(remaining<end)end=wrap(end,-static_cast<std::int64_t>(remaining));frames_.human_model_time(model_,current,end);}
        }else if(flags&8u) {
            const auto time=frames_.human_model_time(model_,other);
            const auto clip=frames_.human_model_clip(model_,other);if(!clip)return;
            const auto duration=frames_.human_clip_duration(clip);
            const auto next=frames_.human_model_clip(model_,current);if(!next)return;
            const auto end=frames_.human_clip_duration(next);
            if(duration>0)frames_.human_model_time(model_,current,ftol(static_cast<long double>(time)/duration*end));
        }
    }
    void update(std::int32_t elapsed,bool transitions_only,const Requests* requests=nullptr) { //007724E0's first DWORD is unused.
        require_model();words_.set(0x228,0.f);
        if(!transitions_only)for(auto offset:{0x2cu,0x58u,0x84u})words_.set(offset,std::int32_t{0});
        if(elapsed==-1&&!transitions_only)return;
        words_.set(0x330,wrap(integer(0x330),elapsed));
        if(!transitions_only) {
            std::array<std::int32_t,8> advances{};
            //Native zeroes the high half of the __ftol result before FILD.
            const float base=static_cast<float>(static_cast<std::uint32_t>(elapsed));
            for(unsigned slot=0;slot!=8;++slot) {
                callbacks_.animation_player_check(model_);
                auto value=static_cast<long double>(base)*callbacks_.animation_time_scale();
                if(slot<6)value*=static_cast<long double>(real(slot*0x24+0xd0))*real(slot*0x24+0xd4);
                advances[slot]=ftol(value);
            }
            frames_.human_model_advance(model_,advances);
            callbacks_.rethrow_animation_event_failure(); //Never let a caught engine-callback failure publish activation.
        }
        for(unsigned group=0;group!=4;++group) {
            if(transitions_only&&requests&&!(*requests)[group].animation)continue;
            const auto base=group*0x2c;const auto lane=integer(base+0xc);if(lane==-1)continue;
            if(lane!=0&&lane!=1)throw std::runtime_error("Invalid Human animation lane");
            const auto slot=group*2+static_cast<unsigned>(lane),other=group*2+static_cast<unsigned>(lane==0),meta=slot*0x24;
            if(integer(base+0x14)<1) {
                words_.set(base+0x10,ftol(static_cast<long double>(elapsed)*callbacks_.animation_time_scale()+integer(base+0x10)));
                const auto stop=integer(0x334);
                if(stop>=0&&!group&&stop<=frames_.human_model_time(model_,slot)) {
                    pause(group,stop);words_.set(0x10,integer(0x334));
                    frames_.human_model_time(model_,slot,integer(0x334));words_.set(0x334,std::int32_t{-10000});
                }
                if(integer(base+0x10)>0x7ffffc17)words_.set(base+0x10,std::int32_t{100000});
                const auto fade=integer(0x224);
                if(fade>=0&&group==3&&integer(0xb4)<=wrap(integer(0x94),fade))fade_fourth(fade);
                words_.set(meta+0xc4,static_cast<float>(static_cast<long double>(elapsed)+real(meta+0xc4)));
            }else {
                if(real(meta+0xc4)<0)continue;
                if(real(meta+0xc4)<real(meta+0xc0))words_.set(meta+0xc4,static_cast<float>(static_cast<long double>(elapsed)+real(meta+0xc4)));
            }
            if(real(meta+0xc4)<real(meta+0xc0)) {
                const float ratio=blend_ratio(real(meta+0xc4),static_cast<float>(elapsed),real(meta+0xc0));
                if(!group)words_.set(0x228,ratio);
                if(!(word(meta+0xcc)&1u)) {
                    if(real(meta+0xc0)>0) {
                        if(words_.template get<std::uint8_t>(meta+0xc8)) {
                            frames_.human_model_weight(model_,other,static_cast<float>((1.L-ratio)*real(other*0x24+0xbc)));
                            frames_.human_model_weight(model_,slot,static_cast<float>(static_cast<long double>(ratio)*real(meta+0xbc)));
                        }else if(slot<other)frames_.human_model_weight(model_,other,static_cast<float>((1.L-ratio)*real(other*0x24+0xbc)));
                        else frames_.human_model_weight(model_,slot,static_cast<float>(static_cast<long double>(ratio)*real(meta+0xbc)));
                    }
                }else {
                    const float weight=static_cast<float>((1.L-ratio)*real(meta+0xbc));
                    frames_.human_model_weight(model_,slot,weight);
                    if(weight<=frames_.human_model_weight(model_,other))frames_.human_model_weight(model_,other,weight);
                }
            }else finish_transition(group,slot,other);
        }
    }
    void pause(unsigned group,std::int32_t value) { //00775260.
        if(group>=4)throw std::out_of_range("Human animation pause group");
        const auto base=group*0x2c;
        if(!words_.template get<std::uint8_t>(base+0x34)) {
            const auto lane=integer(base+0xc);
            if(lane<0||lane>1)throw std::runtime_error("Human pause has no animation lane");
            words_.set((group*2+static_cast<unsigned>(lane))*0x24+0xd0,0.f);words_.set(base+0x14,value);
        }
    }
    long double transition_progress(unsigned group){ //00775480.
        if(group>=4)throw std::out_of_range("Human animation transition group");
        words_.set(0x228,0.f);
        const auto lane=integer(group*0x2c+0xc);
        if(lane==-1)return 0;
        if(lane<0||lane>1)throw std::runtime_error("Invalid Human animation lane");
        const auto meta=(group*2+static_cast<unsigned>(lane))*0x24;
        const float elapsed=real(meta+0xc4),duration=real(meta+0xc0);
        return elapsed<0||duration<=elapsed?1.L:blend_ratio(elapsed,0,duration);
    }
    void motion_velocity(const HumanVector& direction,float speed,float maximum,HumanVector& output){ //00773850.
        for(unsigned axis=0;axis!=4;++axis)words_.set(0x230+4*axis,word(0x250+4*axis));
        words_.set(0x240,word(0x260));
        words_.set(0x250,direction);words_.set(0x260,speed);
        const auto source=real(0x240)<speed?0x250u:0x230u;
        const auto amount=source==0x250u?speed:real(0x240);
        for(unsigned axis=0;axis!=3;++axis)output[axis]=real(source+4*axis)*amount;
        words_.set(0x264,maximum);
    }
    void refresh_motion_velocity(const HumanVector& direction,float speed,float maximum,HumanVector& output){ //00773980.
        words_.set(0x250,direction);words_.set(0x260,speed);words_.set(0x264,maximum);
        if(integer(0xc)==-1)return;
        const auto source=speed<=real(0x240)?0x230u:0x250u;
        const auto amount=source==0x230u?real(0x240):speed;
        for(unsigned axis=0;axis!=3;++axis)output[axis]=real(source+4*axis)*amount;
    }
    void motion_animation_scale(float scale){words_.set(0x32c,scale);} //0076B670, actual DD0+58C.
    void motion_animation_rate(unsigned group,float rate){ //00774D50.
        if(group>=4)throw std::out_of_range("Human animation rate group");
        const auto lane=integer(0xc+group*0x2c);
        if(lane==-1)return;
        if(lane<0||lane>1)throw std::runtime_error("Human animation rate has an invalid lane");
        words_.set(0xd0+(static_cast<unsigned>(lane)+group*2)*0x24,rate);
    }
    void motion_impulse(const HumanVector& displacement,std::int32_t duration){ //00773A90.
        words_.set(0x280,displacement);words_.set(0x290,0u);words_.set(0x294,duration);
    }
    void clear_motion_displacement(){for(unsigned axis=0;axis!=3;++axis)words_.set(0x270+4*axis,0.f);} //0074B4F0, XYZ only.
    void secondary_motion_impulse(const HumanVector& displacement,std::int32_t duration){ //00773B20.
        words_.set(0x2e0,displacement);words_.set(0x2f0,0u);words_.set(0x2f4,duration);
    }
    void motion_deceleration(const HumanVector& velocity,std::int32_t duration){ //00773B60.
        words_.set(0x320,0u);words_.set(0x300,velocity);words_.set(0x324,0u);words_.set(0x328,duration);
        auto decrement=velocity;
        const auto square=(static_cast<long double>(velocity[0])*velocity[0]+
            static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2];
        normalize_human_ai_shot(decrement,square);
        const auto stored=static_cast<float>(square);
        const auto length=stored<1.e-8f?0.L:std::abs(static_cast<long double>(stored)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(stored));
        const auto amount=length/(static_cast<long double>(duration)/80.f); //Native preserves zero-duration IEEE results.
        for(unsigned axis=0;axis!=3;++axis)decrement[axis]=static_cast<float>(amount*decrement[axis]);
        words_.set(0x310,decrement);
    }
    void advance_motion(std::int32_t elapsed,bool player){ //00772A00, SAME DD0+260 scalar owner.
        require_model();
        const float seconds=static_cast<float>(static_cast<long double>(elapsed)*.001f);
        auto effective=real(0x260);
        const auto lane=integer(0xc);
        const bool ordinary=!player||real(0x228)<=0||real(0x228)>=1;
        HumanVector velocity{};
        for(unsigned axis=0;axis!=3;++axis)velocity[axis]=real(0x250+4*axis)*real(0x260);
        if(ordinary){
            words_.set(0x240,0u);
            const auto maximum=real(0x264);
            if(maximum>0){
                const auto speed=motion_length(velocity);
                if(maximum<speed){
                    const auto inverse=1.L/speed;
                    effective=speed<=static_cast<long double>(maximum)+maximum?maximum:speed*.5f;
                    for(unsigned axis=0;axis!=3;++axis)
                        velocity[axis]=static_cast<float>(velocity[axis]*inverse*effective);
                }
            }
        }
        for(unsigned axis=0;axis!=3;++axis)words_.set(0x270+4*axis,velocity[axis]*seconds);
        if(lane==-1)return;
        if(lane<0||lane>1)throw std::runtime_error("Human motion has an invalid primary animation lane");
        if(player)effective=real(0x32c);
        else{
            if(real(0x260)==0||effective==0){
                const auto x=static_cast<long double>(real(0x250))*real(0x250);
                const auto y=static_cast<long double>(real(0x254))*real(0x254);
                const auto z=static_cast<long double>(real(0x258))*real(0x258);
                effective=(z+y)+x>=1.e-16f?1.f:.3f;
            }else{
                effective=static_cast<float>(static_cast<long double>(effective)/real(0x260));
                if(effective>1)effective=1;
                else if(effective<.4f)effective=.4f;
            }
            effective*=real(0x32c);
        }
        words_.set((lane==0?1u:0u)*0x24+0xd4,integer(0x1dc)<190?1.f:effective);
        words_.set(static_cast<unsigned>(lane)*0x24+0xd4,integer(0x200)<190?1.f:effective);
        advance_timed_displacement(0x280,0x290,0x294,elapsed);
        if(integer(0x2b4)!=-10000){
            const auto matrix=frames_.actor_world_matrix(model_);
            HumanVector delta{{real(0x2a0)-matrix[12],real(0x2a4)-matrix[13],real(0x2a8)-matrix[14],0}};
            const auto distance=motion_length(delta);
            const auto timer=wrap(integer(0x2b0),elapsed);
            words_.set(0x2b0,timer);
            if(integer(0x2b4)<timer)words_.set(0x2b4,std::int32_t{-10000});
            const auto speed=real(0x298);
            if((speed<distance&&distance!=0)||static_cast<long double>(seconds)*speed<distance)
                for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>(delta[axis]*(1.L/distance)*speed);
            for(unsigned axis=0;axis!=3;++axis)
                words_.set(0x270+4*axis,static_cast<float>(static_cast<long double>(delta[axis])*seconds+real(0x270+4*axis)));
        }
        advance_timed_displacement(0x2e0,0x2f0,0x2f4,elapsed);
        if(integer(0x328)!=-10000){
            auto step=elapsed;
            if(integer(0x328)<=wrap(integer(0x324),elapsed)){
                step=wrap(integer(0x328),-static_cast<std::int64_t>(integer(0x324)));
                words_.set(0x328,std::int32_t{-10000});
            }
            words_.set(0x324,wrap(integer(0x324),step));
            auto remaining=step;
            while(remaining>0){
                const auto accumulated=wrap(integer(0x320),remaining);
                if(static_cast<float>(accumulated)<=80.f){
                    words_.set(0x320,accumulated);
                    break;
                }
                for(unsigned axis=0;axis!=3;++axis)
                    words_.set(0x300+4*axis,real(0x300+4*axis)-real(0x310+4*axis));
                remaining=ftol(static_cast<long double>(accumulated)-80.f);
                words_.set(0x320,0u);
            }
            const float dt=static_cast<float>(static_cast<long double>(step)*.001f);
            for(unsigned axis=0;axis!=3;++axis)
                words_.set(0x270+4*axis,static_cast<float>(static_cast<long double>(dt)*real(0x300+4*axis)+real(0x270+4*axis)));
        }
        const auto x=static_cast<long double>(real(0x2c0))*real(0x2c0);
        const auto y=static_cast<long double>(real(0x2c4))*real(0x2c4);
        const auto z=static_cast<long double>(real(0x2c8))*real(0x2c8);
        if((z+y)+x>=1.e-16f){
            const auto timer=wrap(integer(0x2d0),elapsed);
            words_.set(0x2d0,timer);
            auto step=static_cast<float>(elapsed);
            if(integer(0x2d4)<timer){
                step=static_cast<float>(wrap(integer(0x2d4),-static_cast<std::int64_t>(timer)+elapsed));
                for(unsigned axis=0;axis!=3;++axis)words_.set(0x2c0+4*axis,0.f); //Native clears BEFORE adding this last portion.
            }
            const float fraction=static_cast<float>(static_cast<long double>(ftol(step))/integer(0x2d4));
            for(unsigned axis=0;axis!=3;++axis)
                words_.set(0x270+4*axis,static_cast<float>(static_cast<long double>(fraction)*real(0x2c0+4*axis)+real(0x270+4*axis)));
        }
    }
    void fade_fourth(std::int32_t duration) { //00775970.
        const auto lane=integer(0x90);if(lane==-1)return;
        if(lane<0||lane>1)throw std::runtime_error("Invalid fourth Human animation lane");
        const auto meta=(6+static_cast<unsigned>(lane))*0x24;
        if(!(word(meta+0xcc)&1u)) {
            words_.set(meta+0xc4,0.f);words_.set(meta+0xcc,word(meta+0xcc)|1u);
            words_.set(meta+0xc0,static_cast<float>(duration));words_.set(meta+0xd0,1.f);words_.set(meta+0xd4,1.f);
            words_.set(0x224,std::int32_t{-1});
        }
    }
    void fade_pair(unsigned group,std::int32_t duration) { //00774CC0: test active lane; write BOTH lanes.
        if(group>=4)throw std::out_of_range("Human animation fade group");
        const auto lane=integer(group*0x2c+0xc);if(lane==-1)return;
        if(lane<0||lane>1)throw std::runtime_error("Invalid Human animation fade lane");
        if(word((group*2+static_cast<unsigned>(lane))*0x24+0xcc)&1u)return;
        for(unsigned n=0;n!=2;++n) {
            const auto meta=(group*2+n)*0x24;
            words_.set(meta+0xc4,0.f);words_.set(meta+0xc0,static_cast<float>(duration));
            words_.set(meta+0xcc,word(meta+0xcc)|1u);words_.set(meta+0xd0,1.f);words_.set(meta+0xd4,1.f);
        }
    }
    void queue_event(const char* text,std::int32_t time) { //00774DF0, first DWORD callback argument is unused.
        if(!text)return;
        switch(static_cast<unsigned char>(*text)) {
        case '0':append_event(1,1);return;
        case '1':append_event(1,2);return;
        case '^':append_event(1,3);return;
        case 'c':append_event(1,8);return;
        case 'd':append_event(1,21);return;
        case 'e':append_event(1,text[1]=='1'?16:15);return;
        case 'f':append_event(1,17);return;
        case 'h':append_event(text[1]=='2'?2:1,text[1]=='2'?14:7);return;
        case 'k':append_event(0,24,event_count(1));return; //Native indexes queue0 with queue1's count.
        case 'l':append_event(1,text[1]=='l'?11:text[1]=='r'?12:10);return;
        case 'm':append_event(2,18);return;
        case 'n':append_event(2,20);return;
        case 'p': {
            const unsigned group=text[1]=='2'?2:0;
            if(integer(group*0x2c+0x14)<1) {
                pause(group,time);
                if(!group&&integer(0x334)!=-10000)words_.set(0x334,std::int32_t{-10000});
            }
            return;
        }
        case 's': {
            const auto threshold=180.L/callbacks_.animation_time_scale(); //FLOAT00810504 / live0084E80C.
            if(threshold>integer(0x330))return; //Native unordered comparison also takes the event branch.
            words_.set(0x330,std::int32_t{0});
            const auto count=event_count(0);
            for(unsigned index=0;index<count;++index)if(event(0,index)==4)return; //Native decrement/increment cancels.
            append_event(0,text[1]=='1'?5:text[1]=='2'?6:4);return;
        }
        case 't':append_event(1,9);return;
        case 'u':append_event(0,23,event_count(1));return; //Same original cross-queue index as k.
        case 'w':append_event(text[1]=='2'?2:1,text[1]=='2'?13:0);return;
        case 'y':append_event(1,22);return;
        case 'z':append_event(0,19);return;
        default:return;
        }
    }
    unsigned event_count(unsigned group) const {
        if(group>=4)throw std::out_of_range("Human animation event group");
        const auto count=integer(group*0x2c+0x2c);
        if(count<0||count>5)throw std::runtime_error("Human animation event queue exceeds native capacity");
        return static_cast<unsigned>(count);
    }
    std::int32_t event(unsigned group,unsigned index) const {
        if(index>=event_count(group))throw std::out_of_range("Human animation event index");
        return integer(group*0x2c+0x18+index*4);
    }
    const auto& words() const{return words_;}
    Frame model() const{return model_;}
    Frame parent_marker() const{return parent_marker_;}
private:
    static float motion_length(const HumanVector& vector){
        const float square=static_cast<float>((static_cast<long double>(vector[1])*vector[1]+
            static_cast<long double>(vector[2])*vector[2])+static_cast<long double>(vector[0])*vector[0]);
        if(square<1.e-8f)return 0;
        return std::abs(static_cast<long double>(square)-1)<1.e-8f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
    }
    void advance_timed_displacement(unsigned vector,unsigned timer,unsigned duration,std::int32_t elapsed){
        const auto limit=integer(duration);
        if(limit==-10000)return;
        const auto accumulated=wrap(integer(timer),elapsed);
        words_.set(timer,accumulated);
        auto step=static_cast<float>(elapsed);
        if(limit<accumulated){
            words_.set(duration,std::int32_t{-10000});
            step=static_cast<float>(wrap(limit,-static_cast<std::int64_t>(accumulated)+elapsed));
        }
        if(limit>0){
            const float fraction=static_cast<float>(static_cast<long double>(ftol(step))/limit);
            for(unsigned axis=0;axis!=3;++axis)
                words_.set(0x270+4*axis,static_cast<float>(static_cast<long double>(fraction)*real(vector+4*axis)+real(0x270+4*axis)));
        }
    }
    void append_event(unsigned group,std::int32_t value,unsigned index=0xffffffffu) {
        const auto count=event_count(group);if(index==0xffffffffu)index=count;
        if(count>=5||index>=5)throw std::runtime_error("Human animation event append exceeds native capacity");
        words_.set(group*0x2c+0x18+index*4,value);words_.set(group*0x2c+0x2c,static_cast<std::int32_t>(count+1));
    }
    void require_model() const{if(!started_||!model_)throw std::logic_error("Human animation tracks are not bound");}
    std::int32_t integer(unsigned offset) const{return words_.template get<std::int32_t>(offset);}
    std::uint32_t word(unsigned offset) const{return words_.template get<std::uint32_t>(offset);}
    float real(unsigned offset) const{return words_.template get<float>(offset);}
    static std::int32_t wrap(std::int32_t first,std::int64_t second) {
        const auto bits=static_cast<std::uint32_t>(first)+static_cast<std::uint32_t>(second);
        std::int32_t value;std::memcpy(&value,&bits,4);return value;
    }
    static std::int32_t ftol(long double value) {
        if(!std::isfinite(value)||value<static_cast<long double>(std::numeric_limits<std::int64_t>::min())||
            value>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))
            throw std::runtime_error("Human animation __ftol exceeds original integer range");
        const auto bits=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
        std::int32_t result;std::memcpy(&result,&bits,4);return result;
    }
    static long double blend_ratio(float elapsed,float step,float duration) { //007724A0, keep its x87 evaluation order.
        const auto denominator=static_cast<long double>(duration)*.01f;
        const auto previous=(static_cast<long double>(elapsed)-step)/denominator;
        return ((static_cast<long double>(elapsed)/denominator-previous)*.5f+previous)*.01f;
    }
    Primary get_primary(unsigned base) const{return {{integer(base),integer(base+4),integer(base+8)}};}
    Secondary get_secondary(unsigned base) const{return {{integer(base),integer(base+4)}};}
    void set_primary(unsigned base,const Primary& value){for(unsigned n=0;n!=3;++n)words_.set(base+n*4,value[n]);}
    void set_secondary(unsigned base,const Secondary& value){for(unsigned n=0;n!=2;++n)words_.set(base+n*4,value[n]);}
    void normalize_primary(unsigned base) { //00782EB0, including byteC write.
        normalize_human_primary_animation(words_,base);
    }
    void reset_meta(unsigned slot) {
        const auto base=slot*0x24;
        words_.set(base+0xd0,1.f);words_.set(base+0xd4,1.f);words_.set(base+0xc4,-10000.f);
        words_.set(base+0xcc,std::uint32_t{0});words_.set(base+0xd8,std::uint8_t{1});
    }
    void finish_transition(unsigned group,unsigned slot,unsigned other) {
        const auto meta=slot*0x24;const bool fading=(word(meta+0xcc)&1u)!=0;
        if(!fading)frames_.human_model_weight(model_,slot,real(meta+0xbc));
        else {
            words_.set(group*0x2c+0xc,std::int32_t{-1});
            if(!group){words_.set(0x200,std::int32_t{206});normalize_primary(0x200);}
            else if(group==1)words_.set(0x210,std::int32_t{91});
            else if(group==2)words_.set(0x21c,std::int32_t{75});
        }
        if(word(other*0x24+0xc4)!=0xc61c4000u) {
            frames_.human_model_weight(model_,other,0.f);frames_.human_model_remove_animation(model_,other);
            frames_.human_model_animation(model_,Frame{},other,1.f,0);reset_meta(other);
        }
        if(fading){frames_.human_model_remove_animation(model_,slot);frames_.human_model_weight(model_,slot,0.f);reset_meta(slot);}
        if(!group){words_.set(0x1dc,std::int32_t{206});normalize_primary(0x1dc);}
        else if(group==1)words_.set(0x1ec,std::int32_t{91});
        else if(group==2)words_.set(0x1f8,std::int32_t{75});
        words_.set(0x228,1.f);
    }
    template<class Work>void attempt(Work&& work) noexcept {
        try{work();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    Frames& frames_;Callbacks& callbacks_;HumanJointWords<0x3a4> words_;
    Frame model_{},parent_marker_{},marker_{};bool started_=false;
};
} //namespace hd2::game
#endif
