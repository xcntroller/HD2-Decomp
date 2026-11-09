#ifndef HD2_GAME_HUD_OBJECTIVE_ICONS_LOADING_H
#define HD2_GAME_HUD_OBJECTIVE_ICONS_LOADING_H
#include <array>
#include <cstdio>
#include <stdexcept>
namespace hd2::game {
//004AD8F0/004AD9B0/004AD9A0: this artwork binder is unconditional HUD loading,
//including ordinary missions. It does not fabricate network objectives.
template<class Frames>
class HudObjectiveIconsLoading {
public:
    using Frame=typename Frames::Frame;
    explicit HudObjectiveIconsLoading(Frames& frames):frames_(frames) {}
    void bind(Frame scene) {
        if(!scene)throw std::runtime_error("HUD objective icons require actual scene");
        root_=frames_.hud_find(scene,"mpobjectives",0x20);
        constexpr std::array<const char*,3> names{{"mpallied_obj%02u","mpaxis_obj%02u","mpneutral_obj%02u"}};
        for(unsigned i=0;i!=icons_.size();++i) {
            for(unsigned kind=0;kind!=3;++kind) {
                std::array<char,32> name{};std::snprintf(name.data(),name.size(),names[kind],i);
                icons_[i][kind]=frames_.hud_find(scene,name.data(),1);
            }
            select(i,-1);
        }
    }
    void select(unsigned index,std::int32_t kind) {
        if(index>=icons_.size())throw std::out_of_range("HUD objective icon index");
        auto& row=icons_[index];
        for(unsigned i=0;i!=3;++i){require(row[i]);frames_.object_visible(row[i],kind==(i==0?1:i==1?0:2));}
        for(auto frame:row)frames_.update_scene_object(frame); //All three update even when already hidden.
    }
    void close()noexcept{} //004AD9A0 is an actual RET.
    void discard_borrowers()noexcept{root_=Frame{};for(auto& row:icons_)row.fill(Frame{});}
    Frame root()const{return root_;}const auto& icons()const{return icons_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD objective icon artwork is missing");}
    Frames& frames_;Frame root_{};std::array<std::array<Frame,3>,16> icons_{};
};
} //namespace hd2::game
#endif
