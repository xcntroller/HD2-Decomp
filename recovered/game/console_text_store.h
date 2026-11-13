#ifndef HD2_GAME_CONSOLE_TEXT_STORE_H
#define HD2_GAME_CONSOLE_TEXT_STORE_H
#include <cstdint>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
namespace hd2::game {
//00744340/00744870/00744800/00744A00/007447B0: SAME console output and
//submitted-input owners. Output is newest first/capped100; input is FIFO and
//unlocked, as in the native main-window/main-frame path. STL owns every string.
template<class Backend>class ConsoleTextStore {
public:
    explicit ConsoleTextStore(Backend& backend):backend_(backend){}
    void append(const std::string& text){
        std::string owned(text.c_str()); //Copy BEFORE48 lock, native strlen/strcpy.
        {
            Lock lock(backend_);lines_.push_front(std::move(owned));
            if(lines_.size()>100)lines_.pop_back();
        }
        backend_.invalidate_console(); //AFTER unlock.
    }
    void character(std::uint32_t word){
        const auto value=static_cast<std::uint8_t>(word);
        if(value==13){submit();return;}
        if(value==8){
            if(input_.empty())throw std::out_of_range("Native console backspace would write before its input buffer");
            input_.pop_back();backend_.redraw_console_input();return;
        }
        if(input_.size()<50){if(value)input_+=static_cast<char>(value);backend_.redraw_console_input();}
    }
    void submit(){ //00744A00: queue owns prior input, then output copy, then new prompt.
        commands_.push_front(input_);append(input_);input_=">";
    }
    std::optional<std::string> take_command(){ //007447B0, transfers oldest command to caller.
        if(commands_.empty())return std::nullopt;
        auto result=std::move(commands_.back());commands_.pop_back();return result;
    }
    template<class Draw>void draw_lines(Draw draw){Lock lock(backend_);for(const auto& line:lines_)draw(line);}
    const std::string& input()const{return input_;}
    std::size_t line_count()const{return lines_.size();}std::size_t command_count()const{return commands_.size();}
private:
    struct Lock{Backend& backend;explicit Lock(Backend& b):backend(b){backend.enter_console_lock();}~Lock(){backend.leave_console_lock();}};
    Backend& backend_;std::list<std::string> lines_,commands_;std::string input_=">"; //0083686C.
};
}
#endif
