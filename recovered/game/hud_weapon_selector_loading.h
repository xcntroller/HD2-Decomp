#ifndef HD2_GAME_HUD_WEAPON_SELECTOR_LOADING_H
#define HD2_GAME_HUD_WEAPON_SELECTOR_LOADING_H
#include <cstdint>
#include <stdexcept>
#include <vector>
#include "game/hud_material_lookup.h"
#include "game/hud_text_loading.h"
#include "game/text_glyphs.h"
#include "game/human_inventory_counts.h"
#include "frontend/pre_mission_data.h"
#include <memory>
#include <optional>
#include <cstdio>
namespace hd2::game {
//004AC440/004AC530/004AC190/004AC990/004ACB70/004AC370/004ACDC0/004AD140.
//Actual Human storage, Items module and icon cache are required producer inputs.
//Show/hide and expiry/rebuild are bound; interactive weapon transfer and its
//timer30/34 producers remain separate required input consumers.
template<class Frames,class Factory>
class HudWeaponSelectorLoading {
public:
    using Frame=typename Frames::Frame;using Entry=typename Frames::Entry;using Text=typename Factory::DisplayText;
    using Owner=decltype(std::declval<Frames&>().create_hud_inventory_visual());
    struct Record {
        Entry item{};std::uint32_t quantity=0,slot=0;
        std::optional<Owner> visual,caption;std::unique_ptr<Text> text;
        Record()=default;Record(const Record&)=delete;Record& operator=(const Record&)=delete;
        ~Record(){visual.reset();caption.reset();text.reset();} //004AC370 frame0C,frame10 THEN inlineText14/font/layout/records.
    };
    HudWeaponSelectorLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory),materials_(frames) {}
    ~HudWeaponSelectorLoading(){close();}
    void bind(Frame scene) {
        require(scene);scene_=scene;root_=frames_.hud_find(scene,"select_weapon",0x20);
        selector_=frames_.hud_find(scene,"selector_weapon",1);require(root_);
        drkr_=frames_.actor_find(root_,"drkr1",1);hide(drkr_);
        weapon00_=frames_.actor_find(root_,"select_weapon00",1);hide(weapon00_);
        hide(frames_.hud_find(scene,"tabl03",1));hide(frames_.hud_find(scene,"drkr2",1));
        hide(frames_.actor_find(root_,"select_weapon01",1));hide(frames_.actor_find(root_,"select_weapon02",1));
        hide(root_);human_=actor_identity_=nullptr;retired_hide_=false;
    }
    template<class Players,class Items,class Icons>void show(bool enabled,std::uint32_t mode,Players& players,Items& items,Icons& icons){ //004ACE20.
        elapsed38_=0;retired_hide_=false;
        if(!enabled){human_=actor_identity_=nullptr;close();require(root_);frames_.object_visible(root_,false);return;}
        auto* human=players.selected_player(mode);human_=human;actor_identity_=human?&human->base():nullptr;
        if(!human)return;
        auto& state=players.state(*human);const auto& words=state.words(); //Checked same World lifetime.
        if((words.template get<std::uint32_t>(0x48)==1&&(state.derived_loading_word(0x4d4)&0xffu)!=0)||
           words.template get<std::int32_t>(0x60)!=-1)return;
        populate(*human,items,icons);layout_current(players,items);require(root_);frames_.object_visible(root_,true);
    }
    template<class Players,class Items,class Icons>void update(std::uint32_t elapsed,std::uint32_t mode,Players& players,Items& items,Icons& icons){ //004AD180.
        flush_retired();
        auto* human=players.selected_player(mode); //Selection query precedes any old borrower read.
        bool admitted=human_==human&&human;
        if(admitted){auto& state=players.state(*human);const auto& words=state.words();
            admitted=!(words.template get<std::uint32_t>(0x48)==1&&(state.derived_loading_word(0x4d4)&0xffu)!=0)&&words.template get<std::int32_t>(0x60)==-1;}
        if(admitted){
            elapsed38_+=elapsed;
            if(signed_word(elapsed38_)<2001){
                if(human->activation.flag178){close();populate(*human,items,icons);layout_current(players,items);return;}
                const auto bits=human->properties.stats_ec[0];
                if(signed_word(bits)>0){
                    auto* current=players.current_player();if(!current)throw std::runtime_error("HUD selector amount refresh requires actual current Human");
                    (void)players.inline_control(*current);refresh_amounts(*current,items);
                    if(signed_word(timer30_)>0){timer30_-=elapsed;if(signed_word(timer30_)<0)timer30_=0;return;}
                    if(signed_word(timer34_)>0){timer34_-=elapsed;if(signed_word(timer34_)<0)timer34_=0;}
                    return;
                }
            }
        }
        show(false,mode,players,items,icons);
    }
    template<class Input,class Queries>void process_controls(Input& input,Queries& queries){ //004ACF00; caller supplies actual visible-root gate.
        if(retired_hide_){flush_retired();return;}
        if(input.second(19)!=0){auto& human=queries.resolve_human(human_);
            if(queries.selector_busy(human)||queries.action_active(human))return;
            if(records_.size()>1){const auto& row=*records_.at(selected_);const auto selected=row.item;const auto slot=row.slot;const auto current=queries.current_weapon(human);
                if(!selected){if(current)queries.move_weapon(human,current,3,7);}
                else if(queries.item_kind(selected)==13){if(!queries.focused(human))queries.use_item(human,selected);}
                else if(selected!=current){if(!current)queries.move_weapon(human,selected,slot,3);else queries.swap_weapon(human,current,selected,slot);}
            }
            input.second(19,0);input.first(19,0);queries.hide_selector(); //Native continues held navigation AFTER closing records.
        }
        if(input.first(33)==0)timer30_=0xffffffffu;else{if(!timer30_){input.second(33,1);timer30_=200;}elapsed38_=0;}
        if(input.first(32)==0)timer34_=0xffffffffu;else{if(!timer34_){input.second(32,1);timer34_=200;}elapsed38_=0;}
        if(input.second(33)!=0){++selected_;if(selected_==records_.size())selected_=0;queries.layout_selector();timer30_=200;elapsed38_=0;return;}
        if(input.second(32)!=0){--selected_;if(signed_word(selected_)<0)selected_=static_cast<std::uint32_t>(records_.size())-1u;queries.layout_selector();timer34_=200;elapsed38_=0;}
    }
    void close()noexcept { //004ACDC0 reverse records, retain list allocation and frame borrowers.
        while(!records_.empty())records_.pop_back();
    }
    void close_cold()noexcept{close();} //Existing containing source now retires actual populated records too.
    void draw(){ //004AD140 ascending inlineText14; root visibility is reread here.
        flush_retired();
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        for(auto& record:records_)factory_.hud_draw_text(*record->text);
    }
    template<class Human,class Items,class Icons>void populate(Human& human,Items& items,Icons& icons){ //004AC530.
        auto* storage=human.base().storage();
        if(!storage||storage->single())throw std::runtime_error("HUD selector requires actual Human storage");
        human_=&human;actor_identity_=&human.base(); //Actual selected-Human borrower, not a copied inventory.
        selected_=0;
        const auto holder=[&](unsigned n){auto* value=storage->holder(n);if(!value)throw std::runtime_error("HUD selector required holder is missing");return value->entry();};
        const auto main=holder(3);append_record(main,main?1u:0u,main?3u:7u,items,icons);
        if(const auto shoulder=holder(2))append_record(shoulder,1,2,items,icons);
        auto* holsters=storage->slots(1);if(!holsters)throw std::runtime_error("HUD selector requires actual holster stacks");
        for(std::size_t n=0;n<holsters->occupied_stacks();++n){const auto& stack=holsters->slots().at(n);
            if(stack.entries.empty())throw std::runtime_error("HUD selector encountered an empty occupied stack");
            const auto entry=stack.entries.front();auto& item=items.runtime_item(entry);
            if(static_cast<std::uint32_t>(item.family())==0||item.definition().kind()==13)
                append_record(entry,static_cast<std::uint32_t>(stack.entries.size()),1,items,icons);
        }
        require(selector_);frames_.object_visible(selector_,true);
    }
    template<class Human,class Items>void layout(Human& current_player,Items& items){ //004AC990→004ACB70.
        require(weapon00_);const auto world=frames_.actor_world_matrix(weapon00_);
        std::array<float,4> position{{world[12],world[13],world[14],world[15]}};
        for(std::size_t n=0;n<records_.size();++n){auto& record=*records_[n];const auto visual=record.visual->get(),caption=record.caption->get();
            frames_.object_visible(visual,true);frames_.hud_scroll_position(visual,position);
            if(n==selected_){
                require(selector_);frames_.link_scene_object(selector_,visual);frames_.update_scene_object(selector_);
                frames_.hud_info_scale(visual,1.1f,1.1f,1.1f);frames_.object_visible(caption,false);
            }else{frames_.hud_info_scale(visual,1.f,1.f,1.f);frames_.object_visible(caption,true);}
            frames_.update_scene_object(visual);
            position[1]=static_cast<float>(static_cast<long double>(position[1])-.045f);
        }
        refresh_amounts(current_player,items);
    }
    template<class Human,class Items>void refresh_amounts(Human& current_player,Items& items){ //004ACB70 uses ACTUAL current player, not a copied equipment list.
        for(auto& owner:records_){auto& record=*owner;bind_hud_text_geometry(frames_,*record.text,scene_,record.caption->get());
            if(!record.item)continue;
            auto& item=items.runtime_item(record.item);const auto kind=item.definition().kind();
            if(kind==0||kind==1)continue;
            std::array<char,128> buffer{};
            if(kind==9||kind==10||kind==11){
                std::snprintf(buffer.data(),buffer.size(),"%d",count_human_definition(current_player,items,static_cast<std::int32_t>(item.definition().identifier)));
            }else if(kind==13){
                const auto& payload=item.definition().payloads[0];
                if(payload.bytes.size()<12)throw std::runtime_error("HUD selector kind13 percentage has no original payload0C");
                const auto denominator=text_real(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+8));
                const auto percent=static_cast<float>(static_cast<long double>(item.auxiliary_amount())/denominator*100.f);
                std::snprintf(buffer.data(),buffer.size(),"%.0f/%d",static_cast<double>(percent),count_human_definition(current_player,items,static_cast<std::int32_t>(item.definition().identifier)));
            }else{
                const auto amount=text_ftol(item.amount());
                const auto reserve=count_human_ammunition(current_player,items,item.definition().ammunition(),kind==6);
                std::snprintf(buffer.data(),buffer.size(),"%d/%d",amount,reserve);
            }
            record.text->text.set(std::make_shared<const std::string>(buffer.data()),0x4b,0);
        }
    }
    void retire_actor(const void* identity)noexcept{if(identity&&actor_identity_==identity)retire_records();}
    void retire_item(Entry entry)noexcept{if(!entry)return;for(const auto& row:records_)if(row->item==entry){retire_records();return;}}
    void discard_borrowers()noexcept{scene_=root_=selector_=weapon00_=drkr_=Frame{};human_=actor_identity_=nullptr;retired_hide_=false;}
    Frame scene()const{return scene_;}Frame root()const{return root_;}Frame selector()const{return selector_;}
    Frame weapon00()const{return weapon00_;}Frame darkener()const{return drkr_;}const void* field28()const{return human_;}
    const auto& records()const{return records_;}
private:
    void retire_records()noexcept{human_=actor_identity_=nullptr;close();retired_hide_=true;} //Retract all loans before their real Human/Item can die.
    void flush_retired(){if(retired_hide_){require(root_);frames_.object_visible(root_,false);retired_hide_=false;}}
    static std::int32_t signed_word(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
    template<class Players,class Items>void layout_current(Players& players,Items& items){
        auto* current=players.current_player();if(!current)throw std::runtime_error("HUD selector layout requires actual current Human");
        (void)players.inline_control(*current);layout(*current,items);
    }
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD weapon-selector artwork is missing");}
    void hide(Frame frame){require(frame);frames_.object_visible(frame,false);}
    template<class Items,class Icons>void append_record(Entry entry,std::uint32_t count,std::uint32_t slot,Items& items,Icons& icons){
        //004AC190 constructs inlineText BEFORE storing item/count/slot or creating
        //either OBJ_. Failed-prefix native owners safely release what exists.
        auto record=std::make_unique<Record>();record->text=factory_.make_hud_text();
        if(!record->text)throw std::runtime_error("HUD selector text factory returned null");
        record->item=entry;record->quantity=count;record->slot=slot;
        require(weapon00_);record->visual.emplace(frames_.create_hud_inventory_visual());
        const auto visual=record->visual->get();require(visual);frames_.hud_inventory_duplicate(visual,weapon00_);
        if(entry){const auto material=materials_.material_for_replacement(visual,"i_frehnd.bmp");require(material);
            frames_.first_person_material_texture(material,icons.get(items.runtime_item(entry).definition()));}
        frames_.link_scene_object(visual,frames_.actor_parent(weapon00_));frames_.object_visible(visual,true);frames_.update_scene_object(visual);
        require(drkr_);record->caption.emplace(frames_.create_hud_inventory_visual());const auto caption=record->caption->get();require(caption);
        frames_.hud_inventory_duplicate(caption,drkr_);frames_.link_scene_object(caption,visual);frames_.object_visible(caption,true);frames_.update_scene_object(caption);
        bind_hud_text_geometry(frames_,*record->text,scene_,caption);record->text->text.inset=0; //AFTER geometry.
        records_.push_back(std::move(record));
    }
    Frames& frames_;Factory& factory_;HudMaterialLookup<Frames> materials_;Frame scene_{},root_{},selector_{},weapon00_{},drkr_{};
    const void* human_=nullptr;const void* actor_identity_=nullptr;bool retired_hide_=false;
    std::uint32_t selected_=0,elapsed38_=0,timer30_=0,timer34_=0;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
