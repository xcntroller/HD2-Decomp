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

//00648860 accepts legacy bytes and, in wide mode, decoded UTF-8 characters;
//006404C0 indexes the selected 256-entry atlas. The English ANSI slice can
//represent Latin-1 names (including Kratochvil's U+00ED) without transliteration.
//Keep raw legacy bytes when the string is not UTF-8, as HumanStore does.
inline std::string menu_glyph_bytes(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        const auto lead = static_cast<unsigned char>(text[i]);
        if (lead < 0x80) {
            result.push_back(static_cast<char>(lead));
            ++i;
            continue;
        }
        const unsigned count = lead >= 0xc2 && lead < 0xe0 ? 2
                             : lead >= 0xe0 && lead < 0xf0 ? 3
                             : lead >= 0xf0 && lead <= 0xf4 ? 4 : 0;
        if (!count || count > text.size() - i)
            return std::string(text);
        unsigned value = lead & ((1u << (7 - count)) - 1);
        for (unsigned j = 1; j < count; ++j) {
            const auto byte = static_cast<unsigned char>(text[i + j]);
            if ((byte & 0xc0) != 0x80)
                return std::string(text);
            value = (value << 6) | (byte & 0x3f);
        }
        if ((count == 2 && value < 0x80) || (count == 3 && value < 0x800) ||
            (count == 4 && value < 0x10000) || value > 0x10ffff ||
            (value >= 0xd800 && value <= 0xdfff))
            return std::string(text);
        // Other alphabets need the original CE/other font owners, outside this
        // ANSI renderer. Keep their missing-glyph representation bounded.
        result.push_back(value <= 0xff ? static_cast<char>(value) : '?');
        i += count;
    }
    return result;
}

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
    const auto glyphs = menu_glyph_bytes(text);
    text = glyphs;
    int line_width = 0;
    for (unsigned char character : text) {
        if (character < 32) {
            throw std::invalid_argument("Menu label contains an unhandled control character");
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
    const auto glyphs=menu_glyph_bytes(text);
    text=glyphs;
    std::vector<std::string_view> lines;
    std::size_t begin=0;
    do {
        const auto end=text.find_first_of("\r\n",begin);
        auto remaining=text.substr(begin,end==std::string_view::npos?end:end-begin);
        while(true) {
            int width=0;std::size_t space=std::string_view::npos,index=0;
            for(;index<remaining.size();++index) {
                const auto character=static_cast<unsigned char>(remaining[index]);
                if(character<32)
                    throw std::logic_error("Runtime dialog contains an unhandled control character");
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
    bool symbols = false;
};

//00647EE0/00648860 retain the editor's viewport while rendering long text.
//Clip complete glyph quads and interpolate UVs for partially visible glyphs.
inline void clip_menu_rectangle(MenuTextSubmission& submission,const MenuRectangle& area) {
    if (area.right<=area.left || area.bottom<=area.top ||
        !std::isfinite(area.left) || !std::isfinite(area.top) ||
        !std::isfinite(area.right) || !std::isfinite(area.bottom) || submission.vertices.size()%6)
        throw std::invalid_argument("Invalid editable text viewport");
    std::vector<CursorVertex> output;
    for (std::size_t i=0;i<submission.vertices.size();i+=6) {
        const auto* glyph=submission.vertices.data()+i;
        const float left=glyph[0].x,right=glyph[2].x,top=glyph[1].y,bottom=glyph[0].y;
        const float x0=std::max(left,area.left),x1=std::min(right,area.right);
        const float y0=std::max(top,area.top),y1=std::min(bottom,area.bottom);
        if (x0>=x1 || y0>=y1) continue;
        const auto vertex=[&](float x,float y) {
            auto result=glyph[0]; result.x=x; result.y=y;
            result.u=glyph[0].u+(glyph[2].u-glyph[0].u)*(x-left)/(right-left);
            result.v=glyph[1].v+(glyph[0].v-glyph[1].v)*(y-top)/(bottom-top);
            return result;
        };
        const auto bl=vertex(x0,y1),tl=vertex(x0,y0),tr=vertex(x1,y0),br=vertex(x1,y1);
        output.insert(output.end(),{bl,tl,tr,br,bl,tr});
    }
    submission.vertices=std::move(output);
}

// Bitmap text is drawn after the 3D scene. Exclude the dropdown's viewport
// from underlying glyph quads so their labels cannot bleed through its panel.
inline void exclude_menu_rectangle(MenuTextSubmission& submission,const MenuRectangle& mask) {
    if (mask.right<=mask.left || mask.bottom<=mask.top ||
        !std::isfinite(mask.left) || !std::isfinite(mask.top) ||
        !std::isfinite(mask.right) || !std::isfinite(mask.bottom) || submission.vertices.size()%6)
        throw std::invalid_argument("Invalid menu text occlusion rectangle");
    std::vector<CursorVertex> output;
    for (std::size_t i=0;i<submission.vertices.size();i+=6) {
        const auto* glyph=submission.vertices.data()+i;
        const float left=glyph[0].x,right=glyph[2].x,top=glyph[1].y,bottom=glyph[0].y;
        if (right<=mask.left || left>=mask.right || bottom<=mask.top || top>=mask.bottom) {
            output.insert(output.end(),glyph,glyph+6); continue;
        }
        if (right<=left || bottom<=top) continue;
        const auto piece=[&](float x0,float y0,float x1,float y1) {
            if (x0>=x1 || y0>=y1) return;
            auto vertex=[&](float x,float y) {
                auto result=glyph[0]; result.x=x; result.y=y;
                result.u=glyph[0].u+(glyph[2].u-glyph[0].u)*(x-left)/(right-left);
                result.v=glyph[1].v+(glyph[0].v-glyph[1].v)*(y-top)/(bottom-top);
                return result;
            };
            const auto bl=vertex(x0,y1),tl=vertex(x0,y0),tr=vertex(x1,y0),br=vertex(x1,y1);
            output.insert(output.end(),{bl,tl,tr,br,bl,tr});
        };
        const float middle_left=std::max(left,mask.left),middle_right=std::min(right,mask.right);
        piece(left,top,std::min(right,mask.left),bottom);
        piece(std::max(left,mask.right),top,right,bottom);
        piece(middle_left,top,middle_right,std::min(bottom,mask.top));
        piece(middle_left,std::max(top,mask.bottom),middle_right,bottom);
    }
    submission.vertices=std::move(output);
}

// Native representation of0062F3E0's dynamically registered information
// styles. Anchors use the text plane;00645270 scales the width cap by
// the text owner's render width (+04), not the smaller plane's extent.
struct MenuTextCell {
    std::string text;
    unsigned font = 3;
    float height = .019f, anchor = 0, width = 1;
    bool right = false;
    std::uint32_t color = 0xff000000;
};
using MenuTextRow = std::vector<MenuTextCell>;
inline std::vector<MenuTextSubmission> menu_column_submissions(
        const std::vector<MenuTextRow>& rows, const MenuRectangle& area,
        std::size_t offset, float line_height, const FontMetrics& big,
        const FontMetrics& small, const FontMetrics& symbols) {
    if (!std::isfinite(line_height) || line_height <= 0 ||
        !std::isfinite(area.left) || !std::isfinite(area.top) ||
        !std::isfinite(area.right) || !std::isfinite(area.bottom) ||
        area.right <= area.left || area.bottom <= area.top)
        throw std::invalid_argument("Invalid column text viewport");
    std::vector<MenuTextSubmission> result;
    const float extent = area.right-area.left;
    for (std::size_t i=offset; i<rows.size(); ++i) {
        const float top=area.top+static_cast<float>(i-offset)*line_height;
        if (top+line_height>area.bottom) break;
        for (const auto& cell:rows[i]) {
            if ((cell.font!=2 && cell.font!=3 && cell.font!=4) ||
                !std::isfinite(cell.height) || cell.height<=0 || cell.height>1 ||
                !std::isfinite(cell.anchor) || cell.anchor<0 || cell.anchor>1 ||
                !std::isfinite(cell.width) || cell.width<0 || cell.width>1 || cell.text.size()>4096)
                throw std::invalid_argument("Invalid authored text column");
            const auto& metrics=cell.font==4?symbols:cell.font==3?small:big;
            const int height=static_cast<int>(cell.height*600+.5f);
            if (height<1) throw std::invalid_argument("Column glyph height rounds to zero");
            const float anchor=area.left+extent*cell.anchor;
            const float capacity=std::min(800.f*cell.width,
                cell.right?anchor-area.left:area.right-anchor);
            auto value=menu_glyph_bytes(cell.text);
            int used=0; std::size_t end=0;
            for (;end<value.size();++end) {
                auto byte=static_cast<unsigned char>(value[end]);
                if (byte<32 || byte==127) {byte=' '; value[end]=' ';}
                const auto advance=glyph_advance(metrics[byte],height);
                if (used+advance>capacity) break;
                used+=advance;
            }
            value.resize(end);
            const float left=cell.right?anchor-used:anchor;
            auto vertices=menu_label_vertices(value,metrics,
                {left,top,left+std::max(1,used),top+height},cell.height,600,false,cell.color);
            // Preserve the viewport when a glyph is taller than the last row.
            for (std::size_t v=0;v<vertices.size();v+=6) {
                const float bottom=std::min(area.bottom,vertices[v].y);
                const float fraction=std::clamp((bottom-vertices[v+1].y)/height,0.f,1.f);
                const float uv=vertices[v+1].v+(vertices[v].v-vertices[v+1].v)*fraction;
                for (const auto j:{0u,3u,4u}) {vertices[v+j].y=bottom; vertices[v+j].v=uv;}
            }
            result.push_back({cell.font==3,std::move(vertices),cell.font==4});
        }
    }
    return result;
}

//00645F20/00646A10/00647270: SMALL top-aligned paragraphs used by mode
//explanations and preparation descriptions. Records start new rows; wrapping
//keeps word boundaries, and alignment4 distributes spare width between words
//on wrapped rows. The final row of a paragraph retains ordinary left spacing.
inline std::vector<CursorVertex> menu_small_paragraph_vertices(
        const frontend::TextObject& object,const FontMetrics& metrics,
        const MenuRectangle& rectangle,std::size_t first_line=0) {
    struct Line {std::string text;bool justify;};
    std::vector<Line> lines;
    const float available=rectangle.right-rectangle.left;
    constexpr int height=12; //00641E30 entries2/3,.02 of600.
    if (!std::isfinite(available) || available<=0) throw std::invalid_argument("Invalid menu paragraph width");
    std::size_t characters=0;
    std::vector<frontend::TextRecord> paragraphs;
    for(const auto &record:object.records()) {
        if(record.style==15 && !paragraphs.empty() && paragraphs.back().style==15) {
            paragraphs.back().text=std::make_shared<const std::string>(
                (paragraphs.back().text?*paragraphs.back().text:std::string{})+
                (record.text?*record.text:std::string{}));
        } else paragraphs.push_back(record);
    }
    for (const auto& record:paragraphs) {
        if (record.style!=1 && record.style!=2 && record.style!=3 && record.style!=15) throw std::logic_error("Mixed unsupported menu paragraph styles");
        const std::string text=record.text?menu_glyph_bytes(*record.text):std::string{};
        characters+=text.size();
        if (characters>65536) throw std::length_error("Menu paragraph is too large");
        std::size_t begin=0;
        do {
            const auto end=text.find_first_of("\r\n",begin);
            std::string_view rest(text.data()+begin,(end==std::string::npos?text.size():end)-begin);
            while (true) {
                int width=0;
                std::size_t last_space=std::string_view::npos,index=0;
                for (;index<rest.size();++index) {
                    const auto character=static_cast<unsigned char>(rest[index]);
                    if (character<32) throw std::logic_error("Menu paragraph contains an unhandled control character");
                    if (width+glyph_advance(metrics[character],height)>available) break;
                    if (character==' ') last_space=index;
                    width+=glyph_advance(metrics[character],height);
                }
                if (index==rest.size()) {lines.push_back({std::string(rest),false});break;}
                if (last_space==std::string_view::npos || !last_space)
                    throw std::logic_error("Menu paragraph word exceeds its rectangle");
                lines.push_back({std::string(rest.substr(0,last_space)),record.style==1 || record.style==3});
                rest.remove_prefix(last_space+1);
            }
            if (end==std::string::npos) break;
            begin=end+1;
            if (text[end]=='\r' && begin<text.size() && text[begin]=='\n') ++begin;
        } while (begin<=text.size());
    }
    std::vector<CursorVertex> vertices;
    for (std::size_t i=first_line;i<lines.size();++i) {
        const float top=rectangle.top+static_cast<float>(i-first_line)*height;
        if (top>=rectangle.bottom) break;
        const auto& line=lines[i];
        unsigned spaces=0;
        int width=0;
        for (unsigned char character:line.text) {
            spaces+=character==' ';
            width+=glyph_advance(metrics[character],height);
        }
        const float extra=line.justify && spaces?(available-width)/spaces:0;
        float x=rectangle.left;
        const auto color=object.disabled?0x60000000u:0xff000000u;
        for (unsigned char character:line.text) {
            const auto advance=glyph_advance(metrics[character],height);
            const char byte=static_cast<char>(character);
            auto glyph=menu_label_vertices(std::string_view(&byte,1),metrics,
                {x,top,x+advance,top+height},.02f,600,false,color);
            //00648860 clips at the text viewport, preserving glyph UVs for
            //the final partial row rather than drawing over the frame border.
            if (!glyph.empty() && glyph[0].y>rectangle.bottom) {
                const float visible=(rectangle.bottom-glyph[1].y)/height;
                const float v=glyph[1].v+(glyph[0].v-glyph[1].v)*std::clamp(visible,0.f,1.f);
                for (const auto index:{0u,3u,4u}) {glyph[index].y=rectangle.bottom;glyph[index].v=v;}
            }
            vertices.insert(vertices.end(),glyph.begin(),glyph.end());
            x+=advance+(character==' '?extra:0);
        }
    }
    return vertices;
}

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
        for(unsigned char byte:menu_glyph_bytes(*record.text)) width+=glyph_advance(metrics[byte],11);
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
        bool available, bool hovered, const FontMetrics& big, const FontMetrics& small,
        std::size_t paragraph_offset=0) {
    if (object.records().empty()) return std::nullopt;
    //00641E30 entries30..34: server row fields share a plane but have
    // independent authored widths/anchors. Their values must never wrap into
    // another server row or draw over a neighbouring column.
    if (object.records().size()==5 && object.records()[0].style==30) {
        static constexpr float widths[]{.22f,.13f,.14f,.07f,.10f};
        static constexpr float anchors[]{.095f,.4f,.75f,.84f,.99f};
        const auto area=menu_rectangle(bounds(),800,600,0);
        const float extent=area.right-area.left;
        MenuTextSubmission result{false,{}};
        for (unsigned i=0;i<5;++i) {
            const auto& record=object.records()[i];
            if (record.style!=30+i) throw std::logic_error("Invalid server-row text styles");
            if (!record.text) continue;
            auto value=menu_glyph_bytes(*record.text);
            const float width=800.f*widths[i]; //00645270: owner render width+04.
            int used=0;
            std::size_t end=0;
            while (end<value.size()) {
                const auto character=static_cast<unsigned char>(value[end]);
                if (character<32) { value[end]=' '; }
                const auto advance=glyph_advance(big[static_cast<unsigned char>(value[end])],24);
                if (used+advance>width) break;
                used+=advance; ++end;
            }
            value.resize(end);
            const float anchor=area.left+extent*anchors[i];
            const float left=i<2?anchor:anchor-used;
            const MenuRectangle column{left,area.top,left+width,area.bottom};
            auto vertices=menu_label_vertices(value,big,column,.04f,600,false,
                object.disabled?0x60000000u:hovered?0xffffffffu:0xff000000u);
            result.vertices.insert(result.vertices.end(),vertices.begin(),vertices.end());
        }
        return result;
    }
    if (object.records().size()==1 && object.records()[0].style==9) {
        const auto& record=object.records()[0];
        if (!record.text) return std::nullopt;
        auto area=menu_rectangle(bounds(),800,600,object.inset);
        area.left+=(area.right-area.left)*.15f;
        area.bottom=area.top+15;
        return MenuTextSubmission{true,menu_label_vertices(*record.text,small,area,.025f,600,false,
            object.disabled?0x60000000u:0xff000000u)};
    }
    if (std::all_of(object.records().begin(),object.records().end(),
            [](const auto& record){return record.style==1 || record.style==2 || record.style==3 || record.style==15;}))
        return MenuTextSubmission{true,menu_small_paragraph_vertices(object,small,
            menu_rectangle(bounds(),800,600,object.inset),paragraph_offset)};
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
    if (!style || !(((style->horizontal == 1 || style->horizontal == 2 || style->horizontal == 3) && style->vertical == 2) ||
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
    auto value=record.text?menu_glyph_bytes(*record.text):std::string{};
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
    if(style->horizontal==3 && style->vertical==2) {
        const int height=static_cast<int>(600.0*style->height+.5);
        rectangle.bottom=rectangle.top+std::max(0.f,(rectangle.bottom-rectangle.top-height)*.5f)+height;
    }
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
    auto vertices=menu_label_vertices(value, metrics,
            rectangle, style->height, 600,
            style->horizontal == 3, color);
    if (selected && object.caret_visible && metrics[static_cast<unsigned char>('_')].aspect!=0 && vertices.size()>=6) {
        //00648C90..00648CB4 use trunc(text_y - glyph_height * .1f).
        //Unlike ordinary glyphs, the caret has this explicit baseline lift.
        //0080F550 is the original IEEE float0.1, not a border-clamp rule.
        const auto first=vertices.size()-6;
        float bottom=vertices[first].y;
        for (auto i=first;i<vertices.size();++i) bottom=std::max(bottom,vertices[i].y);
        const int height=static_cast<int>(600.0*style->height+.5);
        const float shift=bottom-static_cast<int>(static_cast<double>(bottom)-height*static_cast<double>(.1f));
        for (auto i=first;i<vertices.size();++i) vertices[i].y-=shift;
    }
    return MenuTextSubmission{style->font==3,std::move(vertices)};
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
