#ifndef HD2_GAME_FIRST_PERSON_FRAME_H
#define HD2_GAME_FIRST_PERSON_FRAME_H
#include <array>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
inline std::int32_t fpv_signed(std::uint32_t bits){std::int32_t v;std::memcpy(&v,&bits,4);return v;}
inline float fpv_real(std::uint32_t bits){float v;std::memcpy(&v,&bits,4);return v;}
template<class Overlay,class Models,class Frames,class Players,class Playback,class Receiver>
void advance_first_person_frame(std::uint32_t mode,std::uint32_t elapsed,Overlay& overlay,Models& models,Frames& frames,Players& players,Playback& playback,Receiver& receiver){ //00491E90, actual FPV frame wrapper.
        if(!(mode&2u)||!overlay.byte3c||overlay.bytes58[1]||!models.model()||!receiver.player())return;
        if(players.health(*receiver.player())<1){
            if(frames.hud_frame_visible(models.model())){frames.object_visible(models.model(),false);frames.update_scene_object(models.model());}
            return;
        }
        if(overlay.bytes58[0]&&!frames.hud_frame_visible(models.model()))frames.object_visible(models.model(),true);
        receiver.advance_motion(mode,elapsed);receiver.complete_animation(mode,elapsed);
        playback.advance(mode,fpv_signed(elapsed));receiver.update_opacity(mode);
        const auto timer=models.flash_time();
        if(timer>=0.f){
            if(timer==0.f){
                models.flash_time(-1.f);
                if(!models.flash())throw std::logic_error("FPV flash timer has no actual flash borrower");
                frames.object_visible(models.flash(),false);frames.update_scene_object(models.flash());
            }
            const auto next=static_cast<long double>(models.flash_time())-fpv_signed(elapsed);
            models.flash_time(next<0?0.f:static_cast<float>(next));
        }
        if(models.model()&&models.playback().current.clip){
            frames.first_person_model_advance(models.model(),fpv_signed(elapsed));frames.update_scene_object(models.model());
        }
    }
template<class Frames,class Overlay,class Models,class Items,class Players,class Camera,class Playback,class Receiver>
void advance_first_person_motion(std::uint32_t mode,std::uint32_t elapsed,Frames& frames,Overlay& overlay,Models& models,Items& items,Players& players,Camera& camera,Playback& playback,Receiver& receiver,
        const std::uint32_t& state,float& phase_time,float& bob,float& lowering_rate,const float& target_fov,const float& target_parameter){
    using Frame=typename Frames::Frame; //00491660.
        if(!(mode&2u)||!overlay.byte3c)return;
        auto* human=receiver.player();if(!human||!models.model())throw std::logic_error("FPV motion requires its live Human/model");
        const auto& velocity=players.velocity(*human);
        const auto square=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2];
        const auto stored=static_cast<float>(square);std::int32_t square_bits;std::memcpy(&square_bits,&stored,4);
        const long double speed=square_bits<0x322bcc77?0.L:std::abs(static_cast<long double>(stored)-1.f)<fpv_real(0x322bcc77)?1.L:std::sqrt(static_cast<long double>(stored));
        const auto dt=fpv_signed(elapsed);const float delta=static_cast<float>(dt);
        const auto phase=static_cast<long double>(dt)*fpv_real(0x3b03126f)+phase_time;phase_time=static_cast<float>(phase);
        const auto wave=static_cast<float>(std::abs(std::sin(static_cast<long double>(fpv_real(0x40490fdb))*phase)));
        const auto integral=std::trunc(phase);
        if(!std::isfinite(integral)||integral<-9223372036854775808.L||integral>=9223372036854775808.L)throw std::runtime_error("FPV phase exceeds native FTOL range");
        const auto parity=static_cast<std::int64_t>(integral);std::array<float,4> sway{};
        if(speed!=0){
            const auto amount=speed*wave*(overlay.bytes58[2]?fpv_real(0x3a03126f):fpv_real(0x3a83126f));
            sway[0]=static_cast<float>((parity&1)?amount:-amount);sway[1]=static_cast<float>(-amount);
        }
        sway=frames.collision_sweep_rotate(sway,frames.actor_world_matrix(models.model()));
        std::array<float,4> position,direction;
        if(!camera.query_vector(0,position))throw std::runtime_error("FPV motion requires actual active camera position");
        for(unsigned i=0;i<3;++i)position[i]=static_cast<float>(static_cast<long double>(position[i])+sway[i]);
        frames.car_camera_position(models.model(),position);
        if(!camera.query_direction(0,direction))throw std::runtime_error("FPV motion requires actual active camera direction");
        const auto scaled=[&](unsigned factor){return static_cast<long double>(fpv_signed(elapsed*factor))*.001f;};
        if(state==3){
            if(bob!=-.8f){
                const auto next=static_cast<long double>(bob)-scaled(4);bob=static_cast<float>(next);
                if(next<=-.8f){
                    bob=-.8f;
                    if(const auto entry=items.kind9_holster(*human,players.time())){
                        receiver.set_state(mode,4);models.reset_tracks(mode);
                        const auto model=items.model(entry);const auto id=items.definition(entry).identifier;
                        models.bind_weapon(mode,model,Frame{},static_cast<std::int32_t>(id));(void)playback.play(mode,2);bob=0;
                    }
                }
            }
        }else if(state==5){
            if(players.state(*human).first_person_state()==1){
                if(bob!=-.2f){const auto next=static_cast<long double>(bob)+scaled(3);bob=static_cast<float>(next);
                    if(next<=-.2f){bob=-.2f;receiver.set_state(mode,players.state(*human).first_person_state());}}
            }else if(bob!=0){const auto next=static_cast<long double>(bob)+scaled(3);bob=static_cast<float>(next);
                if(next>=0){bob=0;receiver.set_state(mode,players.state(*human).first_person_state());}}
        }else if(state==1){
            if(bob!=-.2f){const auto rate=static_cast<long double>(bob)*3.f+1.f;lowering_rate=static_cast<float>(rate);
                const auto next=static_cast<long double>(bob)-rate*delta*.001f;bob=static_cast<float>(next);if(next<=-.2f)bob=-.2f;}
        }else if(state==2&&bob!=0){const auto next=static_cast<long double>(bob)+scaled(4);bob=static_cast<float>(next);if(next>=0)bob=0;}
        if(bob!=0){
            direction[1]=static_cast<float>(static_cast<long double>(direction[1])+bob);
            const auto length=(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0])+static_cast<long double>(direction[1])*direction[1];
            const auto stored_length=static_cast<float>(length);
            if(!(std::abs(length-1.f)<fpv_real(0x322bcc77))){
                if(!(stored_length<fpv_real(0x322bcc77))){const auto scale=1.L/std::sqrt(static_cast<long double>(stored_length));for(unsigned n=0;n<3;++n)direction[n]=static_cast<float>(direction[n]*scale);}
                else {const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
            }
        }
        frames.car_camera_direction(models.model(),direction,0.f);
        const auto interpolate=[&](float current,float target){
            const auto step=static_cast<long double>(delta)*fpv_real(0x3b4de32f);
            const auto next=current>target?static_cast<long double>(current)-step:static_cast<long double>(current)+step;
            return static_cast<float>(current>target?(next<target?target:next):(next>target?target:next));
        };
        const auto current_fov=camera.fov();if(current_fov!=target_fov)(void)camera.set_fov(interpolate(current_fov,target_fov));
        const auto current_parameter=camera.parameter78();if(current_parameter!=target_parameter)(void)camera.set_parameter78(interpolate(current_parameter,target_parameter));
    }
}
#endif
