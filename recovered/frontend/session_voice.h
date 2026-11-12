#ifndef HD2_FRONTEND_SESSION_VOICE_H
#define HD2_FRONTEND_SESSION_VOICE_H
#include "frontend/settings_sound.h"
#include <list>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cstring>
namespace hd2::frontend {
using SessionVoiceGuid=std::array<std::uint32_t,4>;
inline constexpr SessionVoiceGuid voice_playback_device{{0xdef00002,0x47ed9c6d,0xda4df1aa,0x035c2b8f}}; //00819488.
inline constexpr SessionVoiceGuid voice_capture_device{{0xdef00003,0x47ed9c6d,0xda4df1aa,0x035c2b8f}}; //00819478.
struct SessionVoiceAudio {
    SessionVoiceGuid playback=voice_playback_device;void* playback_interface=nullptr;
    SessionVoiceGuid capture=voice_capture_device;std::uintptr_t window=0;
};
//00741DC0..00742270. COM allocations are the real objects returned by the
//platform factory. Settings/transmit consumers borrow this SAME native04 view.
template<class Backend>class SessionVoice {
public:
    struct Wrapper:SettingsControlInterface {void* server=nullptr;void* context=nullptr;std::uint32_t half_duplex=0;Wrapper(){object=nullptr;}};
    SessionVoice(SettingsControlInterface*& published,Backend& backend):published_(published),backend_(backend){}
    ~SessionVoice(){try{retire_all();}catch(...){}} //Explicit containing cleanup reports failures first.
    SessionVoice(const SessionVoice&)=delete;SessionVoice& operator=(const SessionVoice&)=delete;
    Wrapper& publish(){ //00724370 publishes BEFORE window lookup/initialization, without retiring previous0C.
        owners_.push_back(std::make_unique<Wrapper>());auto& result=*owners_.back();published_=&result;return result;
    }
    std::int32_t initialize(Wrapper& wrapper,std::uintptr_t window,bool server,bool client,void* connection,
            std::uint32_t mode,const SessionVoiceGuid* compression,SettingsControlBlock* block,void* playback_interface){ //00741E20 eight stack arguments.
        std::int32_t result;
        if(server&&(result=start_server(wrapper,connection,mode,compression))<0)return result;
        if(client){if((result=test_audio(window))<0)return result;if((result=start_client(wrapper,window,connection,block,playback_interface))<0)return result;}
        return 0;
    }
    void create(bool authority,void* server,void* client){ //00724370 deliberately ignores initializer HRESULT.
        auto& wrapper=publish();SettingsControlBlock block{{0x20,0x28,1,0,0xfffffffeu,0,0,0}};
        auto* connection=authority?server:client;const auto window=backend_.voice_window();
        (void)initialize(wrapper,window,false,true,connection,3,nullptr,&block,nullptr);
    }
    std::int32_t close(Wrapper& wrapper){ //00741E80; client then server; publication clears only AFTER Release.
        if(wrapper.object){(void)backend_.voice_disconnect(wrapper.object,1);if(wrapper.object){backend_.voice_release(wrapper.object);wrapper.object=nullptr;}}
        if(wrapper.server){(void)backend_.voice_disconnect(wrapper.server,0);if(wrapper.server){backend_.voice_release(wrapper.server);wrapper.server=nullptr;}}
        return 0;
    }
    void retire(){ //00724450→00741DF0/00741E10; actual published borrower survives the deleting receiver.
        if(!published_)return;
        for(auto it=owners_.begin();it!=owners_.end();++it)if(it->get()==published_){close(**it);owners_.erase(it);published_=nullptr;return;}
        throw std::logic_error("Voice retirement requires actual process0C owner");
    }
    void retire_all(){
        retire();
        //Native repeated creation loses the old pointer. Source safety reclaims
        //those abandoned allocations only at process fallback retirement.
        while(!owners_.empty()){close(*owners_.back());owners_.pop_back();}
    }
private:
    std::int32_t test_audio(std::uintptr_t window){ //00741EF0; DirectPlayVoiceTest is a temporary COM owner.
        void* test=nullptr;auto result=backend_.voice_create(0,test);if(result<0)return result;
        result=backend_.voice_test(test,voice_playback_device,voice_capture_device,window,2);
        if(static_cast<std::uint32_t>(result)==0x80150183u)result=backend_.voice_test(test,voice_playback_device,voice_capture_device,window,0x10);
        if(test)backend_.voice_release(test);
        return result;
    }
    std::int32_t start_server(Wrapper& wrapper,void* connection,std::uint32_t mode,const SessionVoiceGuid* compression){ //00741FE0.
        auto result=backend_.voice_create(1,wrapper.server);if(result<0)return result;
        result=backend_.voice_initialize(wrapper.server,connection,false,wrapper.context);if(result<0)return result;
        if(!compression)throw std::logic_error("Voice server requires original compression GUID");
        std::array<std::uint32_t,9> session{{0x24,0,mode,(*compression)[0],(*compression)[1],(*compression)[2],(*compression)[3],0,0}};
        result=backend_.voice_start_server(wrapper.server,session);return result<0?result:0;
    }
    std::int32_t start_client(Wrapper& wrapper,std::uintptr_t window,void* connection,SettingsControlBlock* block,void* playback_interface){ //00742090.
        auto result=backend_.voice_create(2,wrapper.object);if(result<0)return result;
        result=backend_.voice_initialize(wrapper.object,connection,true,reinterpret_cast<void*>(window));if(result<0)return result;
        SessionVoiceAudio audio;audio.playback_interface=playback_interface;audio.window=window;
        result=backend_.voice_connect(wrapper.object,audio,block);if(result<0)return result;
        const std::uint32_t target=0;result=backend_.voice_targets(wrapper.object,&target,1,0);if(result<0)return result;
        std::uint32_t size=0;result=backend_.voice_audio_config(wrapper.object,nullptr,size);
        if(result<0&&static_cast<std::uint32_t>(result)!=0x8015001eu)return result;
        if(size<8)throw std::logic_error("Voice audio configuration is shorter than required native words");
        std::vector<unsigned char> bytes(size,0);const std::uint32_t header=0x40;std::memcpy(bytes.data(),&header,4);
        auto* current=wrapper.object;result=backend_.voice_audio_config(current,bytes.data(),size);if(result<0)return result;
        std::uint32_t value;std::memcpy(&value,bytes.data()+4,4);wrapper.half_duplex=(value>>2)&1u;return 0;
        //Source RAII also frees failed query allocation; native negative second query leaks it.
    }
    SettingsControlInterface*& published_;Backend& backend_;std::list<std::unique_ptr<Wrapper>> owners_;
};
}
#endif
