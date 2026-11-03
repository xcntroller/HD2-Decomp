#ifndef HD2_HOST_MENU_PRESENTATION_H
#define HD2_HOST_MENU_PRESENTATION_H

#include "frontend/startup_language.h"
#include <cmath>
#include <utility>

namespace hd2::host {

// One application owns the original shared font manager/render dimensions.
// The diagnostic preview retains its accepted English Set2 defaults.
struct MenuPresentation {
    int width = 800, height = 600;
    std::string language = "english";
    bool configured = false;
    unsigned font_set() const { return width < 800 ? 1u : width < 1024 ? 2u : 3u; }
    std::string font_family() const {
        //0063DC50 selects CE for Czech/Polish, ANSI for the other byte fonts.
        return frontend::startup_case_equal(language,"Czech") ||
               frontend::startup_case_equal(language,"Polish") ? "CE" : "ANSI";
    }
};
inline MenuPresentation& menu_presentation() { static MenuPresentation value; return value; }
inline int menu_width() { return menu_presentation().width; }
inline int menu_height() { return menu_presentation().height; }
inline int menu_line_height(float normalized) {
    return static_cast<int>(static_cast<double>(menu_height()) * normalized + .5);
}
inline float menu_design_height(float pixels) {
    return static_cast<float>(static_cast<int>(static_cast<double>(pixels)*menu_height()/600.0+.5));
}
inline void validate_menu_dimensions(int width,int height) {
    if (width < 320 || height < 200 || width > 16384 || height > 16384)
        throw std::invalid_argument("Render dimensions must be between 320x200 and 16384x16384");
}
class MenuPresentationSession {
public:
    MenuPresentationSession(int width,int height) : previous_(menu_presentation()) {
        validate_menu_dimensions(width,height);
        menu_presentation()={width,height,"english",true};
    }
    ~MenuPresentationSession() { menu_presentation()=std::move(previous_); }
    MenuPresentationSession(const MenuPresentationSession&)=delete;
    MenuPresentationSession& operator=(const MenuPresentationSession&)=delete;
    void language(const std::string& name,bool wide) {
        if (wide)
            throw std::runtime_error("The selected language requires unavailable wide-font configuration: " + name);
        bool supported=false;
        for(const auto* candidate:{"English","EnglishUS","Czech","Polish","French","German","Italian","Spanish"})
            supported=supported || frontend::startup_case_equal(name,candidate);
        if(!supported)throw std::runtime_error("Unsupported menu font language: "+name);
        menu_presentation().language=name;
    }
private:
    MenuPresentation previous_;
};
inline std::string menu_font_metrics_path(const char* size,bool ce=false) {
    const auto& configuration=menu_presentation();
    if (!configuration.configured && !ce) return std::string("MenuText\\")+size+".tbl";
    return "Text\\Fonts\\" + (ce ? std::string("CE") : configuration.font_family()) +
           "\\Set" + std::to_string(configuration.font_set()) + "\\" + size + ".tbl";
}
inline std::string menu_font_texture(const char* size,bool ce=false) {
    const auto& configuration=menu_presentation();
    if (!configuration.configured) return std::string(ce ? "HD2_CREDITS_CE_" : "HD2_MENU_")+size+".TGA";
    auto path=menu_font_metrics_path(size,ce);
    path.replace(path.size()-3,3,"tga");
    return path;
}

} // namespace hd2::host
#endif
