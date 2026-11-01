#ifndef HD2_HOST_MENU_SCREEN_FACTORY_H
#define HD2_HOST_MENU_SCREEN_FACTORY_H

#include "frontend/owned_screens.h"
#include "frontend/screen_construction.h"
#include "frontend/menu_event_queue.h"
#include "host/menu_screen_product.h"

namespace hd2::host {

// Concrete engine adapter for the two currently implemented named definitions.
// The scene/resource service already owns the loaded model roots. Products own
// their copied definitions and children. More screens must supply their actual
// resource/builders before they enter this factory; unsupported builders fail.
// Native owning-vector indices are construction order; the bounded host still
// resolves event ID families through the original static registry.
template<class Bindings>
class MenuScreenFactory {
public:
    using Products = frontend::OwnedScreens<MenuScreenProduct>;
    MenuScreenFactory(const MenuScene& scene, HMODULE module,
                      std::function<MenuTextObject*(std::uint32_t)> text_lookup,
                      frontend::MenuEventQueue& queue, Products& products, Bindings& bindings, MenuWidgetContext* widgets = nullptr, bool credits = false,
                      EngineFrame* cinematic_model=nullptr,
                      std::function<frontend::MenuScreenDefinition()> build_cinematics={},
                      std::function<void(const frontend::MenuScreenDefinition&)> attach_cinematics={},
                      std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition&,EngineFrame*,MenuTextObject&)> extra={},
                      EngineFrame* options_model=nullptr,
                      std::function<frontend::MenuScreenDefinition()> build_options={},
                      EngineFrame* profiles_model=nullptr, std::function<frontend::MenuScreenDefinition()> build_profiles={}, bool single_player=false, bool multiplayer=false)
        : scene_(scene), module_(module), text_lookup_(std::move(text_lookup)),
          queue_(queue), products_(products), bindings_(bindings), widgets_(widgets),build_cinematics_(std::move(build_cinematics)),attach_cinematics_(std::move(attach_cinematics)),extra_(std::move(extra)),
          main_{scene.main_model_frame(), false, credits ?
                std::vector<frontend::MenuControlRole>{frontend::MenuControlRole::exit,frontend::MenuControlRole::credits} :
                std::vector<frontend::MenuControlRole>{frontend::MenuControlRole::exit}},
          confirmation_{scene.confirmation_model_frame(), true,
                        {frontend::MenuControlRole::yes, frontend::MenuControlRole::no}},
          cinematics_{cinematic_model,false,{frontend::MenuControlRole::play,frontend::MenuControlRole::back,frontend::MenuControlRole::movie_list}},
          options_{options_model,false,{}},build_options_(std::move(build_options)),profiles_{profiles_model,false,{frontend::MenuControlRole::profiles}},build_profiles_(std::move(build_profiles)) {
        if(single_player)main_.available.push_back(frontend::MenuControlRole::single_player);
        if(multiplayer)main_.available.push_back(frontend::MenuControlRole::multiplayer);
        if(profiles_model && build_profiles_ && attach_cinematics_)main_.available.push_back(frontend::MenuControlRole::profiles);
        if(cinematic_model && build_cinematics_ && attach_cinematics_)
            main_.available.push_back(frontend::MenuControlRole::cinematics);
        if(options_model && build_options_ && attach_cinematics_)
            main_.available.push_back(frontend::MenuControlRole::options);
    }

    void create_definitions() {
        definitions_ = {frontend::main_menu_definition(), frontend::confirmation_definition()};
        if(cinematics_.root) definitions_.push_back({"cinematics",0x1c000000,"single mission 2",{}});
        if(profiles_.root) definitions_.push_back({"profiles",0x08000000,"change_profile",{}});
        if(options_.root) definitions_.push_back({"options",0x18000000,"options",{}});
    }
    std::size_t definition_count() const { return definitions_.size(); }
    frontend::MenuScreenDefinition& definition(std::size_t index) { return definitions_.at(index); }

    struct Model {
        EngineFrame* root;
        bool confirmation;
        std::vector<frontend::MenuControlRole> available;
    };
    Model* load_model(const frontend::MenuScreenDefinition& definition) {
        Model* model = definition.identifier == 0x08000000 ? &profiles_ : definition.identifier == 0x04000000 ? &main_ :
                       definition.identifier == 0x10000000 ? &confirmation_ : definition.identifier == 0x1c000000 ? &cinematics_ : definition.identifier == 0x18000000 ? &options_ : nullptr;
        if (!model) throw std::logic_error("Screen needs a recovered engine/model backend");
        return model->root ? model : nullptr;
    }
    void augment_screen(frontend::ScreenBuilder builder, frontend::MenuScreenDefinition& definition, Model*) {
        if(builder==frontend::ScreenBuilder::profile && build_profiles_ && attach_cinematics_){definition=build_profiles_();attach_cinematics_(definition);return;}
        if(builder==frontend::ScreenBuilder::cinematics && build_cinematics_ && attach_cinematics_){definition=build_cinematics_();attach_cinematics_(definition);return;}
        if(builder==frontend::ScreenBuilder::options && build_options_ && attach_cinematics_){definition=build_options_();attach_cinematics_(definition);return;}
        throw std::logic_error("Screen-specific application builder is not reconstructed");
    }
    void position_model(Model* model, float x, float y, float z) {
        scene_.position_menu_model(module_, model->root, x, y, z);
    }
    std::unique_ptr<MenuScreenProduct> allocate_screen(Model* model) {
        return std::make_unique<MenuScreenProduct>(scene_, module_, model->confirmation,
            model->available, text_lookup_, queue_, model->root, widgets_, extra_);
    }
    void append_screen(std::unique_ptr<MenuScreenProduct> screen) {
        const auto& definition = screen->definition();
        const auto family = definition.identifier >> 26;
        auto* product = screen.get();
        products_.append({definition.label, definition.model, definition.identifier}, std::move(screen));
        bindings_.bind_screen(family, product->events());
    }
    void destroy_definitions() {
        definitions_.clear();
        definitions_.shrink_to_fit();
    }
    void dispatch_initial_event(frontend::ControlEvent& event) { queue_.dispatch_immediate(event); }

private:
    const MenuScene& scene_;
    HMODULE module_;
    std::function<MenuTextObject*(std::uint32_t)> text_lookup_;
    frontend::MenuEventQueue& queue_;
    Products& products_;
    Bindings& bindings_;
    MenuWidgetContext* widgets_;
    std::function<frontend::MenuScreenDefinition()> build_cinematics_;
    std::function<void(const frontend::MenuScreenDefinition&)> attach_cinematics_;
    std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition&,EngineFrame*,MenuTextObject&)> extra_;
    Model main_;
    Model confirmation_;
    Model cinematics_;
    Model options_;
    std::function<frontend::MenuScreenDefinition()> build_options_;
    Model profiles_;
    std::function<frontend::MenuScreenDefinition()> build_profiles_;
    std::vector<frontend::MenuScreenDefinition> definitions_;
};

// Slot zero has a separate singleton owner installed before definition binding.
// Registration happens only after success, and root initialization is omitted.
template<class Bindings>
class RuntimeMenuScreenFactory {
public:
    RuntimeMenuScreenFactory(const MenuScene& scene, HMODULE module,
                            const frontend::MenuScreenDefinition& definition,
                            std::function<MenuTextObject*(std::uint32_t)> text_lookup,
                            frontend::MenuEventQueue& queue,
                            std::unique_ptr<MenuScreenProduct>& installed, Bindings& bindings)
        : scene_(scene), module_(module), definition_(definition), text_lookup_(std::move(text_lookup)),
          queue_(queue), installed_(installed), bindings_(bindings) {}
    EngineFrame* load_runtime_model() const { return scene_.runtime_confirmation_frame(); }
    void position_model(EngineFrame* model, float x, float y, float z) {
        scene_.position_menu_model(module_, model, x, y, z);
    }
    std::unique_ptr<frontend::MenuScreenDefinition> create_runtime_definition() {
        return std::make_unique<frontend::MenuScreenDefinition>(definition_);
    }
    std::unique_ptr<MenuScreenProduct> allocate_runtime_confirmation(EngineFrame* model) {
        return std::make_unique<MenuScreenProduct>(scene_, module_, true,
            std::vector<frontend::MenuControlRole>{frontend::MenuControlRole::yes,
                frontend::MenuControlRole::no, frontend::MenuControlRole::ok}, text_lookup_, queue_, model);
    }
    MenuScreenProduct& install_runtime_confirmation(std::unique_ptr<MenuScreenProduct> screen) {
        if (installed_) throw std::logic_error("Runtime confirmation singleton already installed");
        installed_ = std::move(screen);
        return *installed_;
    }
    void append_runtime_confirmation(MenuScreenProduct& screen) { bindings_.bind_screen(0, screen.events()); }
private:
    const MenuScene& scene_;
    HMODULE module_;
    const frontend::MenuScreenDefinition& definition_;
    std::function<MenuTextObject*(std::uint32_t)> text_lookup_;
    frontend::MenuEventQueue& queue_;
    std::unique_ptr<MenuScreenProduct>& installed_;
    Bindings& bindings_;
};

}

#endif
