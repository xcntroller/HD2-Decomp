#ifndef HD2_GAME_HUD_DIALOGS_LOADING_H
#define HD2_GAME_HUD_DIALOGS_LOADING_H
#include "game/hud_signal_tables.h"
#include "game/hud_text_loading.h"
#include <memory>
#include <stdexcept>
namespace hd2::game {
//004B0750/004B0900 roll menu and004BF2E0/004BF480 signals. HUD scene,
//actual HUD identity, font/localization factory and PROCESS signal tables are
//borrowed. No menu population, command dispatch or rendering is supplied.
template<class Frames,class Factory,class Hud>
class HudDialogsLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    struct Roll {Frame scene{};Hud* hud=nullptr;Frame root{},cursor{};std::unique_ptr<Text> text;
        std::array<float,4> position20{};std::uint8_t flag30=0,flag31=0;const void* field88=nullptr;}; //Actual borrowed actor88.
    struct Signals {Hud* hud=nullptr;Frame root{},scene{};std::uint32_t field0c=50;
        std::array<std::uint32_t,3> fields60{};std::uint8_t flag6c=1;std::unique_ptr<Text> text;};
    HudDialogsLoading(Frames& frames,Factory& factory,HudSignalTables& tables):frames_(frames),factory_(factory),tables_(tables) {}
    void bind_roll(Hud& hud,Frame scene,std::uint8_t flag) { //Original RET12, third BYTE argument supplies31.
        bind_roll(&hud,scene,flag);
    }
    void bind_roll(Hud* hud,Frame scene,std::uint8_t flag) { //004B4980's nested roll explicitly passes a null HUD.
        cold(roll_.text);require(scene);roll_.scene=scene;roll_.hud=hud;
        roll_.root=frames_.hud_find(scene,"rollmenu",0xffff);require(roll_.root);
        roll_.position20=frames_.actor_position(roll_.root); //All FOUR nativeB0 words, no world-matrix update.
        roll_.cursor=frames_.hud_find(scene,"kurzor",0xffff);frames_.object_visible(roll_.root,false);
        roll_.text=make_text();bind_hud_text_geometry(frames_,*roll_.text,scene,roll_.root);
        roll_.field88=nullptr;roll_.flag31=flag;roll_.flag30=0;
    }
    bool bind_signals(Hud& hud,Frame scene,std::uint32_t mode) { //004BF2E0 RET8, final table publish only after geometry.
        cold(signals_.text);require(scene);signals_.scene=scene;
        signals_.root=frames_.hud_find(scene,"signals",0xffff);signals_.hud=&hud;
        if(signals_.root)frames_.object_visible(signals_.root,false);
        signals_.field0c=50;signals_.text=make_text();
        bind_hud_text_geometry(frames_,*signals_.text,scene,signals_.root);
        signals_.fields60.fill(0);signals_.flag6c=1;tables_.select(mode);
        return signals_.root!=Frame{};
    }
    void close_roll()noexcept {if(roll_.text)roll_.text->text.clear();roll_.text.reset();} //004B0900 clear then release.
    void draw_roll(){ //004B2FA0 checks ROOT0C; the caller separately checks flag30.
        require(roll_.root);if(!frames_.hud_frame_visible(roll_.root))return;
        if(!roll_.text)throw std::logic_error("HUD roll draw before text initialization");
        factory_.hud_draw_text(*roll_.text);
    }
    bool signals_active(){require(signals_.root);return frames_.hud_frame_visible(signals_.root);} //004C05F0.
    void draw_signals(){ //004C0880 rereads the SAME root visibility before text70.
        require(signals_.root);if(!frames_.hud_frame_visible(signals_.root))return;
        if(!signals_.text)throw std::logic_error("HUD signals draw before text initialization");
        factory_.hud_draw_text(*signals_.text);
    }
    void close_signals()noexcept {if(signals_.text)signals_.text->text.clear();signals_.text.reset();} //004BF480; active process table stays.
    void retire_roll_actor(const void* actor,bool reset_signals) { //0049EF70's004B0920/004C11C0 consumers.
        if(roll_.field88!=actor)return;
        (void)reset_signals;
        //004B0750 initializes88=null. No populated roll/menu producer is
        //offered by this loading owner. Never pretend to clear its positive
        //004B0920 target/rebuild or004C11C0 signal rows successfully.
        throw std::runtime_error("Populated HUD roll retirement requires its actual target/menu receivers");
    }
    void discard_borrowers()noexcept {
        roll_.scene=roll_.root=roll_.cursor=Frame{};roll_.hud=nullptr;
        signals_.root=signals_.scene=Frame{};signals_.hud=nullptr;
        roll_.field88=nullptr;
    }
    const auto& roll()const{return roll_;}const auto& signals()const{return signals_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD dialog artwork is missing");}
    static void cold(const std::unique_ptr<Text>& owner){if(owner)throw std::logic_error("HUD dialog rebound before teardown");}
    std::unique_ptr<Text> make_text(){auto result=factory_.make_hud_text();if(!result)throw std::runtime_error("HUD dialog text factory returned null");return result;}
    Frames& frames_;Factory& factory_;HudSignalTables& tables_;Roll roll_;Signals signals_;
};
} //namespace hd2::game
#endif
