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
//Interactive weapon transfer/selection timing remain separate consumers.
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
        hide(root_);human_=nullptr;
    }
    void close()noexcept { //004ACDC0 reverse records, retain list allocation and frame borrowers.
        while(!records_.empty())records_.pop_back();
    }
    void close_cold()noexcept{close();} //Existing containing source now retires actual populated records too.
    void draw(){ //004AD140 ascending inlineText14; root visibility is reread here.
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        for(auto& record:records_)factory_.hud_draw_text(*record->text);
    }
    template<class Human,class Items,class Icons>void populate(Human& human,Items& items,Icons& icons){ //004AC530.
        auto* storage=human.base().storage();
        if(!storage||storage->single())throw std::runtime_error("HUD selector requires actual Human storage");
        human_=&human; //Actual selected-Human borrower supplied by containing004ACE20, not a copied inventory.
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
    void discard_borrowers()noexcept{scene_=root_=selector_=weapon00_=drkr_=Frame{};human_=nullptr;}
    Frame scene()const{return scene_;}Frame root()const{return root_;}Frame selector()const{return selector_;}
    Frame weapon00()const{return weapon00_;}Frame darkener()const{return drkr_;}const void* field28()const{return human_;}
    const auto& records()const{return records_;}
private:
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
    const void* human_=nullptr;std::uint32_t selected_=0;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
