#ifndef HD2_GAME_SESSION_MATCH_APPLICATION_H
#define HD2_GAME_SESSION_MATCH_APPLICATION_H
#include "game/session_activation.h"
#include "game/world_loading_dependency_link.h"
#include <optional>
namespace hd2::game {
//Actual process/controller current-state/menu owner, including BEFORE World
//creation. OWN must bind its real retaining menu implementation; no default
//success, copied state/type or Frame-as-application lifetime substitution.
class SessionMatchApplication {
public:
    virtual ~SessionMatchApplication()=default;
    virtual const void* process_identity()const noexcept=0;
    virtual std::optional<std::uint32_t> current_kind()=0;
    virtual void install_retaining_game_menu(std::int32_t)=0; //0069E620(current state): retain game BEFORE replacing controller14.
    virtual void open_original_game_menu(SessionActivationState&,std::int32_t)=0; //00699900's SECOND0069E620, original game receiver, ignored result still owned/retired.
    virtual void current_menu_respawn_pending(bool)=0; //Fresh actual kind11 field1D1.
    virtual void change_current_menu_team(std::uint32_t)=0; //00685260 actual menu owner; no new function credit for this declaration.
};
class SessionMatchApplicationLink {
public:
    explicit SessionMatchApplicationLink(const void* process):process_(process){if(!process_)throw std::invalid_argument("Match application requires its actual process");}
    void bind(SessionMatchApplication& owner){if(owner.process_identity()!=process_)throw std::logic_error("Match application belongs to a different process");owner_.bind(owner);}
    void unbind(SessionMatchApplication& owner)noexcept{owner_.unbind(owner);}
    SessionMatchApplication& get()const{return owner_.get();}
private:
    const void* process_;WorldLoadingDependencyLink<SessionMatchApplication> owner_;
};
}
#endif
