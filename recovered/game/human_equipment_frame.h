#ifndef HD2_GAME_HUMAN_EQUIPMENT_FRAME_H
#define HD2_GAME_HUMAN_EQUIPMENT_FRAME_H

#include "game/human_equipment_action.h"

namespace hd2::game {
//00792CE0: actual230 state, attachment/event receivers and borrowed light.
//Elapsed is SIGNED for the two FLOAT rates, unlike Human005A0420's DWORD.
template<class Frames,class State,class Services,class Attachments,class Transfers,class Catalogue>
void advance_human_equipment_frame(Frames& frames,State& state,Services& services,Attachments& attachments,
        Transfers& transfers,typename Frames::Frame light,const Catalogue& catalogue,std::uint32_t elapsed){
    auto& words=state.words();std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
    float angle_step=static_cast<float>(static_cast<long double>(signed_elapsed)*.000699999975040555f);
    float shift_step=static_cast<float>(static_cast<long double>(signed_elapsed)*.0012000000569969416f);
    const auto expired=[&](unsigned offset){
        const auto previous=words.template get<std::uint32_t>(offset);if(previous==0xffffd8f0u)return false;
        const auto next=previous-elapsed;words.set(offset,next);std::int32_t value;std::memcpy(&value,&next,4);return value<=0;
    };
    if(expired(0xd4)){
        if(services.equipment_life_value()<1)words.set(0xd4,std::int32_t{-10000});
        else{
            attachments.attach(3);
            if(const auto* definition=catalogue.item(words.template get<std::int32_t>(0x34)))
                if(definition->kind()!=9&&definition->kind()!=2)transfers.event(0);
            if(services.equipment_look_command()==1)services.blend_equipment_weapon(300);
        }
    }
    if(const auto model=state.model(2)){
        const auto hiding=words.template get<std::uint32_t>(0xcc)==1;
        const auto shift=words.template get<float>(0xc0);
        if(hiding){
            const auto limit=words.template get<float>(0xc8);
            if(shift<limit){
                if(static_cast<long double>(shift)+shift_step>limit)shift_step=static_cast<float>(static_cast<long double>(limit)-shift);
                words.set(0xc0,static_cast<float>(static_cast<long double>(shift)+shift_step));
                frames.human_equipment_shift_z(model,-shift_step);
            }
        }else if(shift>0&&words.template get<float>(0xbc)<=0){
            if(static_cast<long double>(shift)-shift_step<0)shift_step=shift;
            words.set(0xc0,static_cast<float>(static_cast<long double>(shift)-shift_step));
            frames.human_equipment_shift_z(model,shift_step);
        }
        const auto angle=words.template get<float>(0xbc);
        bool rotate=false;float rotation=0;
        if(hiding){
            const auto limit=words.template get<float>(0xc4);
            if(angle<limit){
                if(static_cast<long double>(angle)+angle_step>limit)angle_step=static_cast<float>(static_cast<long double>(limit)-angle);
                words.set(0xbc,static_cast<float>(static_cast<long double>(angle)+angle_step));
                rotation=-shift_step;rotate=true; //00792FD3 uses SHIFT step at esp+C, not angle step at esp+8.
            }
        }else if(angle>0){
            if(static_cast<long double>(angle)-angle_step<0)angle_step=angle;
            words.set(0xbc,static_cast<float>(static_cast<long double>(angle)-angle_step));rotation=angle_step;rotate=true;
        }
        if(rotate){
            const auto rotation_matrix=frames.human_axis_rotation(HumanVector{{1,0,0,0}},rotation);
            const auto matrix=frames.car_local_matrix(model);
            frames.human_equipment_local_matrix(model,frames.rigid_matrix_product(matrix,rotation_matrix));
        }
    }
    if(expired(0x6c)){
        words.set(0x6c,std::int32_t{-10000});
        if(!light)throw std::runtime_error("Human equipment light expiry requires actual230+68 light");
        frames.human_equipment_remove_light_sectors(light);
    }
    if(expired(0x70))human_equipment_action_lens(frames,state,false); //Missing lens leaves the expired timer unchanged.
}
} //namespace hd2::game
#endif
