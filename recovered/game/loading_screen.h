#ifndef HD2_GAME_LOADING_SCREEN_H
#define HD2_GAME_LOADING_SCREEN_H

#include "frontend/cleanup.h"
#include "frontend/game_data.h"
#include "frontend/text_object.h"
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

namespace hd2::game {

struct LoadingTextBinding {
    const char* name;
    std::uint8_t style;
    std::uint32_t caption;
    const char* suffix;
};
//00842FF8, fifteen sixteen-byte original descriptors, not native object ABI.
inline constexpr std::array<LoadingTextBinding,15> loading_text_bindings{{
    {"briefing",84,0xffffffffu,nullptr},{"textbar",80,0xffffffffu,nullptr},
    {"title",80,0xffffffffu,nullptr},{"MP_list1",83,0xb87," :"},
    {"MP_list2",83,0xb87," :"},{"MP_objective1",82,0xb86," :"},
    {"MP_objective2",82,0xb86," :"},{"MP_title",80,0xffffffffu,nullptr},
    {"MP_text_screen",79,0xffffffffu,nullptr},{"MP_team allied1",80,0xa3d,nullptr},
    {"MP_team allied2",80,0xa3c,nullptr},{"MP_map survey",82,0xb88," :"},
    {"MP_progress",82,0xb89," :"},{"MP_designated",81,0xb8a," :"},
    {"text_h&dversion",40,0xffffffffu,nullptr}
}};

//006C9640/006CA470/006CA2D0's single-player loading presentation, including
//006C95B0/006C95C0's complementary finish gates. Backend supplies real engine
//owners, geometry/text, graphics/input and effects consumers; none default to
//success. Multiplayer-specific descriptor/text producers remain unsupported.
template<class Backend>
class LoadingScreen {
public:
    using Frame=typename Backend::Frame;
    using FrameOwner=decltype(std::declval<Backend&>().create_loading_frame(13));
    using LabelOwner=decltype(std::declval<Backend&>().make_loading_label(std::declval<Frame>(),std::declval<Frame>()));
    explicit LoadingScreen(Backend& backend) : backend_(backend) {}
    ~LoadingScreen() {discard();} //Native failure cleanup never starts an interactive wait.
    LoadingScreen(const LoadingScreen&)=delete;
    LoadingScreen& operator=(const LoadingScreen&)=delete;

    bool prepare(const std::string& resource,bool enabled,bool wait_for_space,bool through_activation,
                 const frontend::GameDataNode* mission) { //006C9640, original RET20.
        close();wait_=wait_for_space;through_activation_=through_activation;value_=0;
        try {
            if(enabled) {
                scene_.emplace(backend_.create_loading_frame(13));
                if(!scene_->get())throw std::runtime_error("Loading scene factory returned null");
                backend_.fit_loading_viewport(scene());
                std::array<std::uint32_t,3> saved{};
                for(std::size_t i=0;i!=states.size();++i)saved[i]=backend_.loading_driver_state(states[i])!=0;
                frontend::CleanupErrors errors;
                std::int32_t status=-1;
                try {
                    for(std::size_t i=0;i!=states.size();++i)backend_.loading_driver_state(states[i],i!=0);
                    status=backend_.open_loading_scene(scene(),"missions\\"+resource+"\\loader");
                }catch(...) {errors=frontend::CleanupErrors(std::current_exception());}
                for(std::size_t i=0;i!=states.size();++i)
                    errors.attempt([&] {backend_.loading_driver_state(states[i],saved[i]);});
                errors.rethrow();
                if(status<0) {discard();return false;} //Original leaves availability false.
                camera_.emplace(backend_.create_loading_frame(3));
                backend_.configure_loading_camera(scene(),camera_->get());
                light_.emplace(backend_.create_loading_frame(2));
                backend_.configure_loading_light(scene(),light_->get());
                for(std::size_t i=labels_.size();i;--i) {
                    const auto index=i-1;const auto& binding=loading_text_bindings[index];
                    const auto frame=backend_.find_loading_frame(scene(),binding.name);
                    if(!frame)continue;
                    backend_.hide_loading_text_frame(frame);
                    labels_[index]=backend_.make_loading_label(scene(),frame);
                    if(!labels_[index])throw std::runtime_error("Loading text factory returned null");
                    auto& text=labels_[index]->text;
                    text.inset=5;text.clear();
                    fill_text(index,text,mission);
                }
                const auto bar=backend_.find_loading_frame(scene(),"loadbar");
                //FindFrame is borrowed, but006CA2D0 consumes this existing
                //engine reference before releasing the scene. Match that
                //specific transfer; do not add an unobserved retain here.
                if(bar)bar_.emplace(backend_.take_loading_bar(bar));
                draw();draw();
            }
            backend_.clear_loading_names(); //006CA670, capacity survives.
            active_=true;return true; //Original also sets availability when scene creation is disabled.
        }catch(...) {discard();throw;}
    }
    bool active() const {return active_;} //006CA2C0, not scene presence.
    Frame scene() const {return scene_?scene_->get():Frame{};}
    float value() const {return value_;}
    void value(float next) {
        if(!std::isfinite(next))throw std::runtime_error("Non-finite loading screen progress");
        value_=next;
    }
    void finish_loading() {if(!through_activation_)close();} //006C95B0.
    void finish_activation() {if(through_activation_)close();} //006C95C0.
    void draw() { //006CA470, no availability check here.
        backend_.pump_loading_messages();
        if(backend_.external_loading_progress()) {backend_.send_loading_progress(value_);return;}
        if(!scene())return;
        if(bar_)backend_.scale_loading_bar(bar_->get(),value_);
        backend_.render_loading_scene(scene());
        const auto saved=backend_.loading_graphics_state();
        bool begun=false;
        frontend::CleanupErrors errors;
        try {
            backend_.loading_graphics_state(1);
            backend_.begin_loading_text();begun=true;
            for(std::size_t i=labels_.size();i;--i)if(labels_[i-1])backend_.draw_loading_label(*labels_[i-1]);
        }catch(...) {errors=frontend::CleanupErrors(std::current_exception());}
        if(begun)errors.attempt([&] {backend_.end_loading_text();});
        errors.attempt([&] {backend_.loading_graphics_state(saved);});
        errors.rethrow();
        backend_.present_loading();backend_.clear_loading(); //Clear AFTER present.
    }
    void close() { //006CA2D0, matching ordered cleanup even after native exceptions.
        value_=1;
        if(!active_)return;
        frontend::CleanupErrors errors;
        try {
            draw();
            if(wait_&&scene()) {
                backend_.advance_loading_effects(100); //Actual GEffects+10, not a fabricated timer.
                if(labels_[1])labels_[1]->text.set(backend_.loading_localized(0x7f8),80,0);
                do {
                    backend_.pump_loading_messages();backend_.read_loading_keys();draw();
                }while(backend_.loading_key()!=0x39);
                if(labels_[1])labels_[1]->text.set(std::make_shared<const std::string>(),80,0);
                for(unsigned i=0;i!=3;++i) {backend_.clear_loading();backend_.present_loading();}
            }
        }catch(...) {errors=frontend::CleanupErrors(std::current_exception());}
        discard();
        errors.attempt([&] {backend_.clear_loading_names();});
        errors.rethrow();
    }
    void discard() noexcept {
        //006CA2D0 releases labels14..0, light, bar, scene, then camera. Native
        //partial-prepare rollback uses this acquired-prefix order as well.
        for(std::size_t i=labels_.size();i;--i)if(labels_[i-1]) {
            labels_[i-1]->text.clear();labels_[i-1].reset();
        }
        light_.reset();bar_.reset();scene_.reset();camera_.reset();active_=false;value_=0;
    }
private:
    void fill_text(std::size_t index,frontend::TextObject& text,const frontend::GameDataNode* mission) {
        const auto style=loading_text_bindings[index].style;
        if(index==0||index==2) {
            if(!mission)return;
            if(mission->kind!=3)throw std::runtime_error("Loading screen requires its actual mission descriptor");
            const auto caption=mission->words40[index==0?0:1];
            if(caption!=0xffffffffu)text.append(backend_.loading_localized(caption),style);
        }else if(index==14)text.set(backend_.loading_version(),style,0);
        else if(index!=1)throw std::runtime_error("Multiplayer loading text requires its actual descriptor producers");
    }
    inline static constexpr std::array<unsigned,3> states{{0x15,0x19,0x11}};
    Backend& backend_;
    std::optional<FrameOwner> scene_,camera_,light_,bar_;
    std::array<LabelOwner,15> labels_{};
    float value_=0;
    bool active_=false,wait_=false,through_activation_=false;
};

} //namespace hd2::game
#endif
