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
    HWND query_parent_window(){return GetParent(window());} //006CA913.
    BOOL query_visible_window(){return IsWindowVisible(window());} //006CAAC0.
    BOOL query_iconic_window(){return IsIconic(window());} //006CAB29.
    DWORD query_activation_ticks(){return GetTickCount();} //00698AAE, also006E9EC2 initializer seed API.
    BOOL query_activation_visible_window(){return IsWindowVisible(window());} //00698C09.
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
