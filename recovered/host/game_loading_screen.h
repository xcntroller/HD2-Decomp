#ifndef HD2_HOST_GAME_LOADING_SCREEN_H
#define HD2_HOST_GAME_LOADING_SCREEN_H

#include "game/loading_screen.h"
#include "host/game_fonts.h"
#include "host/menu_text.h"
#include "host/loading_text_geometry.h"
#include "host/graphics_interface.h"
#include "host/chunk_files.h"

namespace hd2::host {

class LoadingPresentationRuntime {
public:
    virtual ~LoadingPresentationRuntime()=default;
    virtual bool external_loading_progress() const=0; //Actual application+170.
    virtual void send_loading_progress(std::int32_t)=0; //00731720, external branch.
    virtual void advance_loading_effects(std::uint32_t)=0; //GEffects+10.
};

//Actual LS3DF/graphics/font endpoints. The containing loader must supply the
//live runtime consumers and call explicit close before destroying this backend.
//No loading scene or runtime callback is installed by this header alone.
class GameLoadingScreenBackend {
public:
    using Frame=EngineFrame*;
    struct Label {
        Label(std::shared_ptr<GameFonts> resources,std::array<float,8> geometry)
            : fonts(std::move(resources)),bounds(geometry) {}
        ~Label() {fonts.reset();text.clear();} //0048ECF0, explicit close clears text first.
        frontend::TextObject text;
        std::shared_ptr<GameFonts> fonts;
        std::array<float,8> bounds;
        std::vector<CursorVertex> vertices;
        unsigned font=2;
    };
    GameLoadingScreenBackend(MenuDriver& driver,HMODULE module,Graphics& graphics,
                             ChunkFiles& files,const frontend::LocalizedTextCache& text,
                             LoadingPresentationRuntime& runtime)
        : driver_(driver),module_(module),graphics_(graphics),files_(files),text_(text),runtime_(runtime) {}
    MenuDriver::Owner create_loading_frame(unsigned kind) {return driver_.create(kind);}
    MenuDriver::Owner take_loading_bar(Frame frame) {
        //006CA222 stores FindFrame's borrowed result without addref;006CA2D0
        //later consumes that engine reference before scene release.
        return {frame,scene_method<MenuDriver::Release>(frame,module_,0,0x1c370)};
    }
    std::uint32_t loading_driver_state(unsigned state) {return driver_.render_state(state);}
    void loading_driver_state(unsigned state,std::uint32_t value) {driver_.set_render_state(state,value);}
    void fit_loading_viewport(Frame scene) {
        const std::array<std::int32_t,4> expected{{0,0,graphics_.methods->width(&graphics_),graphics_.methods->height(&graphics_)}};
        if(field<std::array<std::int32_t,4>>(scene,0x244)!=expected)
            call<void>(scene,0x148,expected[0],expected[1],expected[2],expected[3]);
    }
    std::int32_t open_loading_scene(Frame scene,const std::string& path) {
        return call<std::int32_t>(scene,0xf4,path.c_str(),std::uint32_t{0},static_cast<void*>(nullptr),std::uint32_t{0});
    }
    void configure_loading_camera(Frame scene,Frame camera) { //006C9865..006C9922.
        call<void>(scene,0x90,camera);
        const std::array<float,3> direction{{0,0,1}};
        call<void>(camera,0x1c,direction.data(),static_cast<const float*>(nullptr));
        position(camera,{{.5f,-.375f,-15.5f}});
        call<void>(camera,0x7c,.2f,500.f);
        call<void>(camera,0x88,std::uint32_t{1});call<void>(camera,0x84,2.f);
        call<void>(camera,0x24);
    }
    void configure_loading_light(Frame scene,Frame light) { //006C9927..006C99A3.
        call<void>(light,0x34,"C_LoaderScreen");call<void>(light,0x74,std::uint32_t{4});
        const std::array<float,3> color{{1,1,1}};call<void>(light,0x84,color.data());
        call<void>(light,0x90,1.f);call<void>(light,0x94,std::uint32_t{0x11});
        call<void>(field<Frame>(scene,0x1f8),0x74,light);
    }
    Frame find_loading_frame(Frame scene,const char* name) {return call<Frame>(scene,0x7c,name,std::uint32_t{0xffff});}
    void hide_loading_text_frame(Frame frame) {call<void>(frame,0x30,std::uint32_t{0});}
    std::unique_ptr<Label> make_loading_label(Frame,Frame frame) {
        auto fonts=fonts_.lock();
        if(!fonts) {fonts=std::make_shared<GameFonts>(files_,&graphics_,module_);fonts_=fonts;}
        if(field<std::uint16_t>(frame,0xf8)!=1)throw std::runtime_error("Loading text requires an authored visual");
        using Update=void(WINAPI*)(Frame);
        const auto entry=GetProcAddress(module_,"?UpdateWBoundProc@I3D_visual@@AAGXXZ");
        if(reinterpret_cast<std::uintptr_t>(entry)!=reinterpret_cast<std::uintptr_t>(module_)+0x571b0)
            throw std::runtime_error("Unsupported loading text world-bounds export");
        Update update;std::memcpy(&update,&entry,sizeof(entry));update(frame);
        return std::make_unique<Label>(std::move(fonts),field<std::array<float,8>>(frame,0x170));
    }
    frontend::SharedText loading_localized(std::uint32_t id) {
        return std::make_shared<const std::string>(frontend::lookup_localized_text(text_,static_cast<std::int32_t>(id)));
    }
    frontend::SharedText loading_version() {
        return std::make_shared<const std::string>(frontend::main_version_text(frontend::lookup_localized_text(text_,0x7d6)));
    }
    void queue_loading_name(unsigned side,frontend::SharedText name) {pending_names_.at(side).push_back(std::move(name));}
    void clear_loading_names() noexcept { //006CA670, forward COW release and retained vector capacity.
        for(auto& names:pending_names_) {for(auto& name:names)name.reset();names.clear();}
    }
    void pump_loading_messages() {graphics_.methods->pump(&graphics_);}
    bool external_loading_progress() const {return runtime_.external_loading_progress();}
    void send_loading_progress(float value) {
        const auto scaled=static_cast<long double>(value)*100.f;
        if(scaled<-2147483648.L||scaled>=2147483648.L)throw std::runtime_error("External loading percentage exceeds integer range");
        runtime_.send_loading_progress(static_cast<std::int32_t>(scaled));
    }
    void advance_loading_effects(std::uint32_t milliseconds) {runtime_.advance_loading_effects(milliseconds);}
    void read_loading_keys() {gcall<void>(0x9c);}
    std::int32_t loading_key() {return gcall<std::int32_t>(0xa0);}
    void scale_loading_bar(Frame frame,float value) {
        const std::array<float,3> scale{{value,1,1}};put(frame,0xd0,scale);
        auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&8)) {
            using Make=void(WINAPI*)(void*,const void*);
            const auto entry=GetProcAddress(module_,"?Make@S_quat@@QAGXABUS_matrix@@@Z");
            if(reinterpret_cast<std::uintptr_t>(entry)!=reinterpret_cast<std::uintptr_t>(module_)+0x2f220)
                throw std::runtime_error("Unsupported loading bar quaternion export");
            Make make;std::memcpy(&make,&entry,sizeof(entry));make(address(frame,0xc0),address(frame,0x80));
        }
        put(frame,0xe0,(flags&0xfffffecbu)|0x40000008u);
    }
    void render_loading_scene(Frame scene) {call<void>(scene,0x78,std::uint32_t{0});}
    std::uint32_t loading_graphics_state() {return gcall<std::uint32_t>(0x78,std::uint32_t{0});}
    void loading_graphics_state(std::uint32_t state) {gcall<void>(0x74,std::uint32_t{0},state);}
    void begin_loading_text() {graphics_.methods->begin_scene(&graphics_);}
    void end_loading_text() {graphics_.methods->end_scene(&graphics_);}
    void present_loading() {graphics_.methods->present(&graphics_);}
    void clear_loading() {graphics_.methods->clear(&graphics_,0,1,3);}
    void draw_loading_label(Label& label) {
        if(label.text.dirty)layout(label);
        if(label.vertices.empty())return;
        TextOverlayState state(device());
        const auto* current=graphics_.methods->viewport(&graphics_);
        if(!current)throw std::runtime_error("Loading text viewport is unavailable");
        const auto saved=*current;
        const CursorViewport viewport{0,0,static_cast<unsigned>(menu_width()),static_cast<unsigned>(menu_height()),0,1};
        frontend::CleanupErrors errors;
        try {
            check_overlay_status(graphics_.methods->set_viewport(&graphics_,&viewport),"Set loading text viewport");
            if(!graphics_.methods->bind_texture(&graphics_,label.fonts->atlas(label.font),0))
                throw std::runtime_error("Cannot bind actual loading font atlas");
            check_overlay_status(graphics_.methods->draw_primitive(&graphics_,3,
                static_cast<std::uint32_t>(label.vertices.size()/3),label.vertices.data(),1),"Draw loading text");
        }catch(...) {errors=frontend::CleanupErrors(std::current_exception());}
        errors.attempt([&] {check_overlay_status(graphics_.methods->set_viewport(&graphics_,&saved),"Restore loading text viewport");});
        errors.attempt([&] {if(!state.restore())throw std::runtime_error("Cannot restore loading glyph states");});
        errors.rethrow();
    }
private:
    static unsigned char* address(void* object,std::size_t offset) {return static_cast<unsigned char*>(object)+offset;}
    template<class T> static T field(const void* object,std::size_t offset) {
        if(!object)throw std::runtime_error("Missing loading engine object");
        T result;std::memcpy(&result,static_cast<const unsigned char*>(object)+offset,sizeof(result));return result;
    }
    template<class T> static void put(void* object,std::size_t offset,const T& value) {std::memcpy(address(object,offset),&value,sizeof(value));}
    template<class Result,class...Args> Result call(Frame frame,std::size_t slot,Args...args) const {
        using Function=Result(WINAPI*)(Frame,Args...);
        return checked_engine_method<Function>(frame,module_,slot)(frame,args...);
    }
    template<class Result,class...Args> Result gcall(std::size_t slot,Args...args) const {
        using Function=Result(WINAPI*)(Graphics*,Args...);
        return checked_engine_method<Function>(reinterpret_cast<Frame>(&graphics_),module_,slot)(&graphics_,args...);
    }
    IDirect3DDevice8* device() const {
        const auto base=reinterpret_cast<std::uintptr_t>(module_);
        std::array<unsigned char,5> anchor{{0xa1,0,0,0,0}};
        const auto target=static_cast<std::uint32_t>(base+0xd216c);std::memcpy(anchor.data()+1,&target,4);
        if(std::memcmp(reinterpret_cast<const void*>(base+0x6b389),anchor.data(),anchor.size()))
            throw std::runtime_error("Unsupported loading Direct3D-device bridge");
        IDirect3DDevice8* result;std::memcpy(&result,reinterpret_cast<const void*>(base+0xd216c),sizeof(result));
        if(!result)throw std::runtime_error("Missing loading Direct3D device");return result;
    }
    static void position(Frame frame,const std::array<float,3>& value) {
        auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&0x80)) {put(frame,0xb0,value);put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);}
    }
    static void layout(Label& label) {
        auto geometry=loading_text_geometry(label.text,label.bounds,label.fonts->metrics(2),label.fonts->metrics(3));
        label.font=geometry.font;label.vertices=std::move(geometry.vertices);label.text.dirty=false;
    }
    MenuDriver& driver_;
    HMODULE module_;
    Graphics& graphics_;
    ChunkFiles& files_;
    const frontend::LocalizedTextCache& text_;
    LoadingPresentationRuntime& runtime_;
    std::weak_ptr<GameFonts> fonts_;
    std::array<std::vector<frontend::SharedText>,2> pending_names_;
};

using GameLoadingScreen=game::LoadingScreen<GameLoadingScreenBackend>;

} //namespace hd2::host
#endif
