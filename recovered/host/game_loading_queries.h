#ifndef HD2_HOST_GAME_LOADING_QUERIES_H
#define HD2_HOST_GAME_LOADING_QUERIES_H

#include "host/graphics_interface.h"
#include "host/startup_session.h"
#include "game/steam_integrity_calls.h"
#include <cstring>
#include <stdexcept>

namespace hd2::host {

//Resolved real Win32 payloads of006CA780/006989B0. Window-bearing calls ask
//actual GetIGraph/virtual48 afresh. Raw payloads use SAME Startup tables/threads;
//Steam descriptor actions publish observations with validation explicitly skipped;
//no cached window/readiness or successful retail validator is owned.
class GameLoadingQueries {
public:
    GameLoadingQueries(HMODULE module,StartupSession& startup):startup_(startup){
        const auto entry=module?GetProcAddress(module,"GetIGraph"):nullptr;
        if(!entry||reinterpret_cast<std::uintptr_t>(entry)!=reinterpret_cast<std::uintptr_t>(module)+0x68f90)
            throw std::runtime_error("Unsupported loading GetIGraph export");
        static_assert(sizeof(get_graphics_)==sizeof(entry));std::memcpy(&get_graphics_,&entry,sizeof(entry));
    }
    //Resolve GetIGraph/virtual48 BEFORE entering the wrapped-call validation.
    //The original caller stores this argument in8CAC30 before the action and
    //overwrites that payload with the raw API return afterward.
    HWND window_argument(){return window();}
    HWND query_parent_window(HWND argument){return GetParent(argument);} //006CA913.
    HWND query_parent_window(){return handle(window_call({117,0x6ca913,0x075ac025},[&](HWND value){return word(query_parent_window(value));}));}
    BOOL query_visible_window(HWND argument){return IsWindowVisible(argument);} //006CAAC0.
    BOOL query_visible_window(){return static_cast<BOOL>(window_call({34,0x6caac0,0x0320eba0},[&](HWND value){return word(query_visible_window(value));}));}
    BOOL query_iconic_window(HWND argument){return IsIconic(argument);} //006CAB29.
    BOOL query_iconic_window(){return static_cast<BOOL>(window_call({39,0x6cab29,0x097a5e32},[&](HWND value){return word(query_iconic_window(value));}));}
    DWORD query_activation_ticks(){return game::steam_integrity_marker_call(startup_,{77,0x698aae,0x0437e723,0x923a1,true},[]{return GetTickCount();}).returned;} //00698AAE, also006E9EC2 initializer seed API.
    BOOL query_activation_visible_window(HWND argument){return IsWindowVisible(argument);} //00698C09.
    BOOL query_activation_visible_window(){return static_cast<BOOL>(window_call({13,0x698c09,0x075ac025},[&](HWND value){return word(query_activation_visible_window(value));}));}
    HWND query_finish_focus(){return handle(game::steam_integrity_marker_call(startup_,{100,0x699112,0x0437e723,0x923a1,true},[]{return word(GetFocus());}).returned);} //00699112, actual shutdown payload.
    HWND query_activation_capture(){return handle(game::steam_integrity_marker_call(startup_,{5,0x698c5d,0x0437e723,0x923a1,true},[]{return word(GetCapture());}).returned);} //00698C5D, no graphics/window query.
private:
    static std::uint32_t word(HWND value){return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));}
    static std::uint32_t word(BOOL value){return static_cast<std::uint32_t>(value);}
    static HWND handle(std::uint32_t value){return reinterpret_cast<HWND>(static_cast<std::uintptr_t>(value));}
    template<class Api>std::uint32_t window_call(game::SteamIntegritySite site,Api api){
        return game::steam_integrity_window_call(startup_,site,[&]{
            const auto value=window_argument();return game::IntegrityApiArgument<HWND>{value,word(value)};
        },api).returned;
    }
    StartupSession& startup_;
    HWND window(){
        auto* graphics=get_graphics_();if(!graphics)throw std::runtime_error("Loading query requires actual graphics");
        return graphics->methods->window(graphics);
    }
    Graphics*(__cdecl *get_graphics_)()=nullptr;
};

} //namespace hd2::host
#endif
