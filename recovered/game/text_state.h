#ifndef HD2_GAME_TEXT_STATE_H
#define HD2_GAME_TEXT_STATE_H

#include "frontend/text_object.h"
#include "game/text_layout.h"
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace hd2::game {

//00496CC0/0048ECF0: native representation of the initialized text owner.
// FontResources owns actual decoded metrics/engine atlases; TextLayout owns the
// recovered byte-font cache. Drawing is supplied by text_render.h's real backend.
template<class FontResources>
class TextState {
public:
    TextState(std::uint32_t width, std::uint32_t height, std::shared_ptr<FontResources> fonts)
        : width_(static_cast<std::uint16_t>(width)), height_(static_cast<std::uint16_t>(height)),
          fonts_(std::move(fonts)) {
        if (!fonts_) throw std::invalid_argument("Game text requires actual font resources");
        text.inset = 10; //+90
    }
    TextState(const TextState&) = delete;
    TextState& operator=(const TextState&) = delete;
    ~TextState() {
        //0048ECF0 releases its font reference before clearing layout/string data.
        fonts_.reset();
        layout.clear();
        text.clear();
    }
    std::uint16_t width() const { return width_; }
    std::uint16_t height() const { return height_; }
    const FontResources* fonts() const { return fonts_.get(); }
    //TextObject is the SAME logical record/selection owner. These native-offset
    //views mirror its existing recovered setters, never a second edit state.
    void synchronize_text_state(){
        bytes70[1]=text.disabled;word94=text.word_94;
        bytes80[0]=text.caret_visible;bytes80[1]=text.selection_active;
        word88=text.selected_index();word8c=text.hovered_record;
    }
    template<class Styles>bool rebuild_layout(Styles& styles){synchronize_text_state();return layout.rebuild(*this,styles,*fonts_);}
    frontend::TextObject text;
    TextLayout layout;
    std::array<std::uint32_t, 7> geometry8{};
    std::uint32_t word2c = 0, word94 = 0, word98 = 0;
    std::array<std::uint8_t, 5> bytes70{};
    std::uint16_t word76 = 0;
    std::uint32_t word78 = 0, word7c = 0;
    std::array<std::uint8_t, 2> bytes80{};
    std::uint32_t word84 = 0;
    std::int32_t word88 = -1, word8c = -1;
private:
    std::uint16_t width_, height_;
    std::shared_ptr<FontResources> fonts_;
};

// Inlined00698510+60 constructor,0048F400/0048EF40/0069F130 and0048EFC0.
// The renderer is absent until mission text is actually initialized by0048EB70.
// It owns its font reference then, rather than allocating a fictitious renderer
// in the empty constructor. Owned entries release in reverse, preserving capacity
// during clear; their own string records release forward.
template<class Text>
class MissionText {
public:
    struct Entry {
        std::uint32_t cursor=0xffffffffu,id=0xffffffffu; //Native00/04; records0C.
        std::vector<std::pair<std::uint32_t, frontend::SharedText>> records;
        ~Entry() { for (auto& record : records) record.second.reset(); }
    };
    MissionText() = default;
    MissionText(const MissionText&) = delete;
    MissionText& operator=(const MissionText&) = delete;
    ~MissionText() { clear(); }
    void reset_selection() {
        index = -1;
        ticks = 0;
        if (renderer) renderer->text.clear();
    }
    void clear() {
        reset_selection();
        renderer.reset();
        enabled = 0;
        while (!entries.empty()) entries.pop_back();
    }
    bool drawable(bool subtitles) const {
        return enabled && visible && index >= 0 && renderer && subtitles;
    }
    std::uint8_t byte0 = 0, enabled = 0, visible = 0, byte8 = 0;
    std::unique_ptr<Text> renderer;
    std::int32_t index = -1, ticks = -1;
    frontend::SharedText mission_name;
    frontend::SharedText current; //Native14, retained independently of renderer records.
    std::vector<std::unique_ptr<Entry>> entries;
};

} // namespace hd2::game
#endif
