#ifndef HD2_GAME_HUD_HANDS_LOADING_H
#define HD2_GAME_HUD_HANDS_LOADING_H
#include "game/hud_material_lookup.h"
#include "game/hud_text_loading.h"
#include "game/human_inventory_counts.h"
#include "game/text_glyphs.h"
#include <memory>
#include <cstdio>
#include "frontend/pre_mission_data.h"
#include <utility>
namespace hd2::game {
//004A9010/004A92F0: hands/shoulder artwork, retained original texture and shared
//text owner and004A9530 storage/seat updates. The actual occupied-Actor facade
//supplies mounted items; there is no replacement item/font/texture owner.
template<class Frames,class Factory>
class HudHandsLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using TextureOwner=decltype(std::declval<Frames&>().retain_hud_texture(std::declval<Frame>()));
    HudHandsLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory),lookup_(frames) {}
    ~HudHandsLoading(){close();}
    bool bind(Frame scene) {
        if(texture_||text_)throw std::logic_error("HUD hands rebound before teardown");
        require(scene);scene_=scene;root_=frames_.hud_find(scene,"tabl02",0xffff);require(root_);
        hands_=frames_.actor_find(root_,"i_hands",0xffff);shoulder_=frames_.actor_find(root_,"i_shoulder",0xffff);
        const auto texture=lookup_.texture(hands_,"X_SQUARE.BMP");require(texture);
        texture_=frames_.retain_hud_texture(texture);if(!texture_)throw std::runtime_error("HUD hands texture retain returned null");
        hands_material_=lookup_.material_for_replacement(hands_,"X_SQUARE.BMP");
        shoulder_material_=lookup_.material_for_replacement(shoulder_,"X_SQUARE.BMP");
        text_=factory_.make_hud_text();if(!text_)throw std::runtime_error("HUD hands text factory returned null");
        text_->text.inset=0;bind_hud_text_geometry(frames_,*text_,scene,root_);
        for(auto style:{0x36u,0x37u,0x38u,0x39u})text_->text.append(std::make_shared<const std::string>(),style);
        fields28_.fill(0xffffffffu);items1c_.fill(nullptr);return root_!=Frame{};
    }
    template<class Items>void set_item(unsigned side,const frontend::PreMissionItem* definition,Items& items){ //004A9320/004A93E0.
        if(side>1)throw std::out_of_range("HUD hands side");
        if(items1c_[side]==definition)return;
        items1c_[side]=definition; //Original stores BEFORE caption/texture queries, including failures.
        if(side==0){require_text();text_->text.set(definition?factory_.hud_localized(definition->caption):std::make_shared<const std::string>(" "),0x36,0);}
        const auto texture=definition?items.item_icon(*definition):texture_.get();
        if(texture){const auto material=side==0?hands_material_:shoulder_material_;require(material);frames_.first_person_material_texture(material,texture);}
    }
    void set_counts(std::uint32_t current,std::uint32_t reserve,unsigned side){ //004A9420 (native1=hand, other=shoulder).
        const unsigned first=side==1?0:1,second=side==1?2:3;
        if(fields28_[first]==current&&fields28_[second]==reserve)return;
        if(side==1){fields28_[first]=current;fields28_[second]=reserve;}
        else {fields28_[second]=reserve;fields28_[first]=current;}
        std::int32_t a,b;std::memcpy(&a,&current,4);std::memcpy(&b,&reserve,4);std::array<char,64> caption{};
        std::snprintf(caption.data(),caption.size(),"%d/%d",a,b);require_text();
        text_->text.set(std::make_shared<const std::string>(caption.data()),side==1?0x37:0x38,side==1?1:2);
    }
    template<class Players,class Items>void update(Players& players,Items& items,std::uint32_t now){ //004A9530, actual current Human and occupied-Actor storage.
        auto* human=players.current_player();if(!human)return;
        (void)players.inline_control(*human); //Validate the existing lifetime before actor/storage reads.
        const auto holder=[&](unsigned slot){auto* storage=human->base().storage();
            if(!storage||storage->single())throw std::runtime_error("HUD hands require actual Human storage");
            auto* value=storage->holder(slot);if(!value)throw std::runtime_error("HUD hands lost its actual holder");return value;};
        const auto publish=[&](auto entry,unsigned side){
            if(!entry){set_counts(0,0,side);return;}
            auto& item=items.runtime_item(entry);const auto& def=item.definition();
            if(side==1&&def.equipment_command()==6){set_counts(1,1,side);return;}
            const auto reserve=count_human_ammunition(*human,items,def.ammunition(),def.kind()==6);
            const auto amount=text_ftol(item.amount()); //Native holder virtual0C→entry amount, __ftol truncation.
            set_counts(static_cast<std::uint32_t>(amount),static_cast<std::uint32_t>(reserve),side);
        };
        if(human->field160&&players.seat_presentation().has_control(*human->field160,human->field164)){
            auto& seats=players.seat_presentation();auto& actor=*human->field160;const auto seat=human->field164;
            set_item(0,seats.definition(actor,seat),items);
            const auto reserve=seats.reserve(actor,seat); //Native A0 BEFORE9C; third004A9420 argument is1.
            const auto current=seats.current(actor,seat);
            set_counts(current,reserve,1);
        }else{
            auto* hand=holder(3);const auto entry=hand->entry();
            set_item(0,entry?&items.runtime_item(entry).definition():nullptr,items);publish(entry,1);
        }
        auto* shoulder=holder(2);const auto shoulder_entry=shoulder->entry();
        set_item(1,shoulder_entry?&items.runtime_item(shoulder_entry).definition():nullptr,items);
        if(shoulder_entry&&static_cast<std::uint32_t>(items.runtime_item(shoulder_entry).family())==0)publish(shoulder_entry,2);else set_counts(0,0,2);
        const auto count=count_human_kind9_holsters(*human,items,now);
        if(fields28_[4]!=count){fields28_[4]=count;require_text();text_->text.set(std::make_shared<const std::string>(std::to_string(signed_word(count))),0x39,3);}
    }
    void close()noexcept { //Original releases texture FIRST, then explicitly clears records before deleting text.
        texture_.reset();if(text_)text_->text.clear();text_.reset();
    }
    void draw(){ //004A9720 is unconditional; containing HUD supplies visibility policy.
        if(!text_)throw std::logic_error("HUD hands draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void refresh_geometry(){require_text();bind_hud_text_geometry(frames_,*text_,scene_,root_);} //004A9730.
    void discard_borrowers()noexcept{scene_=root_=hands_=shoulder_=hands_material_=shoulder_material_=Frame{};items1c_.fill(nullptr);}
    Frame scene()const{return scene_;}Frame root()const{return root_;}Frame hands()const{return hands_;}Frame shoulder()const{return shoulder_;}
    Frame original_texture()const{return texture_.get();}Frame hands_material()const{return hands_material_;}Frame shoulder_material()const{return shoulder_material_;}
    const auto& text()const{return text_;}const auto& fields28()const{return fields28_;}const auto& items1c()const{return items1c_;}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    void require_text(){if(!text_)throw std::logic_error("HUD hands update before text initialization");}
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD hands artwork/texture is missing");}
    Frames& frames_;Factory& factory_;HudMaterialLookup<Frames> lookup_;
    Frame scene_{},root_{},hands_{},shoulder_{},hands_material_{},shoulder_material_{};
    TextureOwner texture_{};std::unique_ptr<Text> text_;std::array<std::uint32_t,5> fields28_{};std::array<const frontend::PreMissionItem*,2> items1c_{};
};
} //namespace hd2::game
#endif
