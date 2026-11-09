#ifndef HD2_GAME_HUD_HANDS_LOADING_H
#define HD2_GAME_HUD_HANDS_LOADING_H
#include "game/hud_material_lookup.h"
#include "game/hud_text_loading.h"
#include <memory>
#include <utility>
namespace hd2::game {
//004A9010/004A92F0: hands/shoulder artwork, retained original texture and shared
//text owner. Runtime item updates004A9320+ remain distinct required consumers.
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
    void close()noexcept { //Original releases texture FIRST, then explicitly clears records before deleting text.
        texture_.reset();if(text_)text_->text.clear();text_.reset();
    }
    void draw(){ //004A9720 is unconditional; containing HUD supplies visibility policy.
        if(!text_)throw std::logic_error("HUD hands draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void discard_borrowers()noexcept{scene_=root_=hands_=shoulder_=hands_material_=shoulder_material_=Frame{};items1c_.fill(nullptr);}
    Frame scene()const{return scene_;}Frame root()const{return root_;}Frame hands()const{return hands_;}Frame shoulder()const{return shoulder_;}
    Frame original_texture()const{return texture_.get();}Frame hands_material()const{return hands_material_;}Frame shoulder_material()const{return shoulder_material_;}
    const auto& text()const{return text_;}const auto& fields28()const{return fields28_;}const auto& items1c()const{return items1c_;}
private:
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD hands artwork/texture is missing");}
    Frames& frames_;Factory& factory_;HudMaterialLookup<Frames> lookup_;
    Frame scene_{},root_{},hands_{},shoulder_{},hands_material_{},shoulder_material_{};
    TextureOwner texture_{};std::unique_ptr<Text> text_;std::array<std::uint32_t,5> fields28_{};std::array<void*,2> items1c_{};
};
} //namespace hd2::game
#endif
