#ifndef HD2_GAME_HUD_PANELS_LOADING_H
#define HD2_GAME_HUD_PANELS_LOADING_H
#include "game/hud_artwork_loading.h"
#include "game/hud_auxiliary_loading.h"
#include "game/hud_command_loading.h"
#include "game/hud_compass_loading.h"
#include "game/hud_crosshair_loading.h"
#include "game/hud_cursor_loading.h"
#include "game/hud_dialogs_loading.h"
#include "game/hud_explosive_timers_loading.h"
#include "game/hud_hands_loading.h"
#include "game/hud_info_loading.h"
#include "game/hud_injury_movies_loading.h"
#include "game/hud_inventory_loading.h"
#include "game/hud_map_loading.h"
#include "game/hud_messages_loading.h"
#include "game/hud_objective_icons_loading.h"
#include "game/hud_scoreboard_loading.h"
#include "game/hud_small_panels_loading.h"
#include "game/hud_soldier_loading.h"
#include "game/hud_player_loading.h"
#include "game/hud_weapon_selector_loading.h"
#include <type_traits>
namespace hd2::game {
//0049B730/0049B620/0049B280 and0049B490 source composition. World and
//player/item owners remain actual required inputs, not successful substitutes.
//This receiver binds no scene/model/camera itself and cannot publish readiness.
template<class Frames,class Factory,class Hud,class Players,class Items,class Styles,class Camera,class World>
class HudPanelsLoading {
public:
    using Frame=typename Frames::Frame;using Soldier=HudSoldierLoading<Frames,Factory>;
    using Dialogs=HudDialogsLoading<Frames,Factory,Hud>;
    using Actor=std::remove_pointer_t<std::decay_t<decltype(std::declval<World&>().hud_actor_identities()[0])>>;
    using Map=HudMapLoading<Frames,Factory,Actor,typename World::Objective>;
    struct Borrowers {void* inventory_context=nullptr;void* collision=nullptr;};
    HudPanelsLoading(Frames& frames,Factory& factory,Hud& hud,Players& players,Items& items,
                     Styles& styles,Camera& camera,World& world,HudSignalTables& signals,HudCompassShared& compass)
        :frames_(frames),factory_(factory),hud_(hud),players_(players),items_(items),world_(world),artwork_(frames),cursor_(frames,players,items),
         dialogs_(frames,factory,signals),command_roll_(frames,factory,signals),
         soldiers_{{{frames,factory},{frames,factory},{frames,factory},{frames,factory},{frames,factory},{frames,factory}}},
         messages_(frames,factory,hud.message_state()),crosshair_(frames,factory,factory),hands_(frames,factory),small_(frames,factory),
         compass_(frames,compass),auxiliary_(frames,factory),map_(frames,factory),inventory_(frames,factory,hud),
         info_(frames,factory,styles),selector_(frames,factory),movies_(frames),objectives_(frames),scoreboard_(frames,factory),
         timers_(frames,factory),command_(frames,factory,hud,command_roll_,camera){}
    void bind(Frame scene,const std::string& selected_mission,std::uint32_t mode,bool editor_kind3,const Borrowers& borrowers) {
        if(started_)throw std::logic_error("HUD panels rebound before retirement");
        started_=true;scene_=scene;
        artwork_.bind_actions(scene);artwork_.bind_player_states(scene);
        artwork_.bind_speed(scene);cursor_.bind(scene);dialogs_.bind_signals(hud_,scene,mode);dialogs_.bind_roll(hud_,scene,0);
        for(unsigned n=6;n;) {--n;soldiers_[n].bind(scene,n,artwork_,mode);}
        messages_.bind(scene,50);crosshair_.bind(scene);hands_.bind(scene);small_.bind_status(scene);compass_.bind(scene);
        auxiliary_.bind_pointer(scene,world_.scene_frame());
        (void)map_.bind(scene,selected_mission,borrowers.inventory_context,mode,factory_,world_,auxiliary_);
        inventory_.bind(scene,borrowers.inventory_context);
        progress_started_=true;auxiliary_.bind_progress(scene);info_.bind(scene,mode,factory_);selector_.bind(scene);
        movies_.bind(scene);small_.bind_hint(scene);small_.bind_camera(scene);objectives_.bind(scene);scoreboard_.bind(scene,mode,factory_);
        small_.bind_voting(scene);timers_.bind(scene,world_.hud_actor_identities());small_.bind_carnage(scene);small_.bind_server_message(scene);small_.bind_injury_message(scene);
        //Native6B6B0 is the just-bound CARNAGE root, not soldier mp_vip.
        const auto carnage=small_.carnage().visual;
        if(!carnage)throw std::runtime_error("Required HUD carnage artwork is missing");
        frames_.object_visible(carnage,(mode&7u)==7u&&editor_kind3);
        auxiliary_.bind_info(scene,artwork_);
        std::array<void*,5> command_contexts{{&hud_,const_cast<std::array<Frame,20>*>(&artwork_.first_states()),
            const_cast<std::array<Frame,39>*>(&artwork_.actions()),
            const_cast<typename HudAuxiliaryLoading<Frames,Factory>::Info*>(&auxiliary_.info()),borrowers.inventory_context}};
        command_started_=true;command_.bind(scene,world_.scene_frame(),borrowers.collision,command_contexts);
    }
    void close() {
        if(!started_)return;
        frontend::CleanupErrors errors;
        //004A1DA0/004A8DE0/004A8D50 are RETs: inline geometry/text owners
        //and cursor/speed borrowers survive until explicit containing retirement.
        dialogs_.close_signals();dialogs_.close_roll();
        for(auto& soldier:soldiers_)soldier.close();
        messages_.close();crosshair_.close();hands_.close();small_.close_status();compass_.close();map_.close();auxiliary_.close_info();
        if(command_started_)errors.attempt([&]{command_.close();});
        if(progress_started_)errors.attempt([&]{auxiliary_.close_progress();});
        auxiliary_.close_pointer();info_.close();errors.attempt([&]{selector_.close_cold();});errors.attempt([&]{movies_.clear();});
        small_.close_hint();small_.close_camera();objectives_.close();scoreboard_.close();small_.close_voting();timers_.close();
        small_.close_carnage();small_.close_server_message();small_.close_injury_message();
        started_=progress_started_=command_started_=false;errors.rethrow();
    }
    void post_bind(std::uint32_t mode,std::uint32_t flags) { //0049F1C0; current-player/action9 mappings must be real.
        bool visible=false;
        if((mode&7u)==7u&&!(flags&0x1a0u)&&!frames_.hud_frame_visible(inventory_.root())&&
                crosshair_.field14()!=1&&!command_.visible()) {
            if(auto* player=players_.current_player())visible=players_.action(*player,9,0)!=0;
        }
        compass_.show(visible);
    }
    template<class Human>bool add_human(Human* human,std::int32_t index,std::uint32_t mode,std::uint32_t difficulty,std::int32_t local_player){
        return add_hud_loading_human(frames_,hud_,soldiers_,compass_,players_,items_,human,index,mode,difficulty,local_player,[&]{post_bind(mode,hud_.flags);});
    }
    template<class Human>bool remove_human(Human* human,std::int32_t index){return remove_hud_loading_human(frames_,hud_,soldiers_,compass_,human,index);}
    template<class Human>void select_human(Human* human,bool selected,std::uint32_t mode,std::uint32_t difficulty){select_hud_loading_human(hud_,compass_,human,selected,mode,difficulty);}
    void inventory_changed(){ //0049FBF0; hidden inventory is a genuine RET, not a successful refresh substitute.
        const auto root=inventory_.root();if(!root)throw std::logic_error("Inventory notification requires its actual loaded HUD root");
        if(!frames_.hud_frame_visible(root))return;
        throw std::runtime_error("Visible inventory refresh requires actual004A49F0/004A26D0 actor/slot consumers");
    }
    bool hide_cold_soldiers(){return hide_hud_loading_humans(frames_,hud_,soldiers_,compass_);} //0049B900 also retires populated marker amounts.
    void discard_borrowers()noexcept {
        inventory_.retire_inline_scrolls();inventory_.discard_borrowers();cursor_.clear();dialogs_.discard_borrowers();
        for(auto& soldier:soldiers_)soldier.discard_borrowers();
        crosshair_.discard_borrowers();hands_.discard_borrowers();small_.discard_borrowers(); //Messages close already clears its visual.
        compass_.discard_borrowers();map_.discard_borrowers();auxiliary_.discard_borrowers();info_.discard_borrowers();selector_.discard_borrowers();
        movies_.discard_borrower();objectives_.discard_borrowers();scoreboard_.discard_borrowers();timers_.discard_borrowers();
        command_.discard_borrowers();artwork_.clear();scene_=Frame{};
    }
    auto& artwork(){return artwork_;}auto& compass(){return compass_;}auto& soldiers(){return soldiers_;}auto& command(){return command_;}
    auto& messages(){return messages_;}auto& crosshair(){return crosshair_;}auto& hands(){return hands_;}auto& small(){return small_;}
    auto& dialogs(){return dialogs_;}auto& timers(){return timers_;}auto& inventory(){return inventory_;}auto& map(){return map_;}
    auto& info(){return info_;}auto& selector(){return selector_;}auto& auxiliary(){return auxiliary_;}auto& scoreboard(){return scoreboard_;}
    template<class Direction>void hud_sound_direction(const Direction& direction,std::uint32_t kind){compass_.sound_direction(direction,kind,players_);} //0049DE80→004947E0.
private:
    Frames& frames_;Factory& factory_;Hud& hud_;Players& players_;Items& items_;World& world_;Frame scene_{};bool started_=false,progress_started_=false,command_started_=false;
    HudArtworkLoading<Frames> artwork_;HudCursorLoading<Frames,Players,Items> cursor_;Dialogs dialogs_,command_roll_;
    std::array<Soldier,6> soldiers_;HudMessagesLoading<Frames,Factory> messages_;HudCrosshairLoading<Frames,Factory,Factory> crosshair_;
    HudHandsLoading<Frames,Factory> hands_;HudSmallPanelsLoading<Frames,Factory> small_;HudCompassLoading<Frames> compass_;
    HudAuxiliaryLoading<Frames,Factory> auxiliary_;Map map_;HudInventoryLoading<Frames,Factory,Hud> inventory_;
    HudInfoLoading<Frames,Factory,Styles> info_;HudWeaponSelectorLoading<Frames,Factory> selector_;HudInjuryMoviesLoading<Frames> movies_;
    HudObjectiveIconsLoading<Frames> objectives_;HudScoreboardLoading<Frames,Factory> scoreboard_;HudExplosiveTimersLoading<Frames,Factory,Actor> timers_;
    HudCommandLoading<Frames,Factory,Hud,Dialogs,Camera> command_;
};
} //namespace hd2::game
#endif
