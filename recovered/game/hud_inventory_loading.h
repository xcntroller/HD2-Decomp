#ifndef HD2_GAME_HUD_INVENTORY_LOADING_H
#define HD2_GAME_HUD_INVENTORY_LOADING_H
#include "game/hud_material_lookup.h"
#include "game/hud_scroll_loading.h"
#include "game/hud_text_loading.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>
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
//catalogue or successful runtime inventory interaction is supplied.
template<class Frames,class Factory,class Hud>
class HudInventoryLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;using Scroll=HudScrollLoading<Frames>;
    struct Slot {Frame visual{},material{},selection{};};
    struct Capacity {Frame selection{},bar{};std::array<Slot,4> slots{};};
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
    void retire_inline_scrolls()noexcept{for(unsigned n=4;n;)scrolls_[--n].close_geometry();} //Containing cold HUD owner must retire these, no constructor/destructor address credit here.
    void discard_borrowers()noexcept {
        for(auto& scroll:scrolls_)scroll.discard_borrowers();
        scene_=root_=description_=skills_root_=dragging_=dragging_material_=original_texture_=Frame{};
        sides_.fill(Frame{});totals_.fill(Frame{});body_={};capacity_={};skills_={};context2df0_=nullptr;bound_=false;
    }
    Frame root()const{return root_;}const auto& sides()const{return sides_;}const auto& body_slots()const{return body_;}
    const auto& capacities()const{return capacity_;}Frame original_texture()const{return original_texture_;}Frame dragging()const{return dragging_;}
    Frame dragging_material()const{return dragging_material_;}Frame skills_root()const{return skills_root_;}const auto& skills()const{return skills_;}
    const auto& scrolls()const{return scrolls_;}void* context()const{return context2df0_;}
private:
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
    void* context2df0_=nullptr;bool bound_=false;std::uint32_t fieldc4_=0,field2df8_=0,field2e10_=0;std::uint8_t flag2df4_=0;
};
} //namespace hd2::game
#endif
