#ifndef HD2_GAME_EFFECTS_DECORATIONS_LOADING_H
#define HD2_GAME_EFFECTS_DECORATIONS_LOADING_H

#include "game/serialized_bytes.h"
#include "frontend/pre_mission_data.h"
#include <memory>
#include <optional>
#include <vector>
#include <limits>

namespace hd2::game {

//Actual GEffects+20 cold decoration definitions/grid. Loading creates names,
//groups, cells and a spatial byte mask; model/resource/grass-generation owners
//are produced later. There is no successful rendering or model-spawn API here.
class EffectsDecorationsLoading {
public:
    class RuntimeModel {public:virtual ~RuntimeModel()=default;virtual void visible(bool)=0;virtual void update()=0;};
    struct Definition {std::string name;std::vector<std::unique_ptr<RuntimeModel>> models;~Definition(){while(!models.empty())models.pop_back();}};
    struct Entry {
        float weight=1,field4=0,field14=.2f,field18=1;
        std::string name;Definition* definition=nullptr;
        bool flag10=false;std::uint8_t enabled11=1,enabled12=1,flag13=0; //Serialized bytes retain all bits.
        std::uint32_t field40=0;std::unique_ptr<RuntimeModel> model;
        //Native24/30 resource arrays have no source producer in this cold path.
    };
    struct Group {
        std::vector<std::unique_ptr<Entry>> entries;float field10=.949999988079071f,field14=.5f;
        ~Group(){while(!entries.empty())entries.pop_back();}
    };
    struct Cell {std::uint32_t field0=0,fieldc=0;std::unique_ptr<RuntimeModel> model;std::int32_t identity=-1;bool flag10=false;}; //0047E1C0.
    struct Mask {
        std::uint32_t width=0,height=0;float step=0;std::array<float,3> origin{};float field80=0;
        std::vector<unsigned char> bytes;
    };
    const auto& groups()const{return groups_;}const auto& definitions()const{return definitions_;}
    const auto& cells()const{return cells_;}const auto& mask()const{return mask_;}
    const auto& active_entries()const{return active_entries_;}
    std::int32_t width()const{return width_;}std::int32_t height()const{return height_;}
    float density()const{return density_;}bool enabled()const{return enabled_;}bool quality()const{return quality_;}
    bool initial_first_person()const{return first_person_;}
    void read(SerializedBytes bytes){ //0047C530 and0047CC90; existing groups only replaced at supplied indices.
        if(enabled_||!cells_.empty())throw std::logic_error("Retire decorations before another loading read");
        auto root=next_mission_chunk(bytes);if(!root||root->id<=1)return;
        width_=signed_word(root->bytes.word());height_=width_;density_=root->bytes.real();
        while(root->bytes.remaining()){
            const auto index=signed_word(root->bytes.word());
            if(index<0||index>0x100000)throw std::runtime_error("Decoration group index exceeds native safe bounds");
            if(groups_.size()<=std::size_t(index))groups_.resize(std::size_t(index)+1);
            auto group=std::make_unique<Group>();
            //Original replaces/deletes the prior group BEFORE reading the new one.
            groups_[std::size_t(index)]=std::move(group);
            auto record=next_mission_chunk(root->bytes);if(!record)break; //0047CC90 returnsfalse if next chunk absent.
            auto& current=*groups_[std::size_t(index)];auto& data=record->bytes;
            const auto count=signed_word(data.word());
            if(root->id>2)current.field10=data.real();
            if(root->id>8)current.field14=data.real();
            for(std::int32_t n=0;n<count;++n){
                auto name=next_mission_chunk(data);if(!name)throw std::runtime_error("Decoration entry has no name chunk");
                auto entry=std::make_unique<Entry>();entry->name=name->bytes.section(std::min<std::size_t>(999,name->bytes.remaining())).text();
                current.entries.push_back(std::move(entry));auto& value=*current.entries.back();
                value.weight=data.real();value.enabled11=data.byte();
                if(root->id>3)value.field4=data.real();
                if(root->id>4)value.enabled12=data.byte();
                if(root->id>5){value.field40=data.word();value.field14=data.real();}
                if(root->id>6)value.field18=data.real();
                if(root->id>7)value.flag13=data.byte();
            }
        }
    }
    void read_mask(SerializedBytes bytes){ //0047C930, first chunk ID ignored.
        release_mask();auto root=next_mission_chunk(bytes);if(!root)throw std::runtime_error("Decoration mask has no header");
        auto& data=root->bytes;mask_.width=data.word();mask_.height=data.word();mask_.step=data.real();
        for(auto& axis:mask_.origin)axis=data.real();
        mask_.field80=data.real();
        const auto count=std::uint64_t(mask_.width)*mask_.height;
        if(count>128u*1024u*1024u||count>data.remaining())throw std::runtime_error("Invalid decoration mask dimensions or short byte payload");
        const auto* first=data.take(std::size_t(count));mask_.bytes.assign(first,first+std::size_t(count));
    }
    template<class Files>bool load(Files& files,const std::string& resource){
        const auto path="missions\\"+resource+"\\decorations.bin";
        auto bytes=frontend::pre_mission_detail::read_file(files,path.c_str());if(!bytes)return false;
        read(SerializedBytes(*bytes));release_mask();
        const auto mask_path="missions\\"+resource+"\\decorator_sl.bin";
        auto mask=frontend::pre_mission_detail::read_file(files,mask_path.c_str());if(mask)read_mask(SerializedBytes(*mask));
        return true;
    }
    void initialize(bool quality,bool first_person){ //0047B0B0,0047E0F0, actual profile4 and game58 OR59.
        if(enabled_||!cells_.empty())throw std::logic_error("Retire the prior decoration grid before initialization");
        for(auto group=groups_.size();group;)if(auto* current=groups_[--group].get())
            for(auto n=current->entries.size();n;){
                auto& entry=*current->entries[--n];auto name=entry.name;
                if(name.size()>=256)throw std::runtime_error("Decoration resource name exceeds native setup buffer");
                for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
                Definition* definition=nullptr;
                for(auto i=definitions_.size();i;)if(definitions_[--i]->name==name){definition=definitions_[i].get();break;}
                if(!definition){auto next=std::make_unique<Definition>();next->name=std::move(name);definition=next.get();definitions_.push_back(std::move(next));}
                entry.definition=definition;if(entry.field40)active_entries_.push_back(&entry);
            }
        if(width_>0){
            const auto count=std::uint64_t(std::uint32_t(width_))*std::uint32_t(height_);
            if(height_<=0||count>0x400000)throw std::runtime_error("Decoration cell grid exceeds native safe bounds");
            cells_.resize(std::size_t(count));cell_x_=cell_z_=100000;field40_=false;field44_=field48_=0;field3c_=.05f;first_person_=first_person;
        }
        enabled_=true;quality_=quality;
        if(!quality_)set_enabled(false); //0047B010, cold cells contain no engine model.
    }
    void set_enabled(bool enabled){
        if(enabled==enabled_||(!quality_&&enabled))return;
        //Native ascending y/x loop visits currently instantiated cell models.
        if(width_>0&&height_>0)for(std::int32_t y=0;y<height_;++y)for(std::int32_t x=0;x<width_;++x){
            auto& cell=cells_.at(std::size_t(y)*std::size_t(width_)+std::size_t(x));
            if(cell.model){cell.model->visible(enabled);cell.model->update();}
        }
        enabled_=enabled;
    }
    void update_loading(bool quality){ //0047BCC0 verified profile transition and genuinely empty definition gates.
        if(quality_!=quality){quality_=quality;set_enabled(quality);}
        if(!enabled_||definitions_.empty())return;
        throw std::runtime_error("Decoration update requires actual0047BCC0 spatial/model producers");
    }
    void clear(){ //0047B090→0047EA60 then0047C020; cold pending grid requests are empty.
        cells_.clear();
        for(auto n=active_entries_.size();n;)active_entries_[--n]->model.reset();
        active_entries_.clear();
        while(!groups_.empty())groups_.pop_back();
        while(!definitions_.empty())definitions_.pop_back();
        release_mask();enabled_=false; //0047D0C0 leaves metadata scalars unchanged.
    }
    ~EffectsDecorationsLoading(){clear();}
    EffectsDecorationsLoading()=default;EffectsDecorationsLoading(const EffectsDecorationsLoading&)=delete;
    EffectsDecorationsLoading& operator=(const EffectsDecorationsLoading&)=delete;
private:
    void release_mask()noexcept{decltype(mask_.bytes){}.swap(mask_.bytes);} //0047D0C0 frees the actual byte buffer.
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    std::vector<std::unique_ptr<Group>> groups_;std::vector<std::unique_ptr<Definition>> definitions_;std::vector<Entry*> active_entries_;
    std::vector<Cell> cells_;Mask mask_;
    std::int32_t width_=60,height_=60,cell_x_=-1,cell_z_=-1;float density_=.5f,field3c_=.05f;
    std::uint32_t field44_=0,field48_=0;bool enabled_=false,quality_=false,first_person_=false,field40_=false;
};
} //namespace hd2::game
#endif
