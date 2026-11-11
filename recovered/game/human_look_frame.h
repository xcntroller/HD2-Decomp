#ifndef HD2_GAME_HUMAN_LOOK_FRAME_H
#define HD2_GAME_HUMAN_LOOK_FRAME_H

#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//0077DE30's nonoffline, nonplayer action1 hand-equipment reconstruction.
//005A2AC0 rereads the actual holder for each native call. Model selection and
//transfer events use the SAME230 owner as activation and inventory changes.
template<class Human,class Look,class World>
void prepare_human_remote_look_equipment(Human& human,Look& look,World& world){
    auto& items=world.human_reload_items();
    using Item=std::remove_reference_t<decltype(items.runtime_item(human.base().storage()->holder(3)->entry()))>;
    const auto hand=[&]()->Item*{
        auto* storage=human.base().storage();auto* holder=storage?storage->holder(3):nullptr;
        if(!holder)throw std::runtime_error("Remote Human look requires the actual hand holder");
        const auto entry=holder->entry();return entry?&items.runtime_item(entry):nullptr;
    };
    const auto required_hand=[&]()->Item&{
        auto* item=hand();if(!item)throw std::runtime_error("Remote Human look lost its hand item");
        return *item;
    };
    auto& equipment=world.human_presentation_equipment(human);
    if(!equipment.state().selected_model(3)&&hand()){
        auto& item=required_hand();
        const auto identity=item.identity;
        const auto definition=item.definition().identifier;
        const auto model=item.model();
        equipment.state().select(model,3,static_cast<std::uint32_t>(identity),-10000,static_cast<std::int32_t>(definition));
        equipment.transfers().event(3);
    }
    if(!look.state().template get<std::uint32_t>(0x54)&&hand()&&required_hand().definition().item_class==1)
        look.frame_words().set(0x54,required_hand().definition().equipment_command());
    const auto command=look.state().template get<std::uint32_t>(0x54);
    if(command!=2&&command!=6)equipment.transfers().event(0);
}

//0077DE30's equipment/action selection. Native previous values are the SAME
//DD0+770 snapshot. Mounted-object retirement and reload-family lookup retain
//their required actual receivers.
template<class Control,class Look,class Human,class World>
void apply_human_look_equipment(Control& control,Look& look,Human& human,World& world,bool walking){
    auto& request=look.frame_words();auto& state=control.physical().control_frame_words();
    const auto wanted=[&](unsigned offset){return request.template get<std::int32_t>(offset);};
    const auto current=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto primary=[&](std::int32_t id){state.set(0x1fc,id);normalize_human_primary_animation(state,0x1fc);};
    const auto mounted=[&]{
        auto& equipment=world.human_presentation_equipment(human);
        control.retire_look_mount(look,equipment.state().selected_model(9)); //0075E290, actual EC-bound Human.
    };
    const auto can_throw=[&]{return wanted(0x4c)!=2||current(0x1d8)<0xbe;};
    auto equipment=wanted(0x54);
    if(!equipment&&!current(0x7c4)){
        switch(wanted(0x58)){
        case 9:{
            const auto phase=state.template get<std::uint32_t>(0xa2c);
            if(phase==0xffffd8f0u)primary(0x9c);
            else if((phase&0xfffffff7u)==1u)primary(phase&8u?0xa1:0x9d);
            request.set(0x58,0u);break;
        }
        case 10:
            if(current(0xa30)==-10000)primary(0x5e);
            else if(current(0xa30)==1){
                if(control.movement().frame_words().template get<float>(0x114)==-10000.099609375f&&
                        !(current(0x1d8)>=0xa2&&current(0x1d8)<=0xa5))primary(0x5f);
                else mounted();
            }
            request.set(0x58,0u);break;
        case 12:
            if(current(0xa2c)==-10000)primary(0xa0);
            else if((state.template get<std::uint32_t>(0xa2c)&0xfffffff7u)==1u)primary(0xa1);
            request.set(0x58,0u);break;
        case 13:if(can_throw())state.set(0x20c,0x4fu);request.set(0x58,0u);break;
        case 14:if(can_throw())state.set(0x20c,0x5au);request.set(0x58,0u);break;
        default:break;
        }
    }else{
        if(current(0x7c8)==wanted(0x58))request.set(0x58,0u);
        std::int32_t animation=0x5b;bool force=false;
        const auto promote_primary=[&]{if(wanted(0x44)==0xce||wanted(0x44)==0x2d)request.set(0x44,0x36u);};
        if(!wanted(0x58)||(equipment!=current(0x7c4)&&!current(0x7c4)&&equipment)){
            if(equipment==100&&current(0xa30)==-10000){animation=0x5c;request.set(0x54,0u);force=true;}
            else if(current(0xa2c)==-10000&&current(0xa30)==-10000&&
                    !(world.human_presentation_equipment(human).state().words().template get<std::uint32_t>(0x50)&1u)){
                const auto previous=current(0x7c4);
                if(equipment!=previous&&(!equipment||!previous)){
                    if(equipment)animation=0x23;
                    else{animation=0x22;state.set(0x220,previous);}
                    force=true;
                }else if(equipment!=6&&equipment!=100&&current(0x1e8)!=0x27&&!(wanted(0x4c)==2&&!walking)){
                    animation=wanted(0x5c)?0x20:(human.properties.field19c!=1?0x1f:0x1e);force=true;
                }
            }else if(current(0xa2c)==1){animation=0x57;request.set(0x54,current(0x7c4));force=true;}
            else if(current(0xa30)==1){animation=0x58;request.set(0x54,current(0x7c4));force=true;}
            else if(state.template get<std::uint32_t>(0xa2c)&8u){animation=0x5c;request.set(0x54,current(0x7c4));force=true;}
        }else{
            switch(wanted(0x58)){
            case 1:animation=0x27;force=true;break;
            case 2:state.set(0x218,0x4au);animation=0x27;force=true;break;
            case 3:
                if(wanted(0x4c)!=2||walking){
                    const auto family=world.human_reload_animation_family(human); //00791C10, same hand holder and lazy item model.
                    animation=family==2||(family!=3&&family!=4)?0x50:0x51;
                    promote_primary();force=true;
                }
                break;
            case 5:animation=0x25;force=true;break;
            case 6:case 7:
                if(can_throw()){animation=wanted(0x58)==6?0x4c:0x4d;promote_primary();force=true;}break;
            case 8:animation=0x46;promote_primary();force=true;break;
            case 10:
                if(current(0xa30)==-10000)primary(0x5e);
                else if(current(0xa30)==1)primary(0x5f);
                request.set(0x58,0u);break;
            case 11:animation=0x47;promote_primary();force=true;break;
            case 13:case 14:
                if(can_throw()){animation=wanted(0x58)==13?0x4f:0x5a;promote_primary();force=true;}break;
            default:break;
            }
        }
        if(force||current(0x20c)!=0x5c)state.set(0x20c,animation);
    }
    if((wanted(0x58)<0||wanted(0x58)>2)&&current(0x1f4)==0x4a)state.set(0x218,0x4cu);
}

//0077DE30, actual E0 command application, normalization and retained snapshot.
//Positive posture, interaction, lateral collision and occupied remote weapon
//workers remain required. No command mask substitutes for their real bodies.
template<class Control,class Look,class Human,class World,class Runtime>
void apply_human_look_frame(Control& control,Look& look,Human& human,World& world,Runtime& runtime,std::uint32_t elapsed){
    auto& request=look.frame_words();auto& state=control.physical().control_frame_words();
    const auto wanted=[&](unsigned offset){return request.template get<std::int32_t>(offset);};
    const auto current=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto walk=[](std::int32_t id){return id>=0x29&&id<=0x41;};
    const auto ladder=[](std::int32_t id){return id>=8&&id<=14;};
    const auto interaction=[](std::int32_t id){return id>=0x71&&id<=0x9a;};
    const auto normalize=[&]{normalize_human_primary_animation(state,0x1fc);};
    const auto cancel_ladder=[&]{request.set(0x44,0xceu);state.set(0x1fc,0xceu);normalize();};
    const auto finish=[&]{
        if(wanted(0xb8)==4)look.mark_frame_feedback(0x40u);
        control.physical().capture_look_frame(look);
    };
    const bool walking=walk(current(0x1d8));
    request.set(0xb4,0u);
    if(control.human_is_player())control.prepare_direction_replay(look,request.template get<HumanVector>(0x20),elapsed);
    if(!interaction(wanted(0x44))){
        if(current(0x1ac)!=1||interaction(current(0x1d8))){finish();return;}
    }else if(control.retire_interaction_clip(look)) { //0074D510, SAME AA0 action owner and queued descriptor.
        request.set(0x44,current(0x1fc));finish();return;
    }
    const auto sensitive=[](std::int32_t id){return id==0x2b||id==0x2c||id==0x36||id==0x3e||id==0x41;};
    const auto desired=wanted(0x44),active=current(0x1d8);
    if(desired!=0xce&&(!walk(desired)||!walking||(desired!=active&&(sensitive(desired)||sensitive(active))))&&
            ((world.mode_flags()&7u)==7u||current(0x1fc)>3)){
        state.set(0x1fc,desired);normalize();
    }
    if(wanted(0x48)!=0x4b)state.set(0x218,wanted(0x48));
    if(current(0x1fc)==0x12){
        if(!state.template get<std::uint8_t>(0x762)||current(0x1d8)==0x18||current(0xa30)!=-10000||wanted(0x4c)==1){state.set(0x1fc,0xceu);normalize();}
        else if(!walking){state.set(0x1fc,0x18u);normalize();}
    }
    const auto queued=current(0x1fc);
    if(queued==0x6a){
        if(((world.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor())&&current(0x988)==-10000){
            if(!ladder(current(0x1d8))){
                state.set(0x1fc,10u);normalize();
                if(wanted(0xb8)==5||!control.prepare_step_motion(look,runtime))cancel_ladder();
            }else cancel_ladder();
        }
    }else if(ladder(queued)){
        if(wanted(0xb8)==5)cancel_ladder();
        else if(queued!=8&&queued!=14&&!control.prepare_step_motion(look,runtime))cancel_ladder();
    }else if((queued==0x22||queued==0x23)&&wanted(0xb8)!=2&&wanted(0xb8)!=3){
        const auto clip=current(0x1d8);
        if(clip!=0x9f&&clip!=0x9e&&clip!=0x9b&&clip!=0x6f&&clip!=0x22){state.set(0x1fc,queued==0x22?0x1cu:0x24u);normalize();} //0074E820.
    }
    if(wanted(0x50)!=current(0x7c0)&&wanted(0x44)==0xce)request.set(0x44,current(0x1d8));
    if(wanted(0x4c)!=current(0x7bc)){
        if(wanted(0x44)==0xce){request.set(0x44,0x2du);state.set(0x1fc,0x2du);normalize();}
        bool defer=false;
        if((world.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor()&&
                (ladder(current(0x1fc))||ladder(current(0x1d8))))
            defer=control.tracks().normalized_clip_position(0)<=.4000000059604645f&&control.tracks().normalized_clip_position(0)>=0;
        if(!defer)control.apply_look_posture(look); //0076AF70, actual room/geometry/transition state.
    }
    if(!(world.mode_flags()&1u)&&!control.human_is_player()&&wanted(0x58)==1){
        request.set(0x5c,1u);
        prepare_human_remote_look_equipment(human,look,world);
    }
    apply_human_look_equipment(control,look,human,world,walking);
    if(wanted(0xb8)!=5&&(current(0xa34)==0||current(0xa34)==1)){
        const auto flags=request.template get<std::uint32_t>(0x60);
        if(flags&0x3fu){
            std::int32_t clip=0x4b;
            const bool first=flags&0x40u,second=flags&0x80u;
            switch(flags&0x3fu){
            case 1:if(first||second)clip=first?0x1e:0x1f;break;
            case 2:if(first||second)clip=first?0x20:0x22;break;
            case 4:if(first||second)clip=first?0x24:0x26;break;
            case 8:
                if(flags&0x100u)clip=0x32;
                else if(flags&0x200u)clip=0x33;
                else if(first||second)clip=first?0x30:0x31;
                break;
            case 16:if(first||second)clip=first?0x28:0x2a;break;
            case 32:if(first||second)clip=first?0x2c:0x2e;break;
            default:break;
            }
            control.face().control_command(2,0); //0075CD00→007812B0.
            request.set(0x60,0u);
            if((wanted(0x48)==0x4b||wanted(0x48)==0x4c||wanted(0x48)==current(0x1f4))&&clip!=0x4b&&current(0x9ac)==-10000)
                state.set(0x218,clip);
        }
    }
    state.set(0x200,wanted(0x4c));normalize();
    state.set(0x210,wanted(0x4c));state.set(0x21c,wanted(0x4c));state.set(0x204,wanted(0x50));
    if(current(0x1fc)>=0xa2&&current(0x1fc)<=0xa5)control.refine_look_lateral_stride(look); //0074A290, original ID gate.
    if(wanted(0x90)){
        control.apply_look_interaction(look,look.interaction_actor(),wanted(0x90),request.template get<std::uint32_t>(0x94),runtime); //00751480.
        look.complete_frame_interaction();
    }
    finish();
}

} //namespace hd2::game
#endif
