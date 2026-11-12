#ifndef HD2_GAME_HUD_INVENTORY_LOADING_H
#define HD2_GAME_HUD_INVENTORY_LOADING_H
#include "game/hud_material_lookup.h"
#include "game/hud_scroll_loading.h"
#include "game/hud_text_loading.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "game/human_hud_loading.h"
#include <functional>
namespace hd2::game {
//004A0EE0's inlined S_vector normalization. Extended sum is tested before its
//stored float is used for reciprocal sqrt. Tiny vectors replace ONE component;
//unordered values follow native arithmetic instead of invented finite defaults.
inline std::array<float,4> hud_inventory_direction(std::array<float,4> value) {
    constexpr float epsilon=9.99999993922529029077850282192230224609375e-9f;
    const auto sum=static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[0])*value[0];
    const auto stored=static_cast<float>(sum);
    if(!(std::abs(sum-1.f)<epsilon)) {
        if(!(stored<epsilon)) {
            const auto scale=1.f/std::sqrt(static_cast<long double>(stored));
            for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(value[n])*scale);
        } else {const auto n=value[0]!=0?0u:value[2]!=0?2u:1u;value[n]=value[n]<0?-1.f:1.f;}
    }
    return value;
}
//004A0EE0/004A1DA0: bind actual HUD+30 inventory inline texts, two model sides,
//slots/capacities/skills and four scrollbars. HUD owns the SAME67 used inline
//texts of its77, scene owns the published drag visual; no duplicate font/text
//catalogue. Presentation reads the real storage/Items owners through checked
//in-call queries. Drag/scroll/transfer UI policy uses these same slots; actual
//Human commands and combat remain with their existing producers.
template<class Frames,class Factory,class Hud>
class HudInventoryLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;using Scroll=HudScrollLoading<Frames>;
    struct Slot {Frame visual{},material{},selection{};const void* item=nullptr;}; //Checked in-call through SAME Items registry before dereference.
    struct Capacity {Frame selection{},bar{};std::array<Slot,4> slots{};std::uint32_t first=0;};
    struct Skill {Frame bar{};};
    HudInventoryLoading(Frames& frames,Factory& factory,Hud& hud):frames_(frames),factory_(factory),hud_(hud),materials_(frames),
        scrolls_{{Scroll(frames),Scroll(frames),Scroll(frames),Scroll(frames)}} {}
    void bind(Frame scene,void* context) {
        if(bound_)throw std::logic_error("HUD inventory rebound before borrower retirement");
        require(scene);bound_=true;context2df0_=context;scene_=scene;
        root_=frames_.hud_find(scene,"inventory",1);sides_[0]=frames_.hud_find(scene,"items00",0x20);sides_[1]=frames_.hud_find(scene,"items01",0x20);
        require(root_);frames_.hud_position(root_,{{.1780000030994415283203125f,-.333000004291534423828125f,0}});
        show(root_,false);frames_.update_scene_object(root_);
        constexpr std::array<std::array<const char*,3>,6> slots{{
            {{"bck*","text_back*","selbck*"}},{{"head*","text_head*","selhead*"}},{{"drop*","text_drop*","seldrop*"}},
            {{"hand*","text_hand*","selhand*"}},{{"uniform*","text_uniform*","seluniform*"}},{{"shoulder*","text_shoulder*","selshoulder*"}}
        }};
        for(unsigned n=0;n!=6;++n) {
            for(unsigned side=0;side!=2;++side) {
                auto& slot=body_[n][side];slot.visual=find(sides_[side],slots[n][0],0x10001);
                slot.material=materials_.material_for_replacement(slot.visual,"X_SQUARE.BMP");
                const auto offset=0xf0+n*0x290+side*0x148;
                bind_text(offset+0xa8,slot.visual,false);frames_.hud_inventory_mesh18(slot.visual);
                const auto caption=find(sides_[side],slots[n][1],0x10001);show(caption,false);
                bind_text(offset,caption,true);localize(offset,0xa96+n);
                //Tags for BOTH sides occur after right caption, but before right selection lookup.
                if(side){frames_.hud_frame_tag(body_[n][0].visual,200+n);frames_.hud_frame_tag(slot.visual,210+n);}
                slot.selection=find(sides_[side],slots[n][2],0x10001);show(slot.selection,false);
            }
        }
        for(unsigned section=0;section!=4;++section)bind_capacity(section);
        for(unsigned n=0;n!=4;++n) {
            for(unsigned section=0;section!=4;++section) {
                auto& slot=capacity_[section].slots[n];frames_.hud_inventory_mesh18(slot.visual);
                bind_text(capacity_offset(section)+0xc0+n*0x9c,slot.visual,false);
                slot.material=materials_.material_for_replacement(slot.visual,"X_SQUARE.BMP");
            }
            for(unsigned section=0;section!=4;++section)frames_.hud_frame_tag(capacity_[section].slots[n].visual,220+section*10+n);
        }
        original_texture_=materials_.texture(body_[0][0].visual,"X_SQUARE.BMP"); //Borrowed, no texture retain in this function.
        totals_[0]=find(sides_[0],"allcpct00",1);totals_[1]=find(sides_[1],"allcpct01",1);
        bind_text(0x2b80,find(sides_[0],"namexx00",0x10001),false);
        bind_text(0x2c1c,find(sides_[1],"namexx01",0x10001),false);
        const auto first_total=find(sides_[0],"text_allcpct00",0x10001);bind_text(0x2cb8,first_total,false);show(first_total,false);
        const auto second_total=find(sides_[1],"text_allcpct01",0x10001);bind_text(0x2d54,second_total,false);show(second_total,false);
        description_=frames_.hud_find(scene,"description",1);show(description_,false);bind_dragging_visual();
        fieldc4_=0;flag2df4_=0;field2df8_=0;
        skills_root_=frames_.hud_find(scene,"skills",0x20);
        const auto stats=find(root_,"text_soldier stats",1);bind_text(0x1fe4,stats,false);localize(0x1fe4,0xa6b);show(stats,false);
        constexpr std::array<std::array<const char*,3>,7> skills{{
            {{"text_shooting","shcpct00","text_shooting01"}},{{"text_strenght","stcpct00","text_strenght01"}},
            {{"text_health","hecpct00","text_health01"}},{{"text_endurance","encpct00","text_endurance01"}},
            {{"text_stealth","sicpct00","text_stealth01"}},{{"text_first aid","facpct00","text_first aid01"}},
            {{"text_lockpick","lpcpct00","text_lockpick01"}}
        }};
        for(unsigned n=0;n!=7;++n) {
            const auto caption=find(root_,skills[n][0],1);bind_text(0x2080+n*0x9c,caption,true);localize(0x2080+n*0x9c,0xa6e + n);show(caption,false);
            skills_[n].bar=find(root_,skills[n][1],1);
            const auto value=find(root_,skills[n][2],1);bind_text(0x24e0+n*0x9c,value,true);show(value,false);
        }
        for(unsigned n=0;n!=4;++n) {
            std::array<char,32> name{};std::snprintf(name.data(),name.size(),"scroll%02u",13+n);
            const auto root=frames_.actor_find(root_,name.data(),1);
            if(root)scrolls_[n].bind(this,root,0xf10+n*0x10);
        }
        field2e10_=0;
    }
    void hide(){ //004A3920(false), including the dragging-frame update after clearing C4.
        show(root_,false);show(description_,false);fieldc4_=0;
        show(dragging_,false);frames_.update_scene_object(dragging_);
    }
    void close()noexcept{} //004A1DA0 is an actual RET. Inline text/scroll ownership survives this loaded-panel close.
    void draw(){ //004A41E0: same inline text owners; preserve the repeated1394 call.
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        const auto draw_text=[&](std::uint32_t offset){factory_.hud_draw_text(text(offset));};
        for(unsigned n=0;n!=6;++n){const auto base=0xf0+n*0x290;
            draw_text(base);draw_text(base+0xa8);
            if(hud_.inventory_word(0x2b74)&2u){draw_text(base+0x148);draw_text(base+0x1f0);}
        }
        for(auto offset:{0x1050u,0x1394u,0x1818u,0x1b5cu})draw_text(offset);
        for(unsigned n=0;n!=4;++n){draw_text(0x1110+n*0x9c);draw_text(0x18d8+n*0x9c);}
        draw_text(0x2b80);draw_text(0x2cb8);
        if(hud_.inventory_word(0x2b74)&2u){
            for(auto offset:{0x1434u,0x1394u,0x1bfcu,0x1f40u})draw_text(offset);
            for(unsigned n=0;n!=4;++n){draw_text(0x14f4+n*0x9c);draw_text(0x1cbc+n*0x9c);}
            draw_text(0x2c1c);draw_text(0x2d54);
        }
        draw_skills();require(description_);if(frames_.hud_frame_visible(description_))draw_text(0x20);
    }
    void draw_skills(){ //004A49A0 checks skills frame1FE0, not the caption frame.
        require(skills_root_);if(!frames_.hud_frame_visible(skills_root_))return;
        factory_.hud_draw_text(text(0x1fe4));
        for(unsigned n=0;n!=7;++n){factory_.hud_draw_text(text(0x2080+n*0x9c));factory_.hud_draw_text(text(0x24e0+n*0x9c));}
    }
    static unsigned body_holder(unsigned row){ //004A1DB0.
        constexpr std::array<unsigned,6> holders{{5,0,7,3,4,2}};return row<holders.size()?holders[row]:7;
    }
    template<class Actor,class Queries>void publish_actors(std::uint32_t flags,Actor* primary,Actor* secondary,Queries& queries){ //004A4380.
        hud_.inventory_word(0x2b74)=flags|8u;actors_[0]=primary?queries.identity(*primary):nullptr;actors_[1]=secondary?queries.identity(*secondary):nullptr;
        if(primary){auto& actor=*queries.resolve(actors_[0]);const auto kind=queries.human_kind(actor);std::string name;
            if(kind==3||kind==4||kind==5)name=queries.full_name(actor);else{const auto rank=queries.rank(actor);name=rank+" "+queries.full_name(actor);}set_string(0x2b80,std::move(name),0x1b);}
        if(secondary&&queries.kind(*queries.resolve(actors_[1]))==3){auto& actor=*queries.resolve(actors_[1]);const auto kind=queries.human_kind(actor);
            std::string name;if(kind==2||kind==3||kind==4||kind==5)name=queries.caption(actor);else{const auto rank=queries.rank(actor);name=rank+" "+queries.caption(actor);}set_string(0x2c1c,std::move(name),0x1b);}
        else text(0x2c1c).text.clear();
    }
    template<class Queries>void show_inventory(bool enabled,Queries& queries){ //004A3920, root publication precedes refresh.
        show(root_,enabled);if(!enabled){show(description_,false);fieldc4_=0;show(dragging_,false);frames_.update_scene_object(dragging_);return;}
        const bool second=(hud_.inventory_word(0x2b74)&2u)!=0;show(sides_[1],second);show(skills_root_,!second);refresh(queries);
    }
    template<class Actor,class Queries>void refresh_body(Actor* actor,unsigned side,Queries& queries){ //004A1E20, actual body holder mapping, fresh queries.
        if(!actor)return;if(side>1)throw std::out_of_range("Inventory body side");
        for(unsigned row=0;row!=6;++row){auto& slot=body_[row][side];const auto offset=0xf0+row*0x290+side*0x148+0xa8;const auto holder=body_holder(row);
            auto entry=holder==7?typename Queries::Entry{}:queries.holder_entry(*actor,holder);
            if(!entry){slot.item=nullptr;set_texture(slot.material,original_texture_);text(offset).text.clear();continue;}
            entry=queries.holder_entry(*actor,holder);slot.item=queries.item_identity(entry);
            if(row==4){const bool full=side==0||(queries.unavailable(*actor)&&!queries.human_flag1a5(*actor));require(slot.visual);frames_.human_equipment_opacity(slot.visual,full?1.f:.699999988079071f);}
            set_texture(slot.material,queries.item_icon(queries.holder_entry(*actor,holder)));
            if(queries.item_family(slot.item)==0&&queries.item_amount(slot.item)!=-1.f)set_string(offset,std::to_string(human_hud_truncate(queries.item_amount(slot.item))),6);
            else text(offset).text.clear();
        }
    }
    template<class Actor,class Queries>void refresh_array(Actor& actor,unsigned section,unsigned storage_index,Queries& queries){ //004A2010.
        auto& part=capacity_.at(section);const auto offset=capacity_offset(section);auto* storage=queries.array_storage(actor,storage_index);
        if(!storage){for(unsigned row=0;row!=4;++row){auto& slot=part.slots[row];slot.item=nullptr;require(slot.visual);frames_.hud_inventory_slot_opacity(slot.visual,.5f);set_texture(slot.material,original_texture_);text(offset+0xc0+row*0x9c).text.clear();}
            text(offset).bytes70[1]=1;text(offset).text.disabled=true;require(part.bar);frames_.hud_info_scale(part.bar,1,1,0);text(offset+0x344).text.clear();return;}
        //Virtual3C provides the actual slot vector,20 returns its current volume,10 its limit.
        (void)storage->slots();const float used=static_cast<float>(storage->total_volume());const auto maximum=storage->second_scalar();
        require(part.bar);frames_.hud_info_scale(part.bar,1,1,static_cast<float>(static_cast<long double>(used)/maximum));frames_.update_scene_object(part.bar);
        text(offset).bytes70[1]=0;text(offset).text.disabled=false;
        const auto label=factory_.hud_localized(0x1cf2);auto* fresh=queries.array_storage(actor,storage_index);if(!fresh)throw std::logic_error("Inventory array retired during capacity publication");
        const auto limit=human_hud_truncate(fresh->second_scalar());fresh=queries.array_storage(actor,storage_index);if(!fresh)throw std::logic_error("Inventory array retired during capacity publication");
        const auto amount=human_hud_truncate(fresh->total_volume());set_string(offset+0x344,(label?*label:std::string{})+": "+std::to_string(amount)+"/"+std::to_string(limit),0x1d);
        for(unsigned row=0;row!=4;++row){auto& slot=part.slots[row];const auto index=static_cast<std::size_t>(part.first)+row;
            require(slot.visual);frames_.hud_inventory_slot_opacity(slot.visual,1.f);storage=queries.array_storage(actor,storage_index);if(!storage)throw std::logic_error("Inventory array retired during row publication");
            const auto& entries=storage->slots().at(index).entries;const auto text_offset=offset+0xc0+row*0x9c;
            if(entries.empty()){slot.item=nullptr;set_texture(slot.material,original_texture_);text(text_offset).text.clear();continue;}
            slot.item=queries.item_identity(entries.front());set_texture(slot.material,queries.item_icon(entries.front()));const auto family=queries.item_family(slot.item);
            if(family==0){const auto amount=human_hud_truncate(queries.item_amount(slot.item));
                set_string(text_offset,std::to_string(amount==-1?static_cast<std::int32_t>(entries.size()):human_hud_truncate(queries.item_amount(slot.item))),6);}
            else if(family==2){std::uint32_t total=0;for(std::size_t n=0;n<entries.size();++n)total+=static_cast<std::uint32_t>(human_hud_truncate(queries.item_amount(queries.item_identity(entries[n]))));
                set_string(text_offset,std::to_string(human_hud_signed(total))+"/"+std::to_string(entries.size()),6);}
            else set_string(text_offset,std::to_string(entries.size()),6);
        }
    }
    template<class Actor,class Queries>void refresh_scroll(Actor& actor,unsigned section,unsigned storage_index,Queries& queries){ //004A2640 tag mapping.
        auto* storage=queries.array_storage(actor,storage_index);const auto count=storage?storage->slots().size():0;
        if(count>0x7fffffffu)throw std::length_error("Inventory slots exceed native range");auto& scroll=scrolls_.at(scroll_index(section));scroll.set_range(static_cast<std::int32_t>(count)-3);capacity_.at(section).first=static_cast<std::uint32_t>(scroll.index());
    }
    template<class Queries>void refresh(Queries& queries){ //004A26D0/004A49F0, no independent actor/Items owner.
        if(fieldc4_){fieldc4_=0;show(dragging_,false);frames_.update_scene_object(dragging_);}
        if(!actors_[0])throw std::logic_error("Inventory refresh requires actual primary actor");auto& first=*queries.resolve(actors_[0]);refresh_weight(first,0,queries);refresh_body(&first,0,queries);
        refresh_scroll(first,0,1,queries);refresh_array(first,0,1,queries);refresh_scroll(first,2,6,queries);refresh_array(first,2,6,queries);
        if(hud_.inventory_word(0x2b74)&2u){if(!actors_[1])throw std::logic_error("Two-sided inventory requires actual secondary actor");auto& second=*queries.resolve(actors_[1]);refresh_weight(second,1,queries);
            if(queries.kind(second)==3){refresh_body(&second,1,queries);refresh_scroll(second,1,1,queries);refresh_array(second,1,1,queries);refresh_scroll(second,3,6,queries);refresh_array(second,3,6,queries);}}
        hud_.inventory_word(0x2b74)&=~8u;
    }
    template<class Queries>void update_skills(Queries& queries){ //004A4680, skills root gate before Human queries.
        require(skills_root_);if(!frames_.hud_frame_visible(skills_root_))return;if(!actors_[0])throw std::logic_error("Inventory skills require actual primary Human");
        constexpr std::array<unsigned,7> indices{{4,1,0,2,6,7,8}};
        for(unsigned row=0;row!=7;++row){auto& actor=*queries.resolve(actors_[0]);const auto index=indices[row];float scale;std::int32_t number;std::string value;
            if(index==0){number=queries.health_percent(actor);scale=static_cast<float>(static_cast<long double>(number)*.009999999776482582f);value=number_string("%5d%%",number);}
            else if(index<3){number=queries.raw_stat(actor,index);scale=static_cast<float>(static_cast<long double>(number)*.009999999776482582f);value=number_string("%5d",number);}
            else{const auto stat=queries.stat(actor,index);scale=static_cast<float>(stat);number=human_hud_truncate(stat*100.f);value=number_string("%5d%%",number);}
            set_string(0x24e0+row*0x9c,std::move(value),0xb);require(skills_[row].bar);frames_.hud_info_scale(skills_[row].bar,1,1,scale);frames_.update_scene_object(skills_[row].bar);
        }
    }
    template<class Pointer>bool pointer_inside(Pointer& pointer){ //004A4630→004ABC00, strict open rectangle, AL-only result.
        if(!root_||!frames_.hud_frame_visible(root_))return false;const auto bounds=frames_.hud_text_world_bounds(root_);const auto position=pointer.pointer_position();
        return position[0]>bounds[0]&&position[1]>bounds[1]&&position[0]<bounds[4]&&position[1]<bounds[5];
    }
    template<class Pointer,class Queries>void update_description(std::uint32_t elapsed,Pointer& pointer,Queries& queries){ //004A39B0.
        auto origin=pointer.pointer_position();origin[2]=static_cast<float>(static_cast<long double>(origin[2])+.5f);
        if(fieldc4_||!pointer_inside(pointer)){hide_description();return;}
        const auto hit=frames_.hud_inventory_hit(scene_,origin,{{0,0,1,0}});if(!hit.first||!hit.second){hide_description();return;}
        if(hover_==hit.first)hover_ticks_+=elapsed;else{hover_=hit.first;show(description_,false);hover_ticks_=0;}
        if(human_hud_signed(hover_ticks_)<=400||frames_.hud_frame_visible(description_))return;
        const auto identity=description_item(hit.second);if(!identity)return;
        queries.validate_item(identity); //SAME Items membership before any retired pointer can be read.
        show(description_,true);const auto position=pointer.pointer_position();
        frames_.hud_message_position(description_,static_cast<float>(static_cast<long double>(position[0])+.15000000596046448f),
            static_cast<float>(static_cast<long double>(position[1])+.014999999664723873f),static_cast<float>(static_cast<long double>(position[2])+.10000000149011612f));
        frames_.update_scene_object(description_);auto& display=text(0x20);bind_hud_text_geometry(frames_,display,scene_,description_);display.text.clear();
        const auto append=[&](std::string value,std::uint32_t style){display.text.append(std::make_shared<const std::string>(std::move(value)),style);};
        append(" ",0x11);display.text.append(factory_.hud_localized(queries.item_caption(identity)),0x10);append(" ",0x11);
        append(localized(0xaaa)+" : "+decimal(queries.item_weight(identity))+" kg",0x11);
        const auto kind=queries.item_kind(identity);if((kind>=2&&kind<=5)||kind==12){
            if(queries.item_ammunition(identity)>=0){const auto label=localized(0xaab);const auto capacity=human_hud_truncate(queries.item_capacity(identity));const auto amount=human_hud_truncate(queries.item_amount(identity));
                append(label+" : "+std::to_string(amount)+"/"+std::to_string(capacity),0x11);}
            const auto speed=queries.item_description_speed(identity);if(speed)append(localized(0xaac)+" : "+decimal(*speed)+" m/s",0x11);
            const auto zoom=queries.item_description_zoom(identity);if(zoom)append(localized(0xaae)+" : "+decimal(*zoom)+" x",0x11);
        }
    }
    template<class Pointer,class Queries>void update(std::uint32_t elapsed,Pointer& pointer,Queries& queries){ //004A4150.
        flush_retired_drag();
        if(field2df8_){field2df8_-=elapsed;if(human_hud_signed(field2df8_)<0)field2df8_=0;}
        if(hud_.inventory_word(0x2b74)&8u){refresh(queries);hud_.inventory_word(0x2b74)&=~8u;}
        if(!actors_[0])throw std::logic_error("Inventory frame requires actual primary actor");
        if(queries.inventory_changed(*queries.resolve(actors_[0]))||(actors_[1]&&queries.inventory_changed(*queries.resolve(actors_[1]))))refresh(queries);
        update_skills(queries);update_description(elapsed,pointer,queries);
    }
    template<class Pointer,class Input,class Queries>void process_controls(Pointer& pointer,Input& input,Queries& queries){ //004A3420, caller owns visibility gate and pointer motion.
        flush_retired_drag();process_scroll_controls(pointer,input,queries);const bool click=input.second(55)!=0;
        if(!field2df8_){
            if(click){if(primary_health(queries)>0){clear_drag_if_any();flag2df4_=1;field2df8_=250;click_position_=pointer.pointer_position();const auto tag=pointer.pointer_hit_tag();if(tag)begin_drag_tag(tag,pointer,queries);}return;} //004A3790.
            if(input.first(55)!=0){if(fieldc4_){require(dragging_);if(!frames_.hud_frame_visible(dragging_)){
                        const auto position=pointer.pointer_position();if(click_position_[0]!=position[0]||click_position_[1]!=position[1]||click_position_[2]!=position[2])show_drag(pointer,queries);}
                    if(fieldc4_){if(primary_health(queries)<1)finish_drag(pointer,queries);else position_drag(pointer);}}return;} //004A3870/004A2CC0/004A2D60.
            if(flag2df4_){flag2df4_=0;if(fieldc4_)finish_drag(pointer,queries);return;} //004A38F0.
            if(input.second(56)!=0)return; //004A3910 is a genuine RET.
        }else if(click&&primary_health(queries)>0){fieldc4_=nullptr;source_actor_e4_=nullptr;const auto tag=pointer.pointer_hit_tag();if(tag)quick_transfer(tag,queries);} //004A3830.
    }
    const void* dragging_item()const{return fieldc4_;}std::uint32_t click_time()const{return field2df8_;}
    void retire_actor(const void* actor)noexcept{if(source_actor_e4_==actor)retire_drag();if(actors_[0]==actor||actors_[1]==actor){actors_.fill(nullptr);clear_item_borrowers();}}
    void clear_item_borrowers()noexcept{retire_drag();for(auto& pair:body_)for(auto& slot:pair)slot.item=nullptr;for(auto& section:capacity_)for(auto& slot:section.slots)slot.item=nullptr;}
    void retire_item(const void* item)noexcept{bool changed=false;for(auto& pair:body_)for(auto& slot:pair)if(slot.item==item){slot.item=nullptr;changed=true;}
        for(auto& section:capacity_)for(auto& slot:section.slots)if(slot.item==item){slot.item=nullptr;changed=true;}
        if(fieldc4_==item)retire_drag();
        if(changed){hover_=Frame{};hover_ticks_=0;for(const auto& owner:hud_.text_owners())if(owner.original_offset==0x50&&owner.text)owner.text->text.clear();}}
    void check_refresh_binding()const{if(refresh_owner_)throw std::logic_error("Inventory already has its actual UI refresh receiver");}
    void bind_refresh(void* owner,std::function<void()> receiver){check_refresh_binding();if(!owner||!receiver)throw std::invalid_argument("Inventory requires actual refresh owner/receiver");refresh_=std::move(receiver);refresh_owner_=owner;}
    void unbind_refresh(void* owner)noexcept{if(refresh_owner_==owner){refresh_={};refresh_owner_=nullptr;actors_.fill(nullptr);clear_item_borrowers();}}
    void notify_changed(){if(!refresh_)throw std::runtime_error("Visible inventory requires actual containing UI refresh binding");refresh_();}
    void retire_inline_scrolls()noexcept{for(unsigned n=4;n;)scrolls_[--n].close_geometry();} //Containing cold HUD owner must retire these, no constructor/destructor address credit here.
    void discard_borrowers()noexcept {
        for(auto& scroll:scrolls_)scroll.discard_borrowers();
        scene_=root_=description_=skills_root_=dragging_=dragging_material_=original_texture_=Frame{};
        sides_.fill(Frame{});totals_.fill(Frame{});body_={};capacity_={};skills_={};actors_.fill(nullptr);hover_=Frame{};hover_ticks_=0;context2df0_=nullptr;bound_=false;
        fieldc4_=source_actor_e4_=nullptr;drag_retired_=false;
    }
    Frame root()const{return root_;}const auto& sides()const{return sides_;}const auto& body_slots()const{return body_;}
    const auto& capacities()const{return capacity_;}Frame original_texture()const{return original_texture_;}Frame dragging()const{return dragging_;}
    Frame dragging_material()const{return dragging_material_;}Frame skills_root()const{return skills_root_;}const auto& skills()const{return skills_;}
    const auto& scrolls()const{return scrolls_;}void* context()const{return context2df0_;}
private:
    static unsigned scroll_index(unsigned section){constexpr std::array<unsigned,4> indices{{0,2,1,3}};return indices.at(section);} //Native F10/F20/F30/F40 are left pocket/left pack/right pocket/right pack.
    template<class Queries>std::int32_t primary_health(Queries& queries){if(!actors_[0])throw std::logic_error("Inventory controls require actual primary actor");return queries.health(*queries.resolve(actors_[0]));}
    void retire_drag()noexcept{if(fieldc4_)drag_retired_=true;fieldc4_=source_actor_e4_=nullptr;}
    void flush_retired_drag(){if(drag_retired_){show(dragging_,false);frames_.update_scene_object(dragging_);drag_retired_=false;}}
    void clear_drag_if_any(){if(fieldc4_){fieldc4_=source_actor_e4_=nullptr;show(dragging_,false);frames_.update_scene_object(dragging_);}}
    template<class Pointer>void drag_offset(Frame visual,Pointer& pointer){require(visual);const auto world=frames_.actor_world_matrix(visual);const auto position=pointer.pointer_position();
        for(unsigned n=0;n!=3;++n)drag_offset_[n]=static_cast<float>(static_cast<long double>(world[12+n])-position[n]);drag_offset_[2]=.10000000149011612f;
    }
    template<class Pointer,class Queries>void begin_drag_tag(std::uint32_t tag,Pointer& pointer,Queries& queries){ //004A34D0, fresh primary health gate.
        if(primary_health(queries)<1)return;
        for(unsigned side=0;side!=2;++side)if(tag>=200+side*10&&tag<206+side*10){begin_body(side,tag-200-side*10,pointer,queries);return;}
        for(unsigned section=0;section!=4;++section)if(tag>=220+section*10&&tag<224+section*10){begin_array(section,tag-220-section*10,pointer,queries);return;}
    }
    template<class Pointer,class Queries>void begin_body(unsigned side,unsigned row,Pointer& pointer,Queries& queries){ //004A2A40.
        clear_drag_if_any();if(row==4){if(queries.offline()&&human_hud_signed(queries.game_status())>=14&&queries.uniform_integrity_mismatch())return;
            if(!actors_[1])return;auto& other=*queries.resolve(actors_[1]);if(!queries.unavailable(other)||queries.human_flag1a5(other))return;}
        const auto& slot=body_.at(row).at(side);if(!slot.item)return;
        source_actor_e4_=actors_[side];source_holder_e0_=body_holder(row);fieldc4_=slot.item;drag_offset(slot.visual,pointer);
    }
    template<class Pointer,class Queries>void begin_array(unsigned section,unsigned row,Pointer& pointer,Queries& queries){ //004A2BB0, material is bound immediately for arrays.
        clear_drag_if_any();const auto& slot=capacity_.at(section).slots.at(row);if(!slot.item)return;
        source_actor_e4_=actors_[section&1u];source_holder_e0_=section<2?1u:6u;fieldc4_=slot.item;
        set_texture(dragging_material_,queries.identity_icon(fieldc4_));drag_offset(slot.visual,pointer);
    }
    template<class Pointer>void position_drag(Pointer& pointer){const auto position=pointer.pointer_position();std::array<float,4> value=drag_offset_;
        for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(drag_offset_[n])+position[n]);require(dragging_);frames_.hud_scroll_position(dragging_,value);frames_.update_scene_object(dragging_);
    }
    template<class Pointer,class Queries>void show_drag(Pointer& pointer,Queries& queries){ //004A2CC0, bind→position→show→update.
        if(primary_health(queries)<1)return;set_texture(dragging_material_,queries.identity_icon(fieldc4_));
        const auto position=pointer.pointer_position();auto value=drag_offset_;for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(drag_offset_[n])+position[n]);
        require(dragging_);frames_.hud_scroll_position(dragging_,value);show(dragging_,true);frames_.update_scene_object(dragging_);
    }
    void clear_selections(){ //004A31F0, no update calls and secondary based on actor presence, not flags.
        for(unsigned side=0;side!=2;++side){if(side&&!actors_[1])break;for(unsigned row=0;row!=6;++row)show(body_[row][side].selection,false);
            show(capacity_[side].selection,false);show(capacity_[side+2].selection,false);}
    }
    template<class Pointer,class Queries>void finish_drag(Pointer& pointer,Queries& queries){ //004A2DE0; OUTSIDE drop does not short-circuit a subsequent tag-directed transfer.
        if(primary_health(queries)>0){
            const auto drop=[&]{if(source_holder_e0_!=4){if(!source_actor_e4_||!fieldc4_)throw std::logic_error("Inventory drag lost its source/item loan");queries.drop(*queries.resolve(source_actor_e4_),fieldc4_,source_holder_e0_);}};
            if(!pointer_inside(pointer))drop();const auto tag=pointer.pointer_hit_tag();
            if(tag==202||tag==212)drop();
            else {const void* destination=nullptr;const void* displaced=nullptr;unsigned holder=7,index=0;
                for(unsigned side=0;side!=2;++side)if(tag>=200+side*10&&tag<206+side*10){destination=actors_[side];const auto row=tag-200-side*10;holder=body_holder(row);displaced=body_[row][side].item;break;}
                for(unsigned section=0;section!=4;++section)if(tag>=220+section*10&&tag<224+section*10){destination=actors_[section&1u];holder=section<2?1u:6u;index=tag-220-section*10;break;}
                if(destination&&(destination!=source_actor_e4_||holder!=source_holder_e0_)){
                    if(!source_actor_e4_||!fieldc4_)throw std::logic_error("Inventory drag source retired during control dispatch");
                    auto& target=*queries.resolve(destination);auto& source=*queries.resolve(source_actor_e4_);
                    if(displaced)queries.swap(target,displaced,holder,source,fieldc4_,source_holder_e0_);
                    else queries.move(target,fieldc4_,source,source_holder_e0_,holder,index);
                }
            }
        }
        fieldc4_=source_actor_e4_=nullptr;show(dragging_,false);frames_.update_scene_object(dragging_);clear_selections();refresh(queries);
    }
    template<class Queries>void quick_transfer(std::uint32_t tag,Queries& queries){ //004A35D0 ignores return values from actual Human005A2B40.
        const void* entry=nullptr;const void* source=nullptr;const void* destination=nullptr;unsigned holder=7;
        for(unsigned side=0;side!=2;++side)if(tag>=200+side*10&&tag<206+side*10){const auto row=tag-200-side*10;if(row==2)return;entry=body_[row][side].item;source=actors_[side];destination=side?actors_[0]:actors_[1]?actors_[1]:actors_[0];holder=body_holder(row);break;}
        for(unsigned section=0;section!=4;++section)if(tag>=220+section*10&&tag<224+section*10){entry=capacity_[section].slots[tag-220-section*10].item;const auto side=section&1u;source=actors_[side];destination=side?actors_[0]:actors_[1]?actors_[1]:actors_[0];holder=section<2?1u:6u;break;}
        if(entry){if(!destination||!source)throw std::logic_error("Inventory quick transfer lost its actual actors");queries.move(*queries.resolve(destination),entry,*queries.resolve(source),holder,7,0);}
    }
    template<class Queries>void scroll_changed(std::uint32_t tag,std::uint32_t index,Queries& queries){ //004A2590.
        const unsigned section=tag==0xf10?0u:tag==0xf20?2u:tag==0xf30?1u:tag==0xf40?3u:4u;if(section==4)return;
        capacity_[section].first=index;if(!actors_[section&1u])throw std::logic_error("Inventory scroll requires actual side actor");refresh_array(*queries.resolve(actors_[section&1u]),section,section<2?1u:6u,queries);
    }
    template<class Pointer,class Input,class Queries>void process_scroll_controls(Pointer& pointer,Input& input,Queries& queries){ //004A3280, hold branch returns before hover routing.
        const auto tag=pointer.pointer_hit_tag();const auto changed=[&](std::uint32_t id,std::uint32_t index){scroll_changed(id,index,queries);};
        const auto active=[&]() -> Scroll& {if(field2e10_<1||field2e10_>4)throw std::out_of_range("Inventory scrollbar identity exceeds original four");return scrolls_.at(field2e10_-1);};
        if(field2e10_){if(input.second(55)!=0)active().click(changed);else if(input.first(55)!=0){const auto delta=pointer.pointer_delta();const auto position=pointer.pointer_position();active().drag(position,delta,changed);return;}else if(flag2df4_)active().release();}
        if((tag&0xf00u)==0xf00u){const auto next=(tag>>4)&15u;if(field2e10_==next){active().hover(tag);return;}
            if(input.first(55)==0&&input.first(56)==0&&(!field2e10_||active().leave()))field2e10_=next;
        }else if(field2e10_&&active().leave())field2e10_=0;
    }
    const void* description_item(std::uint32_t tag)const{
        if(tag>=200&&tag<=205)return body_[tag-200][0].item;if(tag>=210&&tag<=215)return body_[tag-210][1].item;
        for(unsigned section=0;section!=4;++section){const auto base=220+section*10;if(tag>=base&&tag<base+4)return capacity_[section].slots[tag-base].item;}
        return nullptr;
    }
    void hide_description(){show(description_,false);hover_ticks_=0;hover_=Frame{};}
    std::string localized(std::uint32_t id){const auto value=factory_.hud_localized(id);return value?*value:std::string{};}
    static std::string decimal(float value){std::array<char,128> buffer{};const auto count=std::snprintf(buffer.data(),buffer.size(),"%.2f",static_cast<double>(value));if(count<0||static_cast<std::size_t>(count)>=buffer.size())throw std::runtime_error("Inventory decimal formatting failed");return buffer.data();}
    static std::string number_string(const char* format,std::int32_t number){std::array<char,64> buffer{};const auto size=std::snprintf(buffer.data(),buffer.size(),format,number);if(size<0||static_cast<std::size_t>(size)>=buffer.size())throw std::runtime_error("Inventory number formatting failed");return buffer.data();}
    void set_string(std::uint32_t offset,std::string value,std::uint32_t style){text(offset).text.set(std::make_shared<const std::string>(std::move(value)),style,0);}
    void set_texture(Frame material,Frame texture){require(material);frames_.first_person_material_texture(material,texture);}
    template<class Actor,class Queries>void refresh_weight(Actor& actor,unsigned side,Queries& queries){
        const auto weight=queries.weight(actor);const auto maximum=queries.weight_limit(actor);set_string(0x2cb8+side*0x9c,std::to_string(human_hud_truncate(weight))+"/"+std::to_string(maximum)+" kg",0x1b);
        const auto fresh_weight=queries.weight(actor);const auto fresh_limit=queries.weight_limit(actor);require(totals_[side]);frames_.hud_info_scale(totals_[side],1,1,static_cast<float>(static_cast<long double>(fresh_weight)/fresh_limit));
    }
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD inventory artwork is missing");}
    Frame find(Frame root,const char* name,std::uint32_t mask){require(root);return frames_.actor_find(root,name,mask);}
    void show(Frame frame,bool visible){require(frame);frames_.object_visible(frame,visible);}
    Text& text(std::uint32_t offset) {
        for(const auto& entry:hud_.text_owners())if(entry.original_offset==offset+0x30){if(entry.text)return *entry.text;break;}
        throw std::logic_error("HUD inventory requires its actual inline text owner");
    }
    void bind_text(std::uint32_t offset,Frame frame,bool zero_inset){auto& t=text(offset);if(zero_inset)t.text.inset=0;bind_hud_text_geometry(frames_,t,scene_,frame);}
    void localize(std::uint32_t offset,std::uint32_t id){text(offset).text.set(factory_.hud_localized(id),0xb,0);}
    static unsigned capacity_offset(unsigned section){return 0x1050+section*0x3e4;}
    void bind_capacity(unsigned section) {
        const auto side=section&1u;const bool backpack=section>=2;auto& state=capacity_[section];const auto offset=capacity_offset(section);
        const auto caption=find(sides_[side],backpack?"bkpckcaptab*":"hlstrscaptab*",0x10001);
        bind_text(offset,caption,true);localize(offset,backpack?0xa9d:0xa9c);show(caption,false);
        const auto value=find(sides_[side],backpack?"text_bkpckcap*":"text_hlstrscap*",0x10001);bind_text(offset+0x344,value,true);show(value,false);
        for(unsigned n=0;n!=4;++n){std::array<char,32> name{};std::snprintf(name.data(),name.size(),"item%02u*",n+(backpack?4:0));state.slots[n].visual=find(sides_[side],name.data(),0x10001);}
        constexpr std::array<const char*,4> bars{{"hlcpct00","hlcpct02","hlcpct01","hlcpct03"}};
        state.bar=find(sides_[side],bars[section],section==0?0x10001u:1u);
        state.selection=find(sides_[side],backpack?"selbackpack*":"selholsters*",0x10001);show(state.selection,false);
    }
    void bind_dragging_visual() {
        auto owner=frames_.create_hud_inventory_visual();if(!owner)throw std::runtime_error("HUD inventory dragging factory returned null");
        dragging_=owner.get();frames_.hud_inventory_duplicate(dragging_,body_[0][0].visual);
        const auto matrix=frames_.actor_world_matrix(body_[0][0].visual);
        const auto direction=hud_inventory_direction({{matrix[8],matrix[9],matrix[10],matrix[11]}});
        frames_.hud_inventory_forward(dragging_,direction);frames_.hud_visual_flags(dragging_,0x1000);frames_.car_particle_name(dragging_,"DraggingFrame");
        frames_.hud_injury_prepare(dragging_);frames_.hud_inventory_mesh18(dragging_);
        frames_.hud_inventory_link_root(dragging_,scene_);show(dragging_,false);frames_.update_scene_object(dragging_);
        dragging_material_=materials_.material_for_replacement(dragging_,"X_SQUARE.BMP");
        frames_.hud_inventory_add(scene_,dragging_);owner.reset(); //Scene retains, release local ref at004A1AD8.
    }
    Frames& frames_;Factory& factory_;Hud& hud_;HudMaterialLookup<Frames> materials_;std::array<Scroll,4> scrolls_;
    Frame scene_{},root_{},description_{},skills_root_{},dragging_{},dragging_material_{},original_texture_{};
    std::array<Frame,2> sides_{},totals_{};std::array<std::array<Slot,2>,6> body_{};std::array<Capacity,4> capacity_{};std::array<Skill,7> skills_{};
    std::array<const void*,2> actors_{};
    Frame hover_{};std::uint32_t hover_ticks_=0;
    void* refresh_owner_=nullptr;std::function<void()> refresh_;
    void* context2df0_=nullptr;bool bound_=false;const void* fieldc4_=nullptr;const void* source_actor_e4_=nullptr;unsigned source_holder_e0_=0;
    std::array<float,4> drag_offset_{},click_position_{};bool drag_retired_=false;std::uint32_t field2df8_=0,field2e10_=0;std::uint8_t flag2df4_=0;
};
} //namespace hd2::game
#endif
