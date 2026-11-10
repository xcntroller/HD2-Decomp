#ifndef HD2_FRONTEND_DUBBING_SOUND_H
#define HD2_FRONTEND_DUBBING_SOUND_H

#include "frontend/sound_runtime.h"
#include <cstdio>
#include <exception>
#include <utility>

namespace hd2::frontend {

//Sixteen process-shared68-byte channels at008CB7D0. Typed frame borrowers and
//owned data replace original pointer DWORDs. Separate from SOUND's voice pool.
//Backend/files outlive this owner; retire bound actors before their frames.
template<class Backend,class Files>
class DubbingSound {
public:
    struct Channel {
        std::vector<unsigned char> data; //+00; original raw file, including header.
        std::uint32_t position=0,timer=0,state=0,word10=0;
        void* sound=nullptr; //+14 owned, except native state5's externally borrowed sound.
        void* model=nullptr;void* morph=nullptr; //+18/+1C borrowers.
        std::array<void*,2> attachments{}; //+24 then+20, original ordering.
        std::array<float,6> pose{}; //+28..3C, first/fifth are swapped in file order.
        float blend=0; //+40
    };
    struct MorphFade {void* morph=nullptr;std::uint32_t duration=0,elapsed=0;std::array<std::uint8_t,4> slots{};};
    DubbingSound(Backend& backend,Files& files):backend_(backend),files_(files) {} //0071DE10's zero state.
    ~DubbingSound(){clear(false);}
    DubbingSound(const DubbingSound&)=delete;
    DubbingSound& operator=(const DubbingSound&)=delete;
    void initialize_loading(){ //0071DE10, actual16x68 zero and morph-fade end=begin; not stop_model/clear.
        for(const auto& channel:channels_)if(channel.sound||!channel.data.empty())
            throw std::logic_error("Retire actual owned dubbing channels before loading initialization");
        for(auto& channel:channels_)channel={};
        fades_.clear(); //Borrowed pending morph records, native capacity survives. No neutral/playback call.
    }
    unsigned free_channel() const { //0071DEF0 searches15 down to0, overwrites15 if full.
        for(unsigned n=16;n!=0;)if(!channels_[--n].state)return n;
        return 15;
    }
    template<class Services>bool play_speech(void* actor,void* model,std::int32_t id,float near_range,
            float far_range,bool subtitles,const std::uint32_t& gate,Services& services) { //00436470.
        if(!(gate&2u))return false;
        auto* actual=actor?services.dubbing_actor_model(actor):model;
        if(!actual)throw std::runtime_error("Dubbing speech requires its actual frame");
        if(!id){stop_model(actual);return true;}
        const auto slot=free_channel();
        bind(slot,actual,actor?services.dubbing_actor_attachments(actor):std::array<void*,2>{},actor!=nullptr);
        const bool player=actor==services.dubbing_current_player(); //Original compares even when actor is null.
        play(slot,id,near_range,far_range,subtitles,player,services);return true;
    }
    void bind(unsigned slot,void* model,const std::array<void*,2>& attachments,bool human) { //0071DF30/0071E490.
        if(slot>=16||!model)return;
        auto& channel=channels_[slot];channel.model=model;
        if(human)channel.attachments=attachments;
        auto* morph=backend_.dubbing_find(model,"a",0xffff);
        if(morph&&backend_.dubbing_visual_type(morph)==0x524d4d53u) {
            channel.morph=morph;if(!human)channel.attachments={};neutral(morph);
            if(human) {
                channel.pose={{.46f,0,1,1,.11f,.6f}};
                apply_pose(channel,channel.pose);
            }
        }else channel.morph=nullptr;
        channel.state=1;
    }
    template<class Services>void play(unsigned slot,std::int32_t id,float near_range,float far_range,
            bool subtitles,bool player,Services& services) { //0071E860/0071E7F0.
        if(slot>=16)return;
        auto& channel=channels_[slot];
        try {
            channel.data.clear();
            const auto data_name=formatted("tables\\dabing\\%08d.dat",id);
            const auto handle=files_.open(data_name.c_str(),0);
            if(static_cast<std::int32_t>(handle)>=0) {
                struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files_,handle};
                const auto size=files_.seek(handle,0,2);
                if(size==0xffffffffu)throw std::runtime_error("Cannot size actual dubbing data");
                if(size) {
                    if(size<8||size>0x7fffffffu)throw std::runtime_error("Dubbing data has no complete header");
                    if(files_.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind dubbing data");
                    channel.data.resize(size);
                    if(files_.read(handle,channel.data.data(),size)!=size)throw std::runtime_error("Incomplete dubbing data");
                    const auto count=rows(channel);
                    if(count<2||count>(size-8)/8)throw std::runtime_error("Dubbing pose table is truncated");
                }
            }else backend_.dubbing_missing_data(id);
            if(!channel.sound)channel.sound=backend_.create_dubbing_sound();
            if(!channel.sound)throw std::runtime_error("Cannot create actual dubbing sound frame");
            const auto name=formatted("sounds\\%08d.wav",id); //00844124's sole actual directory.
            if(backend_.open_dubbing_sound(channel.sound,name,0)!=0) {
                auto* sound=std::exchange(channel.sound,nullptr);backend_.release_catalog_sound(sound);
            }
            channel.state=2;
            if(subtitles)services.dubbing_subtitle(id,channel.sound?backend_.dubbing_sound_length(channel.sound):0xffffffffu);
            if(channel.sound) {
                backend_.dubbing_sound_word128(channel.sound,0x80);
                auto* marker=channel.morph?backend_.dubbing_find(channel.morph,"f",0xffff):nullptr;
                backend_.link_dubbing_sound(channel.sound,marker?marker:channel.model);
                backend_.position_dubbing_sound(channel.sound,marker?SoundPosition{{0,0,.1f,0}}:SoundPosition{{0,1.6f,0,0}});
                backend_.direction_dubbing_sound(channel.sound,{{0,0,1,0}});
                backend_.type_dubbing_sound(channel.sound,player?3u:1u);
                backend_.range_dubbing_sound(channel.sound,near_range,far_range,.3f,.25f);
                backend_.frequency_dubbing_sound(channel.sound,1);
                backend_.sound_volume(channel.sound,1);backend_.enable_sound(channel.sound,0);
                backend_.seek_dubbing_sound(channel.sound,0);backend_.update_sound(channel.sound);
                backend_.sound_mode(channel.sound,1,1);
            }
            channel.timer=200;channel.position=0;
        }catch(...){retire(slot,true);throw;}
    }
    void stop(unsigned slot) { //0071ECF0: schedules, retains sound/data until retirement.
        if(slot>=16)return;
        auto& channel=channels_[slot];if(channel.sound)backend_.sound_mode(channel.sound,0,1);
        channel.timer=1;channel.state=3;
    }
    void stop_model(void* model) {for(unsigned n=0;n!=16;++n)if(channels_[n].state==2&&channels_[n].model==model)stop(n);} //0071E460.
    bool playing(void* model) const {for(const auto& channel:channels_)if(channel.state==2&&channel.model==model)return true;return false;} //0071ECC0.
    void retire_model(void* model) noexcept { //0071E650; model null does nothing.
        if(!model)return;
        for(unsigned n=0;n!=16;++n)if(channels_[n].model==model)retire(n,true);
    }
    void clear(bool reset_morph=true) noexcept { //0071DE60(false)/0071E2B0(true).
        for(unsigned n=0;n!=16;++n)retire(n,reset_morph);
        fades_.clear();
    }
    void synchronize_attachments(){ //0071ED30, ascending channels; native+20 then+24.
        for(std::size_t channel=0;channel<channels_.size();++channel){
            if(auto* frame=channels_[channel].attachments[1])backend_.synchronize_dubbing_attachment(frame);
            if(auto* frame=channels_[channel].attachments[0])backend_.synchronize_dubbing_attachment(frame);
        }
    }
    void update(std::uint32_t elapsed) { //0071EE00, native unsigned millisecond/timer arithmetic.
        bool resume=false;if(resume_countdown_>0&&--resume_countdown_<1)resume=true;
        for(auto& channel:channels_) {
            if(!channel.state)continue;
            if(!channel.sound)channel.position+=elapsed;
            std::uint32_t row=0;
            if(channel.state==2) {
                bool ended=false;
                if(!channel.sound){row=channel.position/20;ended=channel.data.empty()||rows(channel)-1<=row;}
                else if(!backend_.sound_active(channel.sound,0))ended=true;
                else if(resume)backend_.seek_dubbing_sound(channel.sound,channel.position);
                else {
                    const auto position=backend_.dubbing_sound_position(channel.sound);
                    if(position<channel.position&&backend_.dubbing_sound_length(channel.sound)<channel.position+300u)ended=true;
                    else {channel.position=position;row=position/20;}
                }
                if(ended){channel.timer=200;channel.state=3;}
            }
            switch(channel.state) {
            case 5:
                if(!channel.sound)break;
                channel.position=backend_.dubbing_sound_position(channel.sound);row=channel.position/20;
                [[fallthrough]];
            case 2:
                if(!channel.data.empty()&&channel.morph) {
                    const auto count=rows(channel);if(row>=count)row=count-2;
                    auto pose=pose_row(channel,row);
                    if(channel.blend<1) {
                        channel.blend=static_cast<float>(static_cast<long double>(elapsed)*.008f+channel.blend);
                        if(channel.blend<1)for(unsigned n=0;n!=6;++n)pose[n]=static_cast<float>(
                            static_cast<long double>(channel.blend)*pose[n]+(1.L-channel.blend)*channel.pose[n]);
                    }
                    apply_pose(channel,pose);
                }
                break;
            case 3:
                channel.timer=elapsed<channel.timer?channel.timer-elapsed:0;
                if(!channel.timer)channel.state=0;
                if(!channel.data.empty()&&channel.morph) {
                    auto pose=pose_row(channel,rows(channel)-2);
                    //Native stores these first two values after extended products.
                    const auto offset=std::size_t{8}+std::size_t(rows(channel)-2)*8;
                    const auto factor=static_cast<long double>(channel.timer)*.005f;
                    for(unsigned n=0;n!=2;++n)pose[n]=static_cast<float>(
                        static_cast<long double>(channel.data[offset+(n==0?1:0)])*.003921568859368563f*factor);
                    channel.pose=pose;apply_pose(channel,pose);
                }
                break;
            case 4:channel.timer=elapsed<channel.timer?channel.timer-elapsed:0;if(!channel.timer)channel.state=0;break;
            default:break;
            }
        }
        for(std::size_t n=0;n<fades_.size();) {
            auto& fade=fades_[n];
            for(unsigned slot=0;slot!=8;++slot)for(unsigned group=0;group!=2;++group)weight(fade.morph,group,slot,0);
            fade.elapsed+=elapsed;
            if(fade.elapsed>fade.duration) {
                weight(fade.morph,0,fade.slots[1],1);weight(fade.morph,1,fade.slots[3],1);
                fades_.erase(fades_.begin()+static_cast<std::ptrdiff_t>(n));
            }else {
                const float ratio=static_cast<float>(static_cast<long double>(static_cast<float>(fade.elapsed))/static_cast<float>(fade.duration));
                const float previous=1.f-ratio;
                weight(fade.morph,0,fade.slots[0],previous);weight(fade.morph,1,fade.slots[2],previous);
                weight(fade.morph,0,fade.slots[1],ratio);weight(fade.morph,1,fade.slots[3],ratio);++n;
            }
        }
    }
    const auto& channels() const{return channels_;}
private:
    static std::string formatted(const char* format,std::int32_t id) {std::array<char,64> text{};std::snprintf(text.data(),text.size(),format,id);return text.data();}
    static std::uint32_t rows(const Channel& channel) {
        if(channel.data.size()<4)throw std::runtime_error("Dubbing data header is absent");
        const auto* p=channel.data.data();return p[0]|(std::uint32_t(p[1])<<8)|(std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);
    }
    static std::array<float,6> pose_row(const Channel& channel,std::uint32_t row) {
        const auto offset=std::uint64_t{8}+std::uint64_t{row}*8;
        if(offset>channel.data.size()||6>channel.data.size()-offset)throw std::runtime_error("Dubbing pose row exceeds original file");
        const auto* p=channel.data.data()+offset;
        //0080F6EC, FLOAT1/255; original+28/+2C are file bytes1/0.
        return {{p[1]*.003921568859368563f,p[0]*.003921568859368563f,p[2]*.003921568859368563f,
            p[4]*.003921568859368563f,p[3]*.003921568859368563f,p[5]*.003921568859368563f}};
    }
    void weight(void* morph,unsigned group,unsigned slot,float value){backend_.dubbing_morph_weight(morph,group,slot,value);}
    void neutral(void* morph) {for(unsigned group=0;group!=3;++group)for(unsigned slot=0;slot!=5;++slot)weight(morph,group,slot,slot?0.f:1.f);}
    void apply_pose(const Channel& channel,const std::array<float,6>& p) {
        //Native operations keep each complement/product extended until call's float store.
        auto* morph=channel.morph;weight(morph,0,0,0);
        weight(morph,0,1,static_cast<float>(static_cast<long double>(p[0])*p[1]));
        weight(morph,0,2,static_cast<float>((1.L-p[0])*p[1]));
        const float first_rest=1.f-p[1];
        weight(morph,0,3,static_cast<float>((1.L-p[2])*first_rest));
        weight(morph,0,4,static_cast<float>(static_cast<long double>(first_rest)*p[2]));
        weight(morph,1,0,0);weight(morph,1,1,static_cast<float>(static_cast<long double>(p[4])*p[3]));
        weight(morph,1,2,static_cast<float>((1.L-p[4])*p[3]));const float rest=1.f-p[3];weight(morph,1,3,rest);
        weight(morph,2,0,0);weight(morph,2,1,static_cast<float>(static_cast<long double>(p[5])*p[3]));
        weight(morph,2,2,static_cast<float>((1.L-p[5])*p[3]));weight(morph,2,3,rest);
    }
    template<class Work>void attempt(Work&& work) noexcept {
        try{work();}catch(...){backend_.record_dubbing_cleanup_failure(std::current_exception());}
    }
    void retire(unsigned slot,bool reset_morph) noexcept {
        auto& channel=channels_[slot];
        if(reset_morph&&channel.morph)attempt([&]{neutral(channel.morph);});
        if(channel.sound&&channel.state!=5)attempt([&]{backend_.release_catalog_sound(channel.sound);});
        channel={};
    }
    Backend& backend_;Files& files_;std::array<Channel,16> channels_{};
    std::vector<MorphFade> fades_;std::int32_t resume_countdown_=0;
};
} //namespace hd2::frontend
#endif
