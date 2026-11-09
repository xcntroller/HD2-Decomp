#ifndef HD2_HOST_GAME_SPATIAL_SOUND_H
#define HD2_HOST_GAME_SPATIAL_SOUND_H

#include "host/sound_services.h"
#include "host/loaded_resources.h"
#include "frontend/spatial_sound.h"
#include "frontend/sound_portal_routing.h"

namespace hd2::host {

//Stateless engine/cache adapter. StartupSession supplies its SAME manager,
//catalogue, gate and category2 cache; World supplies actual actor/module
//consumers. Portal routing uses the same SoundServices scene/sector endpoints.
//Construction creates no sound frame and publishes no readiness.
template<class World>
class GameSpatialSoundServices : public SoundServices {
public:
    GameSpatialSoundServices(HMODULE engine,frontend::SoundWorldView*& view,LoadedResources& resources,World& world)
        :SoundServices(engine,view),engine_(engine),resources_(resources),world_(world) {}
    void* create_spatial_sound() {
        return call<EngineFrame*>(driver(),0x54,std::uint32_t{4});
    }
    std::int32_t load_spatial_sound(void* frame,const std::string& name,std::uint32_t flags) {
        return resources_.load_sound(static_cast<EngineFrame*>(frame),name,flags);
    }
    void copy_spatial_sound(void* destination,void* source) {
        CachedResourceBackend(engine_,2).copy(static_cast<EngineFrame*>(destination),static_cast<EngineFrame*>(source));
    }
    void* cache_spatial_sound(const std::string& name) {
        MenuDriver::Owner result{nullptr,nullptr};
        (void)resources_.cache(2).get(name,result);return result.release(); //Transferred catalogue primary reference.
    }
    void link_spatial_sound(void* frame,void* parent) {call<void>(frame,0x38,parent,std::uint32_t{0});}
    void position_spatial_sound(void* frame,const frontend::SoundPosition& position) {
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(flags&0x80u)return;
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xb0+axis*4,position[axis]);
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void type_spatial_sound(void* frame,std::uint32_t type) {call<void>(frame,0x7c,type);}
    void range_spatial_sound(void* frame,float near_range,float far_range,float first,float second) {call<void>(frame,0x80,near_range,far_range,first,second);}
    void loop_spatial_sound(void* frame,std::uint32_t loop) {call<void>(frame,0x90,loop);}
    void parameter94_spatial_sound(void* frame,std::uint32_t value) {call<void>(frame,0x94,value);}
    unsigned sound_actor_type(void* actor) {return world_.sound_actor_type(actor);}
    void* sound_actor_sector(void* actor) {return world_.sound_actor_sector(actor);}
    void* sound_camera_sector() {return world_.sound_camera_sector();}
    bool sound_special_mode() {return world_.sound_special_mode();}
    bool sound_actor_flag18(void* actor) {return world_.sound_actor_flag18(actor);}
    long double sound_occlusion_distance(float& scale,const frontend::SoundPosition& first,const frontend::SoundPosition& second,
            void* first_sector,void* second_sector,float range) {
        return frontend::sound_portal_distance(scale,first,second,first_sector,second_sector,range,*this);
    }
    void deliver_spatial_sound(const std::array<void*,3>& registration,const frontend::HeardSpatialSound& heard) {
        static_assert(sizeof(frontend::HeardSpatialSound)==32,"Original spatial hearing callback requires i386 layout");
        using Deliver=void(WINAPI*)(const frontend::HeardSpatialSound*); //00435CD9/435CFE: callee removes the one DWORD.
        auto* callback=reinterpret_cast<Deliver>(registration[2]);callback(&heard);
    }
    void record_sound_cleanup_failure(std::exception_ptr failure) noexcept {world_.record_actor_cleanup_failure(std::move(failure));}
private:
    template<class Result,class... Args>Result call(void* frame,std::size_t slot,Args... args) {
        auto* object=static_cast<EngineFrame*>(frame);using Function=Result(WINAPI*)(EngineFrame*,Args...);
        return checked_engine_method<Function>(object,engine_,slot)(object,args...);
    }
    template<class Value>static Value field(void* frame,std::size_t offset) {
        if(!frame)throw std::runtime_error("Null spatial sound frame");
        Value value;std::memcpy(&value,static_cast<const unsigned char*>(frame)+offset,sizeof value);return value;
    }
    template<class Value>static void put(void* frame,std::size_t offset,const Value& value) {
        if(!frame)throw std::runtime_error("Null spatial sound frame");
        std::memcpy(static_cast<unsigned char*>(frame)+offset,&value,sizeof value);
    }
    HMODULE engine_;LoadedResources& resources_;World& world_;
};
} //namespace hd2::host
#endif
