#ifndef HD2_GAME_HUD_HELP_BINDINGS_H
#define HD2_GAME_HUD_HELP_BINDINGS_H
#include <array>
#include <cstdint>
#include <optional>
#include <string>
namespace hd2::game {
//006AC1C0/006ABE80, original ordered table00842500. First TABLE match wins,
//not the first token in the input. Binding names come from the actual profile.
struct HudHelpBinding {const char* token;std::int32_t action;std::uint32_t mode;};
inline constexpr std::array<HudHelpBinding,36> hud_help_bindings{{
    {"<Move_forward>",0,2},
    {"<Move_back>",1,2},
    {"<Strafe_left>",2,2},
    {"<Strafe_right>",3,2},
    {"<Increase_speed>",5,2},
    {"<Decrease_speed>",4,2},
    {"<Lean_left>",10,2},
    {"<Lean_right>",11,2},
    {"<Jump_key>",12,2},
    {"<Climb>",17,2},
    {"<Crouch>",8,2},
    {"<Lie_down>",9,2},
    {"<Primary_fire>",6,2},
    {"<Secondary_action>",7,2},
    {"<Throw_grenade>",16,2},
    {"<Reload>",13,2},
    {"<Previous_weapon>",32,2},
    {"<Next_weapon>",33,2},
    {"<Drop_item>",34,2},
    {"<Action_button>",19,1},
    {"<Action_pull_down_menu>",20,1},
    {"<GameInfo>",21,1},
    {"<Tactical_map>",22,1},
    {"<Interface_mode>",23,1},
    {"<Map>",24,1},
    {"<Inventory_key>",25,1},
    {"<Pause>",31,1},
    {"<Binoculars>",15,2},
    {"<Camera>",26,1},
    {"<Console>",27,1},
    {"<Hand_brake>",18,2},
    {"<SwitchPlayer>",30,1},
    {"<Select_soldier_1>",35,1},
    {"<Select_soldier_2>",36,1},
    {"<Select_soldier_3>",37,1},
    {"<Select_soldier_4>",38,1},
}};
template<class Settings>std::optional<std::size_t> find_hud_help_binding(const Settings& settings,std::int32_t action,std::uint32_t mode){ //006A8360.
    for(std::size_t n=0;n<settings.size();++n)if(settings[n].index==action&&settings[n].mode==mode)return n;
    return std::nullopt;
}
template<class Name>std::optional<std::string> expand_hud_help_binding(const std::string& input,Name&& name){
    for(const auto& entry:hud_help_bindings){const auto at=input.find(entry.token);if(at==std::string::npos)continue;
        const auto end=at+std::char_traits<char>::length(entry.token);const auto label=name(entry.action,entry.mode);
        if(at==0)return "'"+label+"' "+input.substr(end);
        const auto prefix=input.substr(0,at<1023?at:1023);
        if(end==input.size())return prefix+" '"+label+"'";
        return prefix+" '"+label+"' "+input.substr(end);
    }
    return std::nullopt; //Native AL=false; caller also copies input via00405180.
}
} //namespace hd2::game
#endif
