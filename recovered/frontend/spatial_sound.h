#ifndef HD2_FRONTEND_SPATIAL_SOUND_H
#define HD2_FRONTEND_SPATIAL_SOUND_H

#include "frontend/sound_runtime.h"
#include <exception>

namespace hd2::frontend {

//004374D0's64-byte value, with typed pointer slots for native64-bit audits.
//An engaged filename (including empty text) selects the direct-file branch.
struct SpatialSoundRequest {
    SoundPosition position{};
    std::uint32_t flags=0;
    void* relative=nullptr;
    void* source=nullptr;
    std::int32_t group=0,entry=0;
    std::optional<std::string> filename;
    float near_range=0,far_range=0;
    std::uint32_t voice_group=4;
    float near_scale=1,far_scale=1,volume=1;
};

inline float sound_float_word(std::uint32_t bits) {float value;std::memcpy(&value,&bits,4);return value;}

//00435B00 callback payload; native i386 layout is32 bytes. Listener frame and
//source actor are typed borrowers, and registered callbacks remain real owners.
struct HeardSpatialSound {
    SoundPosition position{};
    void* listener=nullptr;
    float volume=0;
    std::int32_t group=0;
    void* source=nullptr;
};
template<class Backend>
void notify_spatial_sound(SoundRuntimeState& state,std::int32_t handle,bool occluded,Backend& backend) {
    if(!state.driver_delay)throw std::runtime_error("Undefined original sound driver delay");
    if(*state.driver_delay||handle<0||static_cast<std::uint32_t>(handle)>=state.voices.size())return;
    const auto* voice=state.voices[static_cast<unsigned>(handle)].get();if(!voice)return;
    const auto far_range=voice->far_range,near_range=voice->near_range;
    const auto position=voice->position;auto* source=voice->source;const auto group=voice->group;
    //Native unconditionally calls005AC4C0 on(source+98); null is undefined.
    if(!source)throw std::runtime_error("Spatial sound hearing requires the actual source actor");
    auto* source_sector=backend.sound_actor_sector(source);
    for(auto count=state.registrations.size();count!=0;) {
        const auto registration=state.registrations.at(--count);
        if(!registration[2]||!registration[0])continue;
        auto listener=backend.sound_position(registration[0]);listener[1]=static_cast<float>(static_cast<long double>(listener[1])+.25f);
        auto distance=sound_distance(position,listener);if(!(distance<=far_range))continue;
        float scale=1;
        if(occluded&&registration[1]) {
            auto* target_sector=backend.sound_actor_sector(registration[1]);
            distance=backend.sound_occlusion_distance(scale,position,listener,source_sector,target_sector,far_range); //00432BE0.
            if(!(distance<=far_range))continue;
        }
        HeardSpatialSound heard{position,registration[0],1,group,source};
        //The native near_range branch ignores occlusion scale; equality gives1.
        if(!(distance<=near_range))heard.volume=static_cast<float>((1.L-(distance-near_range)/(static_cast<long double>(far_range)-near_range))*scale);
        backend.deliver_spatial_sound(registration,heard);
    }
}

//00433FA0. Uses the actual manager pool/catalogue, live gate and shared CRT
//random. Backend must supply actual scene/actor/module and category2 cache
//receivers; no optional successful playback/hearing services exist here.
template<class Resources,class Backend>
std::int32_t emit_spatial_sound(SoundRuntimeState& state,SoundCatalog<Resources>& catalog,
        const SpatialSoundRequest* request,const std::uint32_t& gate,Backend& backend) {
    if(!request)return -1;
    const auto& value=*request;
    const auto* world=backend.sound_world();
    if(!world||!world->scene)throw std::runtime_error("Spatial sound has no actual world scene");
    auto position=value.position;
    if(value.relative) {
        const auto parent=backend.sound_position(value.relative);
        for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(position[axis])+parent[axis]);
    }
    const auto listener=backend.sound_position(backend.scene_sound_listener(world->scene));
    const auto x=static_cast<long double>(listener[0])-position[0],y=static_cast<long double>(listener[1])-position[1],z=static_cast<long double>(listener[2])-position[2];
    const float square=static_cast<float>((z*z+y*y)+x*x);
    CatalogSoundEntry* entry=nullptr;
    if(!value.filename) {
        if(value.group<0||static_cast<std::uint32_t>(value.group)>=catalog.groups.size())return -1;
        const auto& group=catalog.groups[static_cast<unsigned>(value.group)];
        if(!group)throw std::runtime_error("Null spatial sound catalogue group");
        if(value.entry<0||static_cast<std::uint32_t>(value.entry)>=group->entries.size())return -1;
        entry=group->entries[static_cast<unsigned>(value.entry)].get();
    }
    auto allocate=[&]() {
        if(state.voices.size()>0x7fffffffu)throw std::length_error("Spatial voice handle exceeds original signed range");
        auto next=std::make_unique<SoundVoice>();
        const auto handle=static_cast<std::int32_t>(state.voices.size());
        state.free_voices.push_back(handle);
        try{state.voices.push_back(std::move(next));}catch(...){state.free_voices.pop_back();throw;}
        return handle;
    };
    auto handle=state.free_voices.empty()?allocate():state.free_voices.back();
    if(handle<0||static_cast<std::uint32_t>(handle)>=state.voices.size())
        throw std::runtime_error("Spatial sound free-list handle is invalid");
    if(!state.voices[static_cast<unsigned>(handle)])handle=allocate(); //Original null free slot allocates another record.
    auto& voice=*state.voices[static_cast<unsigned>(handle)];
    CatalogSoundVariant* variant=nullptr;
    if(!value.filename) {
        if(!entry||entry->variants.empty())return -1;
        const auto choice=backend.sound_random_word()%entry->variants.size();
        variant=entry->variants[choice].get();voice.cache=variant;
        if(!variant)return -1;
        voice.far_range=static_cast<float>(static_cast<long double>(variant->float0c)*value.far_scale);
        voice.near_range=static_cast<float>(static_cast<long double>(sound_float_word(variant->word08))*value.near_scale);
        voice.target_volume=static_cast<float>(static_cast<long double>(variant->float14)*value.volume);
        voice.extra=variant->float18;voice.flags=value.flags|(value.source?0u:1u);
    }else {
        voice.far_range=static_cast<float>(static_cast<long double>(value.far_scale)*value.far_range);
        voice.near_range=static_cast<float>(static_cast<long double>(value.near_scale)*value.near_range);
        voice.target_volume=value.volume;voice.extra=0;voice.flags=1;
    }
    voice.group=value.group;voice.position=value.position;voice.relative=value.relative;voice.source=value.source;
    if(value.relative) {
        const auto parent=backend.sound_position(value.relative); //Native recomputes after shared random/variant selection.
        for(unsigned axis=0;axis!=3;++axis)voice.position[axis]=static_cast<float>(static_cast<long double>(voice.position[axis])+parent[axis]);
    }
    voice.active=1;
    if(!state.free_voices.empty())state.free_voices.pop_back();
    try {
        //The square remains float32 but far_range squared is extended in FCOMPP.
        bool audible=static_cast<long double>(square)<=static_cast<long double>(voice.far_range)*voice.far_range;
        float volume_scale=1;
        if(audible) {
            bool crossed=false;
            if(value.group==2) {
                auto* source_sector=value.source&&backend.sound_actor_type(value.source)==3?backend.sound_actor_sector(value.source):nullptr;
                auto* camera_sector=backend.sound_camera_sector();
                if(source_sector&&camera_sector) {
                    const auto distance=sound_sector_distance(volume_scale,position,listener,source_sector,camera_sector,backend);
                    if(distance>voice.far_range){audible=false;crossed=true;}
                }
            }
            if(!crossed&&backend.sound_special_mode()) {
                switch(value.group) {
                case 0:case 1:case 3:case 4:case 5:case 7:case 9:
                    if(value.source&&backend.sound_actor_type(value.source)==3&&backend.sound_actor_flag18(value.source))audible=false;
                    break;
                default:voice.target_volume=static_cast<float>(static_cast<long double>(voice.target_volume)*.4f);break; //0080F554.
                }
            }
        }
        const bool ambient=value.filename?(value.flags&0x40u)!=0:entry->flag!=0;
        if(value.flags&0x80u)audible=true;
        if((gate&2u)&&(audible||ambient||(value.flags&0xcu))&&!(value.flags&2u)) {
            auto name=value.filename?*value.filename:variant->name;
            //007E5A40 is strstr, preserving a .wav occurrence anywhere in the name.
            if(name.find(".wav")==std::string::npos)name+=".wav";
            if(name.size()>=260)throw std::runtime_error("Spatial sound name exceeds original buffer");
            if(backend.sound_driver_initialized()) {
                if(variant&&!variant->sounds.empty()) {
                    voice.sound=variant->sounds.back();variant->sounds.pop_back();
                }else {
                    voice.sound=backend.create_spatial_sound();
                    if(voice.sound) {
                        if(variant&&variant->sound)backend.copy_spatial_sound(voice.sound,variant->sound);
                        else {
                            const auto status=backend.load_spatial_sound(voice.sound,name,0x10);
                            //Native also retains its empty created frame after a refused open.
                            if(variant&&status==0)variant->sound=backend.cache_spatial_sound(name);
                        }
                    }
                }
                if(!voice.sound)return -1;
                const auto parent=value.relative?value.relative:backend.primary_sound_sector(backend.sound_world()->scene);
                backend.link_spatial_sound(voice.sound,parent);
                backend.position_spatial_sound(voice.sound,value.position);
                backend.type_spatial_sound(voice.sound,(value.flags&0x40u)||ambient?3u:1u);
                backend.range_spatial_sound(voice.sound,voice.near_range,voice.far_range,.4f,.3f);
                backend.sound_volume(voice.sound,(value.flags&0x10u)?0.f:static_cast<float>(static_cast<long double>(volume_scale)*voice.target_volume));
                backend.loop_spatial_sound(voice.sound,(value.flags>>3)&1u);
                backend.parameter94_spatial_sound(voice.sound,0);
                backend.sound_mode(voice.sound,1,0);
                if(value.voice_group!=4) {
                    if(value.voice_group>=state.groups.size())throw std::out_of_range("Spatial sound voice group");
                    auto& group=state.groups[value.voice_group];
                    if(group.cursor<0||static_cast<std::uint32_t>(group.cursor)>=group.slots.size())
                        throw std::runtime_error("Spatial sound group cursor has no native slot");
                    auto& slot=group.slots[static_cast<unsigned>(group.cursor)];
                    if(slot!=-1)stop_sound_voice(state,slot,backend);
                    slot=handle;++group.cursor;
                    if(group.cursor>=sound_signed_word(group.limit))group.cursor=0;
                    voice.flags|=4u;
                }
            }
        }
        if((gate&1u)&&!(value.flags&1u))notify_spatial_sound(state,handle,(value.flags&0x20u)!=0,backend);
        if(backend.sound_driver_initialized())return handle;
        stop_sound_voice(state,handle,backend);return -1;
    }catch(...) {
        const auto failure=std::current_exception();
        try{stop_sound_voice(state,handle,backend);}catch(...){backend.record_sound_cleanup_failure(std::current_exception());}
        std::rethrow_exception(failure);
    }
}
} //namespace hd2::frontend
#endif
