#ifndef HD2_GAME_HUD_COMMAND_INPUT_H
#define HD2_GAME_HUD_COMMAND_INPUT_H
#include "game/hud_command_frame.h"
#include "game/text_glyphs.h"
#include <cmath>
namespace hd2::game {
using HudCommandVector=std::array<float,4>;
struct HudCommandInputState { //SAME command owner; no order/model pointer is retained by mouse gestures.
    HudCommandVector anchor5a790{},last5a7a0{};bool order_drag5b0=false,middle5ba=false,captured5bc=false,spare5bd=false;
};
inline HudCommandVector command_normalize(HudCommandVector v,long double square,long double x){
    const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-9f;
    //TEST AH,05's parity branches admit unordered values into FSQRT.
    if(!(std::abs(square-1.f)<epsilon)){if(!(stored<epsilon)){const auto scale=1/std::sqrt(static_cast<long double>(stored));v[0]=static_cast<float>(x*scale);for(unsigned n=1;n!=3;++n)v[n]=static_cast<float>(v[n]*scale);}
        else {const unsigned n=x!=0?0u:v[2]!=0?2u:1u;const auto value=n==0?x:static_cast<long double>(v[n]);v[n]=value>=0?1.f:-1.f;}}
    return v;
}
inline HudCommandVector command_normalize_forward(HudCommandVector v){ //004B890E/004B8B5E: (Y²+Z²)+X², all original FLOAT lanes.
    const auto x=static_cast<long double>(v[0])*v[0],y=static_cast<long double>(v[1])*v[1],z=static_cast<long double>(v[2])*v[2];return command_normalize(v,(y+z)+x,v[0]);
}
inline HudCommandVector command_normalize_direction(const HudCommandVector& to,const HudCommandVector& from,bool ground){
    const auto x=static_cast<long double>(to[0])-from[0],y=static_cast<long double>(to[1])-from[1],z=static_cast<long double>(to[2])-from[2];
    HudCommandVector v{{static_cast<float>(x),static_cast<float>(y),static_cast<float>(z),0}};
    //004BAFAA retains Z while multiplying its stored FLOAT;004BAB48 retains
    //X through both its square and final scaling. Other deltas have been stored.
    const auto square=ground?(z*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0]:(x*x+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
    return command_normalize(v,square,ground?static_cast<long double>(v[0]):x);
}
//004B79C0 and reached UI policies. Queries use real World/Players/engine
//owners or REQUIRED Human-order/Player/camera producers. This is not simulation.
template<class Command,class Pointer,class Input,class Queries>class HudCommandInput {
public:
    HudCommandInput(Command& command,Pointer& pointer,Input& input,Queries& queries)
        :c(command),p(pointer),i(input),q(queries),s(c.input_state()),f(c.frame_state()),v(c.presentation()){}
    void run(){
        if(!c.input_bound()||!c.visible())return;(void)p.pointer_position();
        if(!c.nested_roll_visible()){
            constexpr std::array<unsigned,18> actions{{0,1,2,3,4,5,6,7,8,9,10,11,12,13,15,16,17,31}};
            constexpr std::array<unsigned,18> tags{{1,4,5,6,7,8,9,10,11,12,13,14,15,2,3,16,17,18}};
            for(unsigned n=0;n!=actions.size();++n)if(i.second(actions[n])!=0){activate_tag(tags[n]);break;}
            if(i.second(55)!=0)left_press();else if(i.first(55)!=0)left_hold();else if(f.point_gate5a7b8)left_release();
            if(i.second(56)!=0)right_press();else if(i.first(56)!=0){(void)i.first(49);}else if(f.point_gate5a7b9)right_release(); //004BB200 genuine RET4, boolean argument still evaluated.
            if(i.second(57)!=0)s.middle5ba=true;else if(i.first(57)!=0){}else if(s.middle5ba)s.middle5ba=false; //004BB230/240/250.
            if(!c.nested_roll_visible())pointer_kind();camera_input();return;
        }
        q.roll_controls(i);const auto scan=i.next_scan(),character=i.scan_character(scan)&0xffu;bool chosen=false;
        if(character>='0'&&character<='9'){chosen=q.roll_choose(character=='0'?9u:character-'1');i.consume_scan(scan);}
        if(!chosen&&i.second(55)!=0)chosen=q.roll_row_valid();
        if(chosen){const auto row=q.roll_row();if(row==-1)return;
            if(row<9){auto* target=q.roll_target();const auto action=q.roll_action(static_cast<unsigned>(row));
                if(target&&action==6){const auto kind=q.kind(*target);if(kind==4||kind==20||kind==21)if(auto* occupant=q.occupant(*target,row))target=occupant;}
                if(v.previous14)submit_action(action,target,static_cast<unsigned>(row));}
            q.show_nested_roll(false,nullptr);
        }
        camera_input();if(!q.roll_row_valid()&&(i.second(55)!=0||i.second(56)!=0))q.show_nested_roll(false,nullptr);
    }
    void activate_tag(unsigned tag){ //004BA2D0.
        if(hud_command_signed(f.feedback740)>=1)return;
        if(tag>=6&&tag<=15){if(!c.button_visible(tag))policy_button(tag);return;}
        if(tag>=16&&tag<=18){c.activate_misc_button(tag,q);return;}
        if(tag>=1&&tag<=5){order_button(tag);return;}
        if(tag>=20&&tag<30){q.select_order_index(static_cast<std::int32_t>(tag-20));c.update_policies(q);}
    }
    void pointer_kind(){ //004B7E50, first snapshot held controls and second wheel, native priority chain.
        if(v.project_pointer)return;
        if(f.cooldown58){if(i.second(54)>0||i.second(54)<0)f.cooldown58=100;return;}
        if(i.first(62)!=0){p.select_pointer(14);return;}if(i.first(63)!=0){p.select_pointer(15);return;}
        if(i.first(18)!=0||i.first(32)!=0||i.first(34)!=0||i.first(33)!=0){unsigned kind=1;
            if(i.first(50)!=0)kind=i.first(18)!=0?5:i.first(32)!=0?6:i.first(34)!=0?4:3;p.select_pointer(kind);return;}
        if(i.first(39)!=0){p.select_pointer(11);return;}if(i.first(40)!=0){p.select_pointer(12);return;}
        if(i.second(54)>0){f.cooldown58=100;p.select_pointer(11);return;}if(i.second(54)<0){f.cooldown58=100;p.select_pointer(12);return;}
        if(i.first(57)!=0){p.select_pointer(2);return;}
        for(const auto pair:{std::pair<unsigned,int>{42,3},{43,4},{44,5},{45,6}})if(i.first(pair.first)!=0){p.select_pointer(pair.second);return;}
        const auto position=p.pointer_position();p.select_pointer(position[0]<.009999999776482582f?7:.9900000095367432f<position[0]?8:position[1]>-.009999999776482582f?9:position[1]<-.7400000095367432f?10:0);
    }
    void camera_input(){ //004B81A0; actual detached owner receives mode then two axes.
        unsigned mode=0;float x=0,y=0;const auto kind=p.pointer_kind();
        switch(kind){
        case 0:break;
        case 1:if(!v.project_pointer){x=sub(i.first(33),i.first(34));y=sub(i.first(18),i.first(32));mode=1;}
            else{p.restrict_pointer_delta(i);x=i.first(52);y=i.first(53);q.camera_mode(3,&v.position20);q.camera_axes(x,y);return;}break;
        case 2:q.camera_mode(2,nullptr);if(q.camera_inverted())i.first(53,-i.first(53));x=static_cast<float>(static_cast<long double>(i.first(52))*40.f);y=static_cast<float>(static_cast<long double>(i.first(53))*40.f);q.camera_axes(x,y);return;
        case 3:mode=2;x=-1;break;case 4:mode=2;x=1;break;case 5:mode=2;y=-1;break;case 6:mode=2;y=1;break;
        case 7:mode=1;x=1;break;case 8:mode=1;x=-1;break;case 9:mode=1;y=1;break;case 10:mode=1;y=-1;break;
        case 11:mode=8;y=1;break;case 12:mode=9;y=-1;break;
        case 13:mode=1;x=sub(i.first(2),i.first(3));y=sub(i.first(0),i.first(1));break;
        case 14:mode=11;y=1;break;case 15:mode=12;y=-1;break;default:return;
        }
        q.camera_mode(mode,nullptr);q.camera_axes(x,y);
    }
    void left_release(){ //004BAD50.
        s.order_drag5b0=v.project_pointer=s.captured5bc=f.point_gate5a7b8=s.spare5bd=false;p.clear_pointer_clip();p.select_pointer(0);
    }
    void right_release(){s.captured5bc=f.point_gate5a7b9=s.spare5bd=false;p.select_pointer(0);} //004BB210.
private:
    static float sub(float a,float b){return static_cast<float>(static_cast<long double>(a)-b);}
    static HudCommandVector delta(const HudCommandVector& a,const HudCommandVector& b){auto r=a;for(unsigned n=0;n!=3;++n)r[n]=sub(a[n],b[n]);return r;}
    auto ray(unsigned mask,bool normal){const auto screen=p.pointer_screen_position();auto value=q.screen_ray(screen[0],screen[1]);for(unsigned n=0;n!=3;++n)value.second[n]=static_cast<float>(static_cast<long double>(value.second[n])*100.f);return q.ray(value.first,value.second,0,mask,normal);}
    void create_order(unsigned kind,const HudCommandVector& position,const HudCommandVector& direction){ //004B8E30, insertion index is reread after first admission.
        auto* receiver=q.previous_human();if(!receiver)throw std::logic_error("Command order creation requires actual previous Human");
        std::int32_t index=-1;if(kind!=4&&q.selected_order_index()>=0)index=q.selected_order_index();q.create_order(*receiver,kind,-1,position,direction,c.policies(),nullptr,0,index);
    }
    void submit_action(unsigned kind,typename Queries::Actor* target,unsigned row){auto* receiver=q.previous_human();if(!receiver)throw std::logic_error("Command action requires actual previous Human");q.create_order(*receiver,3,static_cast<std::int32_t>(kind),{}, {},c.policies(),target,row,-1);} //004B8FF0.
    void change_policy(){const auto index=q.selected_order_index();if(index!=-1)q.change_order(index,{}, {},c.policies());} //004B8EE0.
    void policy_button(unsigned tag){ //004B8470: only the changed group is touched, in its original order.
        auto& values=c.policies();unsigned first,last,selected=tag;
        if(tag<=8){values[2]=tag==6?4u:tag==7?2u:1u;first=6;last=8;}
        else if(tag<=12){values[0]=tag==9?0x10000u:tag==10?0x4000u:tag==11?0x2000u:0x1000u;first=9;last=12;}
        else{values[1]=tag==13?0x400u:tag==14?0x200u:0x100u;first=13;last=15;}
        for(auto n=first;n<=last;++n)c.input_button(n,n==selected,n==selected);change_policy();
    }
    void order_button(unsigned tag){ //004B87B0, flags/feedback publish only after required producers return.
        if(!c.button_visible(tag)||!q.previous_index_valid())return;
        if(tag==2){c.input_button(tag,false,true);const auto index=q.selected_order_index();if(index>=0)q.remove_order(index);}
        else if(tag==3){c.input_button(tag,false,true);q.clear_orders();}
        else{auto* human=q.previous_human();if(!human)throw std::logic_error("Command button requires actual previous Human");HudCommandVector position{},direction{};
            if(q.order_count(*human)==0){position=q.actor_position(*human);if(tag!=1)direction=command_normalize_forward(q.actor_forward(*human));}
            else{const auto count=q.order_count(*human);if(count<=0)throw std::logic_error("Command last order retired during input");const auto order=q.order(*human,count-1);position=q.order_endpoint(order);if(tag!=1)direction=order.direction;}
            create_order(tag==1?4u:tag==4?6u:7u,position,direction);c.input_button(tag,false,true);
        }
        f.feedback_tag=tag;f.feedback740=200;
    }
    void left_press(){ //004BA390.
        v.project_pointer=s.captured5bc=s.spare5bd=false;f.point_gate5a7b8=true;const auto tag=p.pointer_hit_tag();if(tag){activate_tag(tag);return;}
        if(!p.pointer_in_command_area(p.pointer_position()))return;if(q.selected_order()){q.select_order_index(-1);c.update_policies(q);}
        const auto hit=ray(0,false);if(hit&&hit->order_model){const auto order=q.pick_order(hit->order_model);s.last5a7a0=q.order_model_position(order);s.anchor5a790=hit->position;s.order_drag5b0=true;q.select_human(*order.human);q.select_order_token(order);c.update_policies(q);return;}
        if(hit&&v.target1c&&v.target1c!=v.previous14){auto* target=q.resolve(v.target1c);if(q.kind(*target)==3){if(q.health(*target)>0&&q.in_roster(*target)){q.select_human(*target);return;}}
            else if(q.kind(*target)==4||q.kind(*target)==20||q.kind(*target)==21){for(std::int32_t n=0;n<q.occupant_count(*target);++n)if(auto* human=q.occupant(*target,n)){if(q.health(*human)>0&&q.in_roster(*human)){q.select_human(*human);return;}break;}}}
        v.project_pointer=true;p.clip_pointer_to(c.border());p.select_pointer(1);
    }
    void left_hold(){ //004BA660, only selected-order drag; navigation and Human update remain actual required owners.
        if(!s.order_drag5b0||!q.selected_order())return;const auto offset=delta(s.anchor5a790,s.last5a7a0);const auto first=q.project_world(s.anchor5a790),last=q.project_world(s.last5a7a0);
        const auto dx=text_ftol(static_cast<long double>(first[0])-last[0]),dy=text_ftol(static_cast<long double>(first[1])-last[1]);const auto point=p.pointer_position();
        const float width=static_cast<float>(q.display_width());const auto height=q.display_height();
        const auto x=text_sub(text_ftol(static_cast<long double>(point[0])*width),dx),y=text_sub(text_ftol(static_cast<long double>(point[1])*-1.3333333333333333*height),dy);
        auto screen=q.screen_ray(x,y);for(unsigned n=0;n!=3;++n)screen.second[n]=static_cast<float>(static_cast<long double>(screen.second[n])*100.f);
        const auto hit=q.ray(screen.first,screen.second,0xffffffffu,0,true);if(!hit||std::abs(static_cast<long double>(hit->normal[1]))<.6000000238418579)return;
        s.last5a7a0=hit->position;for(unsigned n=0;n!=3;++n)s.anchor5a790[n]=static_cast<float>(static_cast<long double>(offset[n])+s.last5a7a0[n]);
        const auto order=q.selected_order();if(!order)throw std::logic_error("Command order retired during drag query");const auto endpoint=q.order_endpoint(*order);
        //The original retains these three differences in x87 registers through
        //the distance test; only the later direction path stores FLOAT deltas.
        const auto dxp=static_cast<long double>(endpoint[0])-hit->position[0],dyp=static_cast<long double>(endpoint[1])-hit->position[1],dzp=static_cast<long double>(endpoint[2])-hit->position[2];
        const auto square=(dxp*dxp+dyp*dyp)+dzp*dzp;if(square<.05000000074505806f)return;
        const auto index=q.selected_order_index();if(order->kind==2||order->kind==6||order->kind==7){const auto direction=command_normalize_direction(hit->position,endpoint,false);q.change_order(index,{},direction,{});}
        else{q.change_order(index,hit->position,{},{});auto* human=q.previous_human();if(!human)throw std::logic_error("Command drag requires previous Human");
            if(q.selected_order_index()<q.order_count(*human)-1){const auto next=q.selected_order_index()+1;const auto following=q.order(*human,next);if(following.kind==2||following.kind==6||following.kind==7)q.change_order(next,hit->position,{},{});}}
    }
    void right_press(){ //004BAD80 and004BA360.
        f.point_gate5a7b9=true;const auto tag=p.pointer_hit_tag();if(tag){if(tag>=20&&tag<30&&v.previous14){q.remove_order(static_cast<std::int32_t>(tag-20));c.update_policies(q);}return;}
        if(!p.pointer_in_command_area(p.pointer_position()))return;const auto hit=ray(0x610000u,true);if(!hit)return;
        if(!hit->dynamic){if(!(.6000000238418579f<hit->normal[1])||!q.previous_index_valid())return;
            if(last_is_regroup())return;auto* human=q.previous_human();if(!human)throw std::logic_error("Command ground order requires previous Human");const auto direction=command_normalize_direction(hit->position,q.actor_base_position(*human),true);create_order(1,hit->position,direction);}
        else if(hit->order_model){const auto order=q.pick_order(hit->order_model);q.select_human(*order.human);q.select_order_token(order);const auto index=q.selected_order_index();if(index>=0)q.remove_order(index);c.update_policies(q);}
        else if(auto* actor=hit->actor){if(q.kind(*actor)==3&&q.human_hidden(*actor))return;if(last_is_regroup())return;q.show_nested_roll(true,actor);}
    }
    bool last_is_regroup(){if(!v.previous14)return false;auto* actor=q.resolve(v.previous14);if(q.order_count(*actor)==0)return false;const auto count=q.order_count(*actor);if(count<=0)throw std::logic_error("Command last-order count changed");return q.order(*actor,count-1).kind==4;}
    Command& c;Pointer& p;Input& i;Queries& q;HudCommandInputState& s;HudCommandFrameState& f;HudCommandPresentation& v;
};
} //namespace hd2::game
#endif
