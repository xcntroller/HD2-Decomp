#ifndef HD2_HOST_GAME_FONTS_H
#define HD2_HOST_GAME_FONTS_H

#include "host/font_atlas.h"
#include "host/bitmap_font.h"
#include "host/menu_presentation.h"
#include "frontend/pre_mission_data.h"

namespace hd2::host {

// Byte-font branch of0063DB50/0063DC50/0063DBB0. All game text owners share this
// actual five-atlas allocation. Use the accepted metrics decoder/font paths and
// real rw_data resolver for metrics; no additional extract/staging requirement.
// Wide/GDI font families retain the launcher's explicit unsupported boundary.
class GameFonts {
public:
    template<class Files, class Graphics>
    GameFonts(Files& files, Graphics* graphics, HMODULE module) {
        const std::array<std::string, 5> metrics{{
            menu_font_metrics_path("BIG"), menu_font_metrics_path("SMALL"),
            "Text\\Fonts\\SYMBOLS.tbl", menu_font_metrics_path("BIG", true),
            menu_font_metrics_path("SMALL", true)}};
        const std::array<std::string, 5> textures{{
            menu_font_texture("BIG"), menu_font_texture("SMALL"),
            "Text\\Fonts\\SYMBOLS.tga", menu_font_texture("BIG", true),
            menu_font_texture("SMALL", true)}};
        //0063DC50 loads symbols and CE before primary BIG/SMALL.
        for (const auto index : {2u, 3u, 4u, 0u, 1u}) {
            const auto bytes = frontend::pre_mission_detail::read_file(files, metrics[index].c_str(), 65537);
            if (!bytes || bytes->size() > 65536)
                throw std::runtime_error("Cannot read original game font metrics: " + metrics[index]);
            metrics_[index] = decode_font_metrics(*bytes);
            atlases_[index] = load_font_atlas(graphics, module, textures[index].c_str());
        }
    }
    const FontMetrics& metrics(unsigned original_id) const { return metrics_.at(original_id - 2); }
    EngineFrame* atlas(unsigned original_id) const { return atlases_.at(original_id - 2).get(); }
    bool contains(unsigned original_id)const{return original_id>=2&&original_id<=6;}
    const GlyphMetrics& glyph(unsigned original_id,unsigned char character)const{return metrics(original_id)[character];}
private:
    std::array<FontMetrics, 5> metrics_;
    std::array<MenuDriver::Owner, 5> atlases_{{{nullptr,nullptr},{nullptr,nullptr},{nullptr,nullptr},
                                           {nullptr,nullptr},{nullptr,nullptr}}};
};

} // namespace hd2::host
#endif
