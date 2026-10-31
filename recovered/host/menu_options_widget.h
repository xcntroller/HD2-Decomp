#ifndef HD2_HOST_MENU_OPTIONS_WIDGET_H
#define HD2_HOST_MENU_OPTIONS_WIDGET_H

#include "frontend/built_options_widget.h"
#include "frontend/cursor_frame.h"
#include "frontend/menu_event_queue.h"
#include "host/menu_pointer_session.h"
#include "host/menu_text_object.h"

namespace hd2::host {

class MenuWidgetVisuals;
using MenuOptionsWidget = frontend::BuiltOptionsWidget<MenuWidgetVisuals>;

// Shared input/listener/preference bindings. This is owned by the menu, with
// products destroyed first. No original client addresses become callable code.
class MenuWidgetContext {
    struct Update final : frontend::FrontendUpdateTarget {
        std::function<void(std::uint32_t)> callback;
        explicit Update(std::function<void(std::uint32_t)> value) : callback(std::move(value)) {}
        void update(std::uint32_t milliseconds) override { const auto current=callback; if (current) current(milliseconds); }
    };
public:
    template<class Settings>
    MenuWidgetContext(MenuInputSession& input,frontend::MenuEventQueue& queue,Settings& settings);
    ~MenuWidgetContext() {
        auto& state=queue.common_state();
        for (auto token : tokens_) {
            const auto found=state.notification_targets.find(token);
            if (found != state.notification_targets.end()) {
                if (auto target=std::dynamic_pointer_cast<Update>(found->second)) target->callback={};
                state.notification_targets.erase(found);
            }
            auto& tokens=state.notification_tokens;
            tokens.erase(std::remove(tokens.begin(),tokens.end(),token),tokens.end());
        }
    }
    std::function<void()> subscribe(std::uint32_t token,std::function<void(std::uint32_t)> callback) {
        auto& targets=queue.common_state().notification_targets;
        if (auto old=targets.find(token);old != targets.end()) {
            auto target=std::dynamic_pointer_cast<Update>(old->second);
            if (!target || target->callback) throw std::logic_error("Duplicate widget listener identity");
        }
        auto target=std::make_shared<Update>(std::move(callback));
        tokens_.push_back(token); targets[token]=target;
        queue.enqueue({0,0x0200007a,0,token,0,{}}); // Original queued registration; updates occur after drain.
        return [weak=std::weak_ptr<Update>(target)] { if (auto owner=weak.lock()) owner->callback={}; };
    }
    void connect_options(MenuOptionsWidget& widget);
    MenuInputSession& input;
    frontend::MenuEventQueue& queue;
private:
    std::vector<std::uint32_t> tokens_;
    std::function<void(MenuOptionsWidget&,std::uint8_t,frontend::OptionsObserverKind)> connect_;
};

// Actual guarded engine/text/input operations for the native widget products.
class MenuWidgetVisuals {
public:
    using Frame=EngineFrame*;
    MenuWidgetVisuals(HMODULE module,Frame root,MenuTextObject& text,MenuWidgetContext& context,
                      std::function<bool()> parent_disabled={})
        : module_(module),root_(root),text_(text),context_(context),parent_disabled_(std::move(parent_disabled)) {}
    Frame root() const { return root_; }
    Frame find(Frame root,const char* pattern,std::uint32_t flags) {
        using Find=Frame (WINAPI *)(Frame,const char*,std::uint32_t);
        return scene_method<Find>(root,module_,0x48,0x1ded0)(root,pattern,flags);
    }
    unsigned children(Frame frame) { using Count=unsigned (WINAPI *)(Frame); return scene_method<Count>(frame,module_,0x3c,0x1db80)(frame); }
    void pick(Frame frame,std::uint32_t id) { store(frame,0xec,id); }
    void visible(Frame frame,std::uint8_t value) { using Set=void (WINAPI *)(Frame,std::uint8_t); scene_method<Set>(frame,module_,0x30,0x1cfe0)(frame,value); }
    void update(Frame frame) { using Update=void (WINAPI *)(Frame); scene_method<Update>(frame,module_,0x24,0x1e610)(frame); }
    void align(Frame destination,Frame source) {
        const auto flags=field<std::uint32_t>(destination,0xe0);
        if (!(flags & 0x80)) { store(destination,0xb0,local_position(source)); store(destination,0xe0,(flags & 0xfffffedfu)|0x40000000u); }
    }
    void alpha(Frame frame,float value) { using Set=void (WINAPI *)(Frame,float); scene_method<Set>(frame,module_,0xa8,0x368f0)(frame,value); }
    void direction(Frame frame,const std::array<float,3>& value) {
        using Set=void (WINAPI *)(Frame,const float*,const float*);
        scene_method<Set>(frame,module_,0x0c,0x1c8d0)(frame,value.data(),nullptr);
    }
    void link(Frame frame,Frame parent,std::uint32_t mode) {
        using Link=int (WINAPI *)(Frame,Frame,std::uint32_t);
        if (scene_method<Link>(frame,module_,0x38,0x1d680)(frame,parent,mode)<0) throw std::runtime_error("Widget frame reparent failed");
    }
    std::array<float,3> local_position(Frame frame) { return field<std::array<float,3>>(frame,0xb0); }
    std::array<float,3> world_position(Frame frame) {
        if (!(field<std::uint32_t>(frame,0xe0)&0x20)) {
            const auto address=GetProcAddress(module_,"?UpdateWMatrixProc@I3D_frame@@AAEXXZ");
            if (reinterpret_cast<std::uintptr_t>(address)!=reinterpret_cast<std::uintptr_t>(module_)+0x1e390) throw std::runtime_error("Unsupported widget world matrix export");
            using Update=void (__thiscall *)(Frame); Update update=nullptr; std::memcpy(&update,&address,sizeof(update)); update(frame);
        }
        return field<std::array<float,3>>(frame,0x70);
    }
    std::array<float,4> bounds(Frame frame) {
        if (!(field<std::uint32_t>(frame,0xe0)&0x100)) { using Update=void (WINAPI *)(Frame); scene_method<Update>(frame,module_,0x58,0x1e7d0)(frame); }
        return {field<float>(frame,0x10),field<float>(frame,0x14),field<float>(frame,0x20),field<float>(frame,0x24)};
    }
    void position(Frame frame,const std::array<float,3>& value) { using Set=void (WINAPI *)(Frame,const float*); scene_method<Set>(frame,module_,8,0x1c9f0)(frame,value.data()); }
    std::array<float,3> pointer() const { const auto& cursor=context_.input.pointer.cursor(); return frontend::cursor_frame_position(cursor.x(),cursor.y(),800,600); }
    std::array<float,3> motion() const { const auto& cursor=context_.input.pointer.cursor(); return frontend::cursor_frame_position(cursor.horizontal(),cursor.vertical(),800,600); }
    std::uint32_t idle() const { return context_.input.pointer.idle(); }
    bool held() const { return context_.input.pointer.buttons().currently_selected(0); }
    bool parent_disabled() const { return parent_disabled_ && parent_disabled_(); }
    std::optional<float> track_percent() {
        if (field<std::uint16_t>(root_,0xf8)!=1) return std::nullopt;
        const auto export_address=GetProcAddress(module_,"?UpdateWBoundProc@I3D_visual@@AAGXXZ");
        if (reinterpret_cast<std::uintptr_t>(export_address)!=reinterpret_cast<std::uintptr_t>(module_)+0x571b0) throw std::runtime_error("Unsupported widget visual bounds export");
        if (!(field<std::uint32_t>(root_,0x1d0)&0x40000)) { using Update=void (WINAPI *)(Frame); Update update=nullptr; std::memcpy(&update,&export_address,sizeof(update)); update(root_); }
        const auto point=pointer(); const auto left=field<float>(root_,0x170),bottom=field<float>(root_,0x174);
        const auto width=field<float>(root_,0x180)-left,height=field<float>(root_,0x184)-bottom;
        const auto extent=height>width ? height : width;
        if (extent==0) throw std::domain_error("Zero widget track extent");
        return height>width ? static_cast<float>(100.L-(static_cast<long double>(point[1])-bottom)/height*100.f) :
                             static_cast<float>((static_cast<long double>(point[0])-left)/width*100.f);
    }
    auto& object() { return text_.object(); }
    frontend::SharedText localize(std::uint32_t id) { return text_.localize(id); }
    void set_localized(std::uint32_t id,std::uint32_t style,std::int32_t index) { text_.set_localized(id,style,index); }
    void append_localized(std::uint32_t id,std::uint32_t style) { text_.append_localized(id,style); }
    void layout_text_on_frame() { text_.layout_text_on_frame(); }
    void text_focus(bool value) { auto& object=text_.object(); if (value) { if (!object.selection_active && !object.records().empty()) object.hovered_record=0; } else object.clear_hover_selection(); }
    bool dispatch_immediate(frontend::ControlEvent& event) { context_.queue.dispatch_immediate(event); return true; }
    std::function<void()> subscribe(std::uint32_t token,std::function<void(std::uint32_t)> callback) { return context_.subscribe(token,std::move(callback)); }
private:
    template<class T> static T field(Frame frame,std::size_t offset) { if (!frame) throw std::invalid_argument("Null widget frame"); T value; std::memcpy(&value,reinterpret_cast<unsigned char*>(frame)+offset,sizeof(value)); return value; }
    template<class T> static void store(Frame frame,std::size_t offset,const T& value) { if (!frame) throw std::invalid_argument("Null widget frame"); std::memcpy(reinterpret_cast<unsigned char*>(frame)+offset,&value,sizeof(value)); }
    HMODULE module_; Frame root_; MenuTextObject& text_; MenuWidgetContext& context_; std::function<bool()> parent_disabled_;
};

template<class Settings>
MenuWidgetContext::MenuWidgetContext(MenuInputSession& input,frontend::MenuEventQueue& queue,Settings& settings)
    : input(input),queue(queue),connect_([&settings](MenuOptionsWidget& widget,std::uint8_t screen,frontend::OptionsObserverKind kind) { widget.connect(settings,screen,kind); }) {}

inline void MenuWidgetContext::connect_options(MenuOptionsWidget& widget) {
        const auto local=widget.identifier() & 0x03ffffffu;
        const auto screen=static_cast<std::uint8_t>(widget.identifier() >> 26);
        if (screen != 6) return; // Named OPTIONS registration; other screens choose explicit bindings.
        using Observer=frontend::OptionsObserverKind;
        if (local==0x604000 || local==0x706000 || local==0x707000 || local==0x806000 || local==0xb06000)
            connect_(widget,screen,Observer::slider);
        else if (local==0x605000 || (local>=0x606000 && local<=0x60c000 && (local & 0xfff)==0))
            connect_(widget,screen,Observer::graphics);
        else if (local==0x804000 || local==0xa02000 || local==0xa03000) connect_(widget,screen,Observer::toggle);
        else if (local==0xb04000) connect_(widget,screen,Observer::voice);
    }

}
#endif
