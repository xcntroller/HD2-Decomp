#ifndef HD2_HOST_DUBBING_SOUND_SERVICES_H
#define HD2_HOST_DUBBING_SOUND_SERVICES_H

#include "host/sound_services.h"
#include "host/menu_scene.h"

namespace hd2::host {

//Actual engine receivers used by the shared16-channel dubbing owner. Files,
//actor selection and subtitles are supplied by StartupSession/containing world.
class DubbingSoundServices : public SoundServices {
public:
    DubbingSoundServices(HMODULE engine,frontend::SoundWorldView*& world):SoundServices(engine,world),engine_(engine) {}
    void* create_dubbing_sound(){return call<EngineFrame*>(driver(),0x54,std::uint32_t{4});}
    std::int32_t open_dubbing_sound(void* frame,const std::string& name,std::uint32_t flags) {return call<std::int32_t>(frame,0x74,name.c_str(),flags);}
    void* dubbing_find(void* frame,const char* name,std::uint32_t flags){return call<void*>(frame,0x48,name,flags);}
    std::uint32_t dubbing_visual_type(void* frame){return call<std::uint32_t>(frame,4);}
    void dubbing_morph_weight(void* frame,std::uint32_t group,std::uint32_t slot,float value) {
        call<void>(static_cast<unsigned char*>(frame)+0x120,8,group,slot,value);
    }
    std::uint32_t dubbing_sound_length(void* frame){return call<std::uint32_t>(frame,0xa8);}
    std::uint32_t dubbing_sound_position(void* frame){return call<std::uint32_t>(frame,0xa4);}
    void seek_dubbing_sound(void* frame,std::uint32_t value){call<void>(frame,0x94,value);}
    void dubbing_sound_word128(void* frame,std::uint32_t value){put(frame,0x128,value);}
    void link_dubbing_sound(void* frame,void* parent){call<void>(frame,0x38,parent,std::uint32_t{0});}
    void position_dubbing_sound(void* frame,const frontend::SoundPosition& position) {
        const auto flags=field<std::uint32_t>(frame,0xe0);if(flags&0x80u)return;
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xb0+axis*4,position[axis]);
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void direction_dubbing_sound(void* frame,const frontend::SoundPosition& direction){call<void>(frame,0x1c,direction.data(),std::uint32_t{0});}
    void type_dubbing_sound(void* frame,std::uint32_t value){call<void>(frame,0x7c,value);}
    void range_dubbing_sound(void* frame,float near_range,float far_range,float first,float second){call<void>(frame,0x80,near_range,far_range,first,second);}
    void frequency_dubbing_sound(void* frame,float value){call<void>(frame,0x88,value);}
    void dubbing_missing_data(std::int32_t id){std::fprintf(stderr,"Nenalezen Dabing %08d.dat\n",id);}
    void record_dubbing_cleanup_failure(std::exception_ptr failure)noexcept{if(!failure_)failure_=std::move(failure);}
    void rethrow_dubbing_cleanup_failure(){if(failure_)std::rethrow_exception(std::exchange(failure_,{}));}
private:
    template<class Result,class... Args>Result call(void* frame,std::size_t slot,Args... args) {
        auto* object=static_cast<EngineFrame*>(frame);using Function=Result(WINAPI*)(EngineFrame*,Args...);
        return checked_engine_method<Function>(object,engine_,slot)(object,args...);
    }
    template<class Value>static Value field(void* frame,std::size_t offset) {
        if(!frame)throw std::runtime_error("Null dubbing frame");
        Value value;std::memcpy(&value,static_cast<const unsigned char*>(frame)+offset,sizeof value);return value;
    }
    template<class Value>static void put(void* frame,std::size_t offset,const Value& value) {
        if(!frame)throw std::runtime_error("Null dubbing frame");
        std::memcpy(static_cast<unsigned char*>(frame)+offset,&value,sizeof value);
    }
    HMODULE engine_;std::exception_ptr failure_;
};
} //namespace hd2::host
#endif
