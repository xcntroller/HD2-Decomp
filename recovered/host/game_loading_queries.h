#ifndef HD2_HOST_GAME_LOADING_QUERIES_H
#define HD2_HOST_GAME_LOADING_QUERIES_H

#include "host/graphics_interface.h"
#include <cstring>
#include <stdexcept>

namespace hd2::host {

//Resolved real Win32 payloads of006CA780/006989B0. Window-bearing calls ask
//actual GetIGraph/virtual48 afresh. No private integrity VM/table side effect
//is inferred from these API results, and no cached window/readiness is owned.
class GameLoadingQueries {
public:
    explicit GameLoadingQueries(HMODULE module){
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
    HWND query_parent_window(){return query_parent_window(window_argument());}
    BOOL query_visible_window(HWND argument){return IsWindowVisible(argument);} //006CAAC0.
    BOOL query_visible_window(){return query_visible_window(window_argument());}
    BOOL query_iconic_window(HWND argument){return IsIconic(argument);} //006CAB29.
    BOOL query_iconic_window(){return query_iconic_window(window_argument());}
    DWORD query_activation_ticks(){return GetTickCount();} //00698AAE, also006E9EC2 initializer seed API.
    BOOL query_activation_visible_window(HWND argument){return IsWindowVisible(argument);} //00698C09.
    BOOL query_activation_visible_window(){return query_activation_visible_window(window_argument());}
    HWND query_finish_focus(){return GetFocus();} //00699112, actual shutdown payload.
    HWND query_activation_capture(){return GetCapture();} //00698C5D, no graphics/window query.
private:
    HWND window(){
        auto* graphics=get_graphics_();if(!graphics)throw std::runtime_error("Loading query requires actual graphics");
        return graphics->methods->window(graphics);
    }
    Graphics*(__cdecl *get_graphics_)()=nullptr;
};

} //namespace hd2::host
#endif
