#ifndef HD2_HOST_LOADING_TEXT_GEOMETRY_H
#define HD2_HOST_LOADING_TEXT_GEOMETRY_H

#include "host/menu_text_geometry.h"

namespace hd2::host {

struct LoadingTextGeometry {
    unsigned font=2;
    std::vector<CursorVertex> vertices;
};

//00641E30 styles80/84 used by the captured single-player loader art. Reuse
//the recovered glyph/word-wrap/justification/clipping bodies with their actual
//.045/.022 heights; style40 keeps its shared version layout. Multiplayer
//styles require their own descriptor producers/layout before becoming valid.
inline LoadingTextGeometry loading_text_geometry(const frontend::TextObject& text,
        const std::array<float,8>& bounds,const FontMetrics& big,const FontMetrics& small) {
    if(text.records().empty())return {};
    const auto rectangle=menu_rectangle(bounds,menu_width(),menu_height(),text.inset);
    const auto& record=text.records().front();
    LoadingTextGeometry result;
    MenuTextSubmission submission;
    if(record.style==84) {
        frontend::TextObject paragraphs;paragraphs.disabled=text.disabled;
        for(const auto& part:text.records()) {
            if(part.style!=84)throw std::runtime_error("Mixed unsupported loading paragraph styles");
            paragraphs.append(part.text,3);
        }
        result.font=3;submission.small=true;
        submission.vertices=menu_small_paragraph_vertices(paragraphs,small,rectangle,0,.022f);
    }else if(record.style==80) {
        if(text.records().size()!=1)throw std::runtime_error("Multiple unsupported loading title records");
        const auto value=record.text?menu_glyph_bytes(*record.text):std::string{};
        auto area=rectangle;const auto height=menu_line_height(.045f);int width=0;
        for(unsigned char c:value)width+=glyph_advance(big[c],height);
        area.left+=(area.right-area.left-width)*.5f;
        submission.vertices=menu_label_vertices(value,big,area,.045f,menu_height(),false,
            text.disabled?0x60000000u:0xff000000u);
    }else if(record.style==40) {
        if(text.records().size()!=1)throw std::runtime_error("Multiple unsupported loading version records");
        result.font=3;submission.small=true;auto area=rectangle;
        area.top=area.bottom-menu_line_height(.02f);
        submission.vertices=menu_label_vertices(record.text?*record.text:std::string{},small,
            area,.02f,menu_height(),false,text.disabled?0x60000000u:0xff000000u);
    }else throw std::runtime_error("Unsupported required loading text style");
    clip_menu_rectangle(submission,rectangle);
    result.vertices=std::move(submission.vertices);return result;
}

} //namespace hd2::host
#endif
