#ifndef HD2_GAME_MISSION_SUBTITLES_LOADING_H
#define HD2_GAME_MISSION_SUBTITLES_LOADING_H

#include "game/text_state.h"
#include "game/serialized_bytes.h"
#include <algorithm>
#include <cstring>
#include <limits>

namespace hd2::game {

//0048EB70/0048F7D0/0048F420: SAME game+60 MissionText, actual live Files
//and shared text/font factory. Dialogs.bin records are not movie subtitles.
template<class Text>class MissionSubtitlesLoading {
public:
    explicit MissionSubtitlesLoading(MissionText<Text>& state):state_(state){}
    const char* speech_caption(std::int32_t identity,const char* missing_caption)const { //004900E0, first matching ID, first text; no selection/renderer mutation.
        if(identity<90000000)return missing_caption;
        const auto index=find(static_cast<std::uint32_t>(identity));if(index==missing)return missing_caption;
        const auto& records=state_.entries[index]->records;
        if(records.empty())return "NO TEXT WITH THIS ID !!!";
        return records.front().second?records.front().second->c_str():"";
    }
    bool read(SerializedBytes bytes) {
        const auto root=next_mission_chunk(bytes);if(!root)return false;
        bool valid=true;
        if(root->id!=0x9c4)return valid; //Original unknown root yields success with no entries.
        auto content=root->bytes;
        while(auto chunk=next_mission_chunk(content)) {
            if(chunk->id==0x9c5)valid=(chunk->bytes.word()==1)&&valid;
            else if(chunk->id==0x9c6)valid=read_entry(chunk->bytes)&&valid;
        }
        return valid;
    }
    template<class Files>bool load(Files& files,const std::string& text_directory) {
        const auto path=text_directory+"Dialogs.bin"; //0048FBC0 supplies actual Text\<language>\ prefix.
        const auto handle=files.open(path.c_str(),0);if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}}close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid mission subtitle file size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind mission subtitle file");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Incomplete mission subtitle file");
        return read(SerializedBytes(bytes)); //Failed native version/record keeps preceding entries.
    }
    template<class Files,class Factory>bool initialize(Files& files,const std::string& text_directory,bool wide,Factory& factory) {
        if(state_.renderer||state_.enabled)throw std::logic_error("Mission subtitle renderer initialized twice");
        if(!load(files,text_directory))return false;
        state_.renderer=factory.make_hud_text(); //Same font/metrics owner as actual game text, not a second text backend.
        if(!state_.renderer)return false;
        state_.byte8=wide?26:8;state_.enabled=1;return true;
    }
    void show(bool visible,bool full_viewport,std::uint32_t width,std::uint32_t height) { //0048EE50/00644B80.
        if(state_.renderer&&visible) {
            state_.byte0=full_viewport;
            const auto w=static_cast<long double>(signed_word(width)),h=static_cast<long double>(signed_word(height));
            const auto margin=w*.06666666666666667; //Native QWORD00810488, not a FLOAT coefficient.
            const float left=static_cast<float>(margin),right=static_cast<float>(w-margin);
            const float bottom=static_cast<float>(h*(full_viewport?.99:.85));
            const float top=static_cast<float>(h*(full_viewport?.88:.74));
            const std::array<float,6> geometry{{left,bottom,right,top,static_cast<float>(static_cast<long double>(right)-left),static_cast<float>(static_cast<long double>(bottom)-top)}};
            auto& text=*state_.renderer;
            for(unsigned n=0;n!=geometry.size();++n)std::memcpy(&text.geometry8[n],&geometry[n],4);
            text.geometry8[6]=0x3cf6b2e5u;text.bytes70[2]=text.bytes70[0]=1;text.text.dirty=true;
        }
        state_.visible=visible;state_.index=-1;
    }
    void select(std::uint32_t id,bool full_viewport,std::uint32_t width,std::uint32_t height) { //0048F080.
        if(!state_.enabled)return;
        if(!state_.visible)show(true,full_viewport,width,height);
        if(const auto index=find(id);index!=missing) {
            auto& entry=*state_.entries[index];entry.cursor=0xffffffffu;
            if(next(entry)){state_.ticks=std::max(state_.ticks,1000);publish();state_.index=static_cast<std::int32_t>(index);return;}
        }
        clear_renderer();state_.index=-1;
    }
    void speech(std::uint32_t id,std::uint32_t duration) { //0048F180; does NOT implicitly show visibility.
        if(!state_.enabled)return;
        if(const auto index=find(id);index!=missing) {
            auto& entry=*state_.entries[index];entry.cursor=0xffffffffu;
            if(next(entry)&&(!state_.current||state_.current->find("NOSUB")==std::string::npos)) {
                if(duration!=0xffffffffu)state_.ticks=signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(static_cast<long double>(duration)*1.2000000476837158f)));
                state_.ticks=std::max(state_.ticks,1500);publish();state_.index=static_cast<std::int32_t>(index);return;
            }
        }
        clear_renderer();state_.index=-1;
    }
    void tick(std::uint32_t elapsed) { //0048F2D0, ONE next row, discard overshoot and do not reapply minimum duration.
        if(!state_.enabled||!state_.visible||state_.index<0)return;
        state_.ticks=signed_word(static_cast<std::uint32_t>(state_.ticks)-elapsed);
        if(state_.ticks>0)return;
        auto& entry=state_.entries.at(static_cast<std::size_t>(state_.index));
        if(!entry)throw std::runtime_error("Mission subtitle current entry has no actual owner");
        if(next(*entry)){publish();return;}
        state_.reset_selection();
    }
    template<class Backend>void draw(bool subtitles,Backend& backend) { //0048F390, preserve Graph state0 around actual text render.
        if(!state_.drawable(subtitles))return;
        const auto previous=backend.hud_graphics_state(0);backend.hud_graphics_state(0,1);
        try{backend.hud_draw_text(*state_.renderer);}catch(...){backend.hud_graphics_state(0,previous);throw;}
        backend.hud_graphics_state(0,previous);
    }
private:
    static constexpr std::size_t missing=std::numeric_limits<std::size_t>::max();
    bool read_entry(SerializedBytes bytes) { //0048F420 expects FIRST id chunk, THEN text chunk; trailing content skipped.
        auto identity=next_mission_chunk(bytes);if(!identity||identity->id!=0x9c7)return false;
        const auto id=identity->bytes.word();auto caption=next_mission_chunk(bytes);
        if(!caption||caption->id!=0x9c8)return false;
        auto text=caption->bytes.section(std::min<std::size_t>(999,caption->bytes.remaining())).text();
        auto entry=std::make_unique<typename MissionText<Text>::Entry>();entry->id=id;
        const auto duration=static_cast<std::uint32_t>(text.size())*200u;
        entry->records.emplace_back(duration,std::make_shared<const std::string>(std::move(text)));
        state_.entries.push_back(std::move(entry));return true;
    }
    std::size_t find(std::uint32_t id)const {
        for(std::size_t n=0;n!=state_.entries.size();++n) {
            const auto& entry=state_.entries[n];if(!entry)throw std::runtime_error("Mission subtitle list has a null native entry");
            if(entry->id==id)return n;
        }
        return missing;
    }
    bool next(typename MissionText<Text>::Entry& entry) {
        ++entry.cursor;if(entry.cursor>=entry.records.size())return false;
        const auto& row=entry.records[entry.cursor];state_.ticks=signed_word(row.first);state_.current=row.second;return true;
    }
    void publish(){if(!state_.renderer)throw std::runtime_error("Enabled mission subtitles require the actual renderer");state_.renderer->text.set(state_.current,state_.byte8,0);}
    void clear_renderer(){if(!state_.renderer)throw std::runtime_error("Enabled mission subtitles require the actual renderer");state_.renderer->text.clear();}
    static std::int32_t signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
    MissionText<Text>& state_;
};
} //namespace hd2::game
#endif
