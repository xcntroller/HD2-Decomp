#ifndef HD2_HOST_GAME_CONSOLE_H
#define HD2_HOST_GAME_CONSOLE_H
#include "game/console_text_store.h"
#include <windows.h>
#include <array>
#include <algorithm>
#include <exception>
#include <functional>
#include <optional>

namespace hd2::host {
//0069F1E0 lazily publishes ONE actual console before00744C60 creates its HWND:
//CreateWindow's callbacks may consult that published owner. This class uses
//typed native ownership, not original addresses/object ABI or original code.
class GameConsole {
public:
    explicit GameConsole(std::function<void()> close):text_(*this),close_(std::move(close)){
        if(active_)throw std::logic_error("Another process console owner is already published");
        InitializeCriticalSection(&lock_);active_=this;
    }
    ~GameConsole(){
        //Source RAII also releases the acquired HWND/font prefix, which native
        //00744490 does not destroy. Stop callbacks BEFORE retiring48/text owners.
        if(window_)DestroyWindow(window_);
        if(font_)DeleteObject(font_);
        if(active_==this)active_=nullptr;
        DeleteCriticalSection(&lock_);
    }
    GameConsole(const GameConsole&)=delete;GameConsole& operator=(const GameConsole&)=delete;
    void open(){ //00744C60, actual Win32 calls only. No window/graphics stand-in.
        if(attempted_){rethrow_failure();return;}attempted_=true;
        try{
            WNDCLASSEXA type{};type.cbSize=sizeof(type);type.style=0xb;type.lpfnWndProc=&procedure;
            type.hIcon=LoadIconA(nullptr,MAKEINTRESOURCEA(0x7f00));type.hCursor=LoadCursorA(nullptr,MAKEINTRESOURCEA(0x7f00));
            type.hbrBackground=static_cast<HBRUSH>(GetStockObject(1));type.lpszClassName="WndConsole";type.hIconSm=LoadIconA(nullptr,MAKEINTRESOURCEA(0x7f00));
            const auto registered=RegisterClassExA(&type);
            if(!registered&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)throw std::runtime_error("Cannot register original console window class");
            window_=CreateWindowExA(0,"WndConsole","Hidden & Dangerous II Console",0x10ee000d,0,0,480,360,nullptr,nullptr,nullptr,nullptr);
            if(!window_)throw std::runtime_error("Cannot create original console window");
            ShowWindow(window_,5);
            const auto dc=GetDC(window_);if(!dc)throw std::runtime_error("Console font requires actual window DC");
            const auto height=-MulDiv(10,GetDeviceCaps(dc,90),72);
            font_=CreateFontA(height,0,0,0,700,0,0,0,0,0,0,0,0,"Courier");ReleaseDC(window_,dc);
            if(!font_)throw std::runtime_error("Cannot create original console font");
            rethrow_failure();
        }catch(...){if(!failure_)failure_=std::current_exception();throw;}
    }
    void append(const std::string& value){open();text_.append(value);rethrow_failure();}
    auto take_command(){open();return text_.take_command();}
    void rethrow_failure()const{if(failure_)std::rethrow_exception(failure_);}
    void enter_console_lock(){EnterCriticalSection(&lock_);}void leave_console_lock()noexcept{LeaveCriticalSection(&lock_);}
    void invalidate_console(){InvalidateRect(window_,nullptr,TRUE);}
    void redraw_console_input(){ //00744A70, original511-byte space padding plus terminator.
        if(!rectangle_)throw std::logic_error("Console input redraw precedes actual paint rectangle");
        const auto dc=GetDC(window_);if(!dc)throw std::runtime_error("Console input requires actual window DC");
        SelectObject(dc,font_);auto rect=input_rectangle(*rectangle_);
        std::array<char,512> line;line.fill(' ');std::copy(text_.input().begin(),text_.input().end(),line.begin());line.back()=0;
        SetBkColor(dc,0xc0c0c0);DrawTextA(dc,line.data(),-1,&rect,0);ReleaseDC(window_,dc);
        //Native callback vector64 is initially empty. Its separate producer is
        //not reconstructed or exposed as an invented successful callback list.
    }
private:
    RECT input_rectangle(RECT rect)const{rect.top=rect.bottom*2-offset28_-17;return rect;}
    void paint(HDC dc,const RECT& bounds){ //00744B80, newest output ascends above current prompt.
        rectangle_=bounds;SelectObject(dc,font_);SetBkMode(dc,1);auto rect=input_rectangle(bounds);
        DrawTextA(dc,text_.input().c_str(),-1,&rect,0);rect.top-=5;
        text_.draw_lines([&](const std::string& line){rect.top-=12;DrawTextA(dc,line.c_str(),-1,&rect,0);});
    }
    static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM first,LPARAM second)noexcept{
        auto* owner=active_;if(!owner)return DefWindowProcA(window,message,first,second);
        try{return owner->message(window,message,first,second);}
        catch(...){if(!owner->failure_)owner->failure_=std::current_exception();return 0;}
    }
    LRESULT message(HWND window,UINT kind,WPARAM first,LPARAM second){ //00744640.
        switch(kind){
        case WM_CREATE:return 0;
        case WM_PAINT:{
            PAINTSTRUCT paint_state{};const auto dc=BeginPaint(window,&paint_state);RECT bounds{};GetClientRect(window,&bounds);
            try{paint(dc,bounds);}catch(...){EndPaint(window,&paint_state);throw;}
            EndPaint(window,&paint_state);return 0;
        }
        case WM_SIZE:{
            const auto height=static_cast<unsigned>(HIWORD(second));SetScrollRange(window,1,0,height,FALSE);
            offset28_=static_cast<std::int32_t>(height);SetScrollPos(window,1,height,TRUE);return DefWindowProcA(window,kind,first,second);
        }
        case WM_VSCROLL:
            if(LOWORD(first)==4||LOWORD(first)==5){offset28_=static_cast<std::int16_t>(HIWORD(first));SetScrollPos(window,1,offset28_,TRUE);InvalidateRect(window,nullptr,TRUE);}return 0;
        case WM_CHAR:text_.character(static_cast<std::uint32_t>(first));return 0;
        case WM_CLOSE:if(!close_)throw std::logic_error("Console close lost its application-state owner");close_();return 0; //Status4, NOT DestroyWindow/PostQuitMessage.
        default:return DefWindowProcA(window,kind,first,second);
        }
    }
    CRITICAL_SECTION lock_{};game::ConsoleTextStore<GameConsole> text_;std::function<void()> close_;
    HWND window_=nullptr;HFONT font_=nullptr;std::optional<RECT> rectangle_;std::int32_t offset28_=0;
    bool attempted_=false;std::exception_ptr failure_;inline static GameConsole* active_=nullptr;
};
}
#endif
