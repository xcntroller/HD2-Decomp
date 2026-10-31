#ifndef HD2_HOST_MENU_TEXT_GEOMETRY_H
#define HD2_HOST_MENU_TEXT_GEOMETRY_H

#include "host/bitmap_font.h"
#include "frontend/text_object.h"
#include "host/cursor_geometry.h"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>
#include <optional>

namespace hd2::host {

struct MenuRectangle {
    float left, top, right, bottom;

    bool contains(CursorPoint point) const {
        return point.x >= left && point.x < right && point.y >= top && point.y < bottom;
    }
};

inline MenuRectangle menu_rectangle(
        const std::array<float, 8>& bounds, int width, int height, std::uint8_t inset = 0) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Menu projection requires positive dimensions");
    }
    for (const auto index : {0u, 1u, 4u, 5u}) {
        if (!std::isfinite(bounds[index])) {
            throw std::invalid_argument("Nonfinite original menu bounds");
        }
    }
    if (bounds[0] >= bounds[4] || bounds[1] >= bounds[5]) {
        throw std::invalid_argument("Empty or inverted original menu bounds");
    }
    // Text object +90 is in 1/1024-screen-width units, truncated before projection.
    const int padding = static_cast<int>(static_cast<double>(width) * (1.0 / 1024.0) * inset);
    const auto left = static_cast<float>(static_cast<double>(width) * bounds[0] + padding);
    const auto right = static_cast<float>(static_cast<double>(width) * bounds[4] - padding);
    if (left >= right) {
        throw std::invalid_argument("Original menu text inset leaves an empty rectangle");
    }
    return {left, 1.0f - height * bounds[5] * (4.0f / 3.0f),
            right, -height * bounds[1] * (4.0f / 3.0f) - 1.0f};
}

struct MenuLabelBinding {
    const char* frame;
    const char* visual;
    const char* active_visual;
    std::uint32_t id;
    float height;
    bool small;
    bool button;
};

inline constexpr std::array<MenuLabelBinding, 13> menu_label_bindings{{
    {"text_screen", "text_screen", nullptr, 2101, .05f, false, false},
    {"text_date00", "text_date00", nullptr, 2102, .02f, true, false},
    {"text_profile00", "text_profile00", nullptr, 2103, .02f, true, false},
    {"text_last save00", "text_last save00", nullptr, 2104, .02f, true, false},
    {"bresume game", "normal00", "actived00", 2105, .04f, false, true},
    {"bchange profile", "normal01", "actived01", 2106, .04f, false, true},
    {"bsingleplayer", "normal02", "actived02", 2107, .04f, false, true},
    {"bcinematics", "normal04", "actived04", 2109, .04f, false, true},
    {"boptions", "normal05", "actived05", 2110, .04f, false, true},
    {"bcredits", "normal06", "actived06", 2111, .04f, false, true},
    {"bexit", "normal07", "actived07", 2112, .04f, false, true},
    {"bmultiplayerlan", "normal03", "actived03", 2115, .04f, false, true},
    {"bmultiplayernet", "normal08", "actived08", 2116, .04f, false, true},
}};

inline int glyph_advance(const GlyphMetrics& metric, int height) {
    const double aspect = metric.aspect == 0 ? .4 : static_cast<double>(metric.aspect);
    return static_cast<int>(height * aspect + .5);
}

inline std::vector<CursorVertex> menu_label_vertices(
        std::string_view text, const FontMetrics& metrics, const MenuRectangle& rectangle,
        float normalized_height, int render_height, bool title, std::uint32_t color) {
    if (render_height <= 0 || !std::isfinite(normalized_height) ||
            normalized_height <= 0 || normalized_height > 1 || text.size() > 4096) {
        throw std::invalid_argument("Invalid bounded menu label dimensions");
    }
    const int height = static_cast<int>(static_cast<double>(render_height) * normalized_height + .5);
    int line_width = 0;
    for (unsigned char character : text) {
        if (character < 32 || character > 126) {
            throw std::invalid_argument("Static English menu labels require printable ASCII");
        }
        line_width += glyph_advance(metrics[character], height);
    }
    // Styles 11/13 are left/vertically centered; style 10 is right/bottom aligned.
    float x = static_cast<float>(static_cast<int>(
        title ? rectangle.right - line_width : rectangle.left));
    const float bottom = static_cast<float>(static_cast<int>(title ? rectangle.bottom :
        rectangle.top + std::max(0.0f, (rectangle.bottom - rectangle.top - height) * .5f)) +
        (title ? 0 : height));
    std::vector<CursorVertex> result;
    result.reserve(text.size() * 6);
    for (unsigned char character : text) {
        const auto& metric = metrics[character];
        const int advance = glyph_advance(metric, height);
        if (metric.aspect != 0) {
            constexpr float z = 0x1.d17e0cp-6f;
            constexpr float rhw = 0x1.1993bep+5f;
            const CursorVertex bl{x, bottom, z, rhw, color, 0, metric.u_left, metric.v_bottom};
            const CursorVertex tl{x, bottom - height, z, rhw, color, 0, metric.u_left, metric.v_top};
            const CursorVertex tr{x + advance, bottom - height, z, rhw, color, 0,
                                  metric.u_right, metric.v_top};
            const CursorVertex br{x + advance, bottom, z, rhw, color, 0,
                                  metric.u_right, metric.v_bottom};
            result.insert(result.end(), {bl, tl, tr, br, bl, tr});
        }
        x += advance;
    }
    return result;
}

//00641E30 styles24/25 set wrapping (+2d);00645F20 uses the last fitting
// whitespace. Keep a single printable-English style, plus explicit CR/LF rows.
// Native bounds reject unsupported overwide words or overflowing paragraphs.
inline std::vector<CursorVertex> menu_prompt_vertices(
        std::string_view text, const FontMetrics& metrics, const MenuRectangle& rectangle,
        float normalized_height, int render_height, std::uint32_t color) {
    if(text.size()>4096 || render_height<=0 || !std::isfinite(normalized_height) ||
       normalized_height<=0 || normalized_height>1)
        throw std::invalid_argument("Invalid runtime dialog paragraph");
    const int height=static_cast<int>(static_cast<double>(render_height)*normalized_height+.5);
    const float available=rectangle.right-rectangle.left;
    if(height<=0 || !std::isfinite(available) || available<=0)
        throw std::invalid_argument("Invalid runtime dialog text bounds");
    std::vector<std::string_view> lines;
    std::size_t begin=0;
    do {
        const auto end=text.find_first_of("\r\n",begin);
        auto remaining=text.substr(begin,end==std::string_view::npos?end:end-begin);
        while(true) {
            int width=0;std::size_t space=std::string_view::npos,index=0;
            for(;index<remaining.size();++index) {
                const auto character=static_cast<unsigned char>(remaining[index]);
                if(character<32 || character>126)
                    throw std::logic_error("Runtime dialogs require printable English text");
                const auto advance=glyph_advance(metrics[character],height);
                if(width+advance>available)break;
                if(character==' ')space=index;
                width+=advance;
            }
            if(index==remaining.size()){lines.push_back(remaining);break;}
            if(space==std::string_view::npos || space==0)
                throw std::logic_error("Runtime dialog word exceeds its rectangle");
            lines.push_back(remaining.substr(0,space));remaining.remove_prefix(space+1);
        }
        if(end==std::string_view::npos)break;
        begin=end+1;
        if(text[end]=='\r' && begin<text.size() && text[begin]=='\n')++begin;
    }while(begin<=text.size());
    const auto total=lines.size()*static_cast<std::size_t>(height);
    if(total>rectangle.bottom-rectangle.top)
        throw std::logic_error("Runtime dialog paragraph exceeds its rectangle");
    const float top=rectangle.top+std::max(0.0f,(rectangle.bottom-rectangle.top-total)*.5f);
    std::vector<CursorVertex> result;
    for(std::size_t i=0;i<lines.size();++i) {
        const MenuRectangle row{rectangle.left,top+i*height,rectangle.right,top+(i+1)*height};
        auto vertices=menu_label_vertices(lines[i],metrics,row,normalized_height,render_height,false,color);
        result.insert(result.end(),vertices.begin(),vertices.end());
    }
    return result;
}

struct MenuTextSubmission {
    bool small;
    std::vector<CursorVertex> vertices;
};

//00641E30 style19: BIG .04, left/top. Profile records form uniform
//nonwrapping rows; retain only complete visible rows in the text viewport.
inline std::vector<MenuTextSubmission> menu_profile_list_submissions(
        const frontend::TextObject& object,const MenuRectangle& area,std::size_t offset,
        int /*selected*/,int hovered,const FontMetrics& big) {
    if(!std::isfinite(area.left)||!std::isfinite(area.top)||!std::isfinite(area.right)||
       !std::isfinite(area.bottom)||area.right<=area.left||area.bottom<=area.top)
        throw std::invalid_argument("Invalid profile-list viewport");
    const auto rows=static_cast<std::size_t>(std::floor((area.bottom-area.top)/24.f));
    std::vector<MenuTextSubmission> result;
    for(std::size_t row=0;row<rows && offset<object.records().size() && row<object.records().size()-offset;++row) {
        const auto index=offset+row;const auto& record=object.records()[index];
        if(!record.text)continue;
        const auto* style=frontend::menu_text_style(record.style);
        if(!style||record.style!=19)throw std::logic_error("Unsupported profile-list record style");
        const MenuRectangle rectangle{area.left,area.top+row*24,area.right,area.top+(row+1)*24};
        const auto color=object.disabled?0x60000000u:
            hovered==static_cast<int>(index)?0xffffffffu:style->color;
        result.push_back({false,menu_label_vertices(*record.text,big,rectangle,.04f,600,false,color)});
    }
    return result;
}

//00641E30 styles27/28/29 use independent anchors .01/.50/.99 across
// the SAME rectangle, small CE font at .019 and alphaA0. They are not a
// three-line paragraph. Preserve the single-record path for ordinary labels.
template<class Bounds>
std::optional<MenuTextSubmission> menu_ruler_submission(const frontend::TextObject& object,
        Bounds bounds,const FontMetrics& metrics) {
    if (object.records().size()!=3 || object.records()[0].style!=27 ||
        object.records()[1].style!=28 || object.records()[2].style!=29) return {};
    MenuTextSubmission result{true,{}};
    const auto rectangle=menu_rectangle(bounds(),800,600,object.inset);
    for (unsigned i=0;i<3;++i) {
        const auto& record=object.records()[i];
        if(!record.text) continue;
        int width=0;
        for(unsigned char byte:*record.text) width+=glyph_advance(metrics[byte],11);
        auto position=rectangle;
        const float anchor=i==0?.01f:i==1?.5f:.99f;
        position.left=rectangle.left+(rectangle.right-rectangle.left)*anchor-
            (i==0?0:i==1?width*.5f:static_cast<float>(width));
        auto vertices=menu_label_vertices(*record.text,metrics,position,.019f,600,false,
                                          object.disabled?0x60000000u:0xa0000000u);
        result.vertices.insert(result.vertices.end(),vertices.begin(),vertices.end());
    }
    return result;
}

// Bounded glyph submission for one attached ASCII record. Bounds are lazy so
// empty dynamic labels/deferred version text do not request unused visuals.
// General multi-style paragraph layout and word_94 color semantics remain unresolved;
// current hover/unavailable colors preserve the desktop-tested host policy.
template<class Bounds>
std::optional<MenuTextSubmission> menu_text_submission(
        const frontend::TextObject& object, Bounds bounds, bool button,
        bool available, bool hovered, const FontMetrics& big, const FontMetrics& small) {
    if (object.records().empty()) return std::nullopt;
    //00627520 appends exactly three style7 records.00645270 makes one row
    // per record;006468D0 centers the complete block, including empty rows.
    if(object.records().size()==3 && object.records()[0].style==7 &&
       object.records()[1].style==7 && object.records()[2].style==7) {
        const auto area=menu_rectangle(bounds(),800,600,object.inset);
        MenuTextSubmission result{false,{}};
        const float top=area.top+std::max(0.f,(area.bottom-area.top-54.f)*.5f);
        for(unsigned row=0;row<3;++row) {
            const auto& record=object.records()[row];if(!record.text)continue;
            const MenuRectangle line{area.left,top+row*18,area.right,top+(row+1)*18};
            auto vertices=menu_label_vertices(*record.text,big,line,.03f,600,false,
                object.disabled?0x60000000u:0xff000000u);
            result.vertices.insert(result.vertices.end(),vertices.begin(),vertices.end());
        }
        return result;
    }
    if (object.records().size() != 1) throw std::logic_error("Multiple text records require original paragraph layout");
    const auto& record = object.records().front();
    const bool selected=object.selection_active && object.selected_text()==record.text;
    if ((!record.text || record.text->empty()) && !(selected && object.caret_visible)) return std::nullopt;
    const auto* style = frontend::menu_text_style(record.style);
    if (!style || !(((style->horizontal == 1 || style->horizontal == 2) && style->vertical == 2) ||
                    ((style->horizontal == 1 || style->horizontal == 3) && style->vertical == 3))) {
        throw std::logic_error("Unsupported original text style layout");
    }
    std::uint32_t color = object.disabled || (button && !available) ? 0x60000000 : style->color;
    if ((hovered && available && !object.disabled) || (button && object.word_94==0xffffffffu)) color = 0xffffffff;
    if(record.style==24 || record.style==25)
        return MenuTextSubmission{false, menu_prompt_vertices(*record.text,big,
            menu_rectangle(bounds(),800,600,object.inset),style->height,600,color)};
    //00647EE0/00648C80 draw the font's underscore after the selected record.
    //00646A10 reserves its width even during the off half of the blink cycle.
    // This path is the single English binding-value record, not a general edit
    // paragraph renderer; the existing actual selection clock controls blinking.
    auto value=record.text?*record.text:std::string{};
    auto rectangle=menu_rectangle(bounds(),800,600,object.inset);
    if(record.style==10) {
        //00641E30 style+18=.01;00646940 excludes the first row's leading
        //space from block height, but006469C0 adds it to the baseline.
        const auto leading=static_cast<int>(600.0f*.01f+.5f);
        rectangle.top+=leading;rectangle.bottom+=leading;
    } else if(style->vertical==3) {
        const int height=static_cast<int>(600.0*style->height+.5);
        rectangle.top=rectangle.bottom-height; // Style40 left/bottom.
    }
    const auto& metrics=style->font==3?small:big;
    if(style->horizontal==2) {
        int width=0;
        const int height=static_cast<int>(600.0*style->height+.5);
        for(unsigned char byte:value) width+=glyph_advance(metrics[byte],height);
        rectangle.left+=(rectangle.right-rectangle.left-width)*.5f;
    }
    if(selected) {
        const int height=static_cast<int>(600.0*style->height+.5);
        int width=glyph_advance(metrics[static_cast<unsigned char>('_')],height);
        for(unsigned char byte:value) width+=glyph_advance(metrics[byte],height);
        if(width>rectangle.right-rectangle.left) rectangle.left=rectangle.right-width;
        if(object.caret_visible) value.push_back('_');
    }
    return MenuTextSubmission{style->font == 3,
        menu_label_vertices(value, metrics,
            rectangle, style->height, 600,
            style->horizontal == 3, color)};
}

class ExitButton {
public:
    bool release(bool inside) {
        const bool activated = pressed_ && inside;
        pressed_ = false;
        return activated;
    }
    void press(bool inside) { pressed_ = inside; }
    void cancel() { pressed_ = false; }

private:
    bool pressed_ = false;
};

}

#endif
