#ifndef HD2_HOST_GAME_HUMAN_SIGNAL_COMMANDS_H
#define HD2_HOST_GAME_HUMAN_SIGNAL_COMMANDS_H
#include "host/game_ui_presentation.h"
namespace hd2::host {
//Actual UI005A45E0→Player0053D910 producer on the SAME World services. This
//borrows that World; its installation in the application belongs to FRAME/OWN.
class GameHumanSignalCommands final:public GameUiSignalCommands {
public:
    explicit GameHumanSignalCommands(world_loading::Services& services):services_(services){}
    void submit(world_loading::Human& human,const game::HudSignalCommand<GameSceneActors::Instance>& request)override{services_.queue_player_signal(human,request);}
private:world_loading::Services& services_;
};
}
#endif
