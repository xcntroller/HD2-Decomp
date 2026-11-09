#ifndef HD2_GAME_EFFECT_DEFINITIONS_H
#define HD2_GAME_EFFECT_DEFINITIONS_H

#include "game/serialized_bytes.h"
#include <array>
#include <algorithm>
#include <string>
#include <vector>

namespace hd2::game {

//Native values for00444140/00444490/004448A0's particle-manager definition tree.
//Serialized words retain their bit patterns; consumers assign their original
//float/integer meanings. No engine textures/instances are invented by parsing.
struct EffectControlPoint {
    std::string name8;
    std::array<std::uint32_t,3> position10{},velocity20{};
    std::array<std::uint32_t,7> fields30{{1000,0,0,0,0,0x3f800000,0}};
    std::array<std::uint32_t,3> vector50{{0x3f000000,0x3f000000,0x3f000000}},vector60{};
    std::array<std::uint32_t,4> fields78{};
    std::array<std::uint32_t,2> fields70{{0x3f800000,0}};
};
struct EffectEmitterDefinition {
    std::string name8;
    std::array<std::uint32_t,8> fields24{};
    std::uint32_t field44=0;
    std::array<std::uint32_t,3> vector50{},vector60{};
    std::array<std::uint32_t,8> fields70{{0x3f800000,0,0x3f800000,0,0x3f800000,0,0,0}};
    std::array<std::uint32_t,2> colour_fields{{0,100}}; //00444880 subowner at+0C.
    std::vector<std::array<std::uint32_t,4>> colour_keys;
    std::vector<EffectControlPoint> points;
};
struct EffectDefinition {
    std::string name8,texturec;
    std::uint8_t flag10=0;
    std::uint32_t field14=1;
    std::array<std::uint32_t,2> fields18{{0x41a00000,0x42480000}},fields20{{5,6}};
    std::vector<EffectEmitterDefinition> emitters;
    bool valid() const { //004454F0, no resource load in this predicate.
        if(texturec.empty()||emitters.empty())return false;
        bool points=false;
        for(auto emitter=emitters.rbegin();emitter!=emitters.rend();++emitter) {
            if(emitter->colour_keys.empty())return false;
            if(!emitter->points.empty())points=true;
        }
        return points;
    }
};

class EffectDefinitions {
public:
    const std::vector<EffectDefinition>& values() const {return values_;}
    void clear() noexcept {values_.clear();}
    bool load(SerializedBytes bytes) { //00444140: a nonempty table is process-cached.
        if(!values_.empty())return true;
        auto root=next_mission_chunk(bytes);
        if(!root||root->id!=1000)return false;
        std::vector<EffectDefinition> next;
        while(auto chunk=next_mission_chunk(root->bytes)) {
            if(chunk->id!=1010)continue; //Original skips other root records.
            EffectDefinition definition;
            read_definition(chunk->bytes,definition);next.push_back(std::move(definition));
        }
        values_=std::move(next);return true;
    }
private:
    template<std::size_t N>static void words(SerializedBytes& bytes,std::array<std::uint32_t,N>& result) {
        for(auto& value:result)value=bytes.word();
    }
    static std::string text(SerializedBytes& bytes) {
        //00415D00 /inlined readers consume at most999 bytes then leave chunk.
        //Native terminator validation replaces the original unbounded C string.
        return bytes.section(std::min<std::size_t>(bytes.remaining(),999)).text();
    }
    static void read_definition(SerializedBytes& bytes,EffectDefinition& result) { //00444490.
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& data=chunk->bytes;
            switch(chunk->id) {
            case 100:result.flag10=data.byte();result.field14=data.word();break;
            case 101:result.name8=text(data);break;
            case 102:result.texturec=text(data);break;
            case 103: {
                EffectEmitterDefinition emitter;read_emitter(data,emitter);result.emitters.push_back(std::move(emitter));break;
            }
            case 104:words(data,result.fields18);break;
            case 105:words(data,result.fields20);break;
            default:throw std::runtime_error("Unknown effect definition field");
            }
        }
    }
    static void read_emitter(SerializedBytes& bytes,EffectEmitterDefinition& result) { //004448A0.
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& data=chunk->bytes;
            switch(chunk->id) {
            case 200:words(data,result.fields24);words(data,result.vector50);words(data,result.vector60);words(data,result.fields70);break;
            case 201:result.name8=text(data);break;
            case 202: {
                words(data,result.colour_fields);const auto count=carried_signed_word(data.word());
                if(count>0) {
                    if(std::uint32_t(count)>data.remaining()/16)throw std::runtime_error("Truncated effect colour keys");
                    for(std::int32_t n=0;n<count;++n){std::array<std::uint32_t,4> value;words(data,value);result.colour_keys.push_back(value);}
                }
                break;
            }
            case 203: {
                EffectControlPoint point;read_point(data,point);result.points.push_back(std::move(point));break;
            }
            case 204:result.field44=data.word();break;
            default:throw std::runtime_error("Unknown effect emitter field");
            }
        }
    }
    static void read_point(SerializedBytes& bytes,EffectControlPoint& result) {
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& data=chunk->bytes;
            switch(chunk->id) {
            case 300:words(data,result.position10);words(data,result.velocity20);words(data,result.fields30);
                words(data,result.vector50);words(data,result.vector60);words(data,result.fields78);break;
            case 301:result.name8=text(data);break;
            case 302:words(data,result.fields70);break;
            default:throw std::runtime_error("Unknown effect control point field");
            }
        }
    }
    static std::int32_t carried_signed_word(std::uint32_t word) {
        std::int32_t result;std::memcpy(&result,&word,4);return result;
    }
    std::vector<EffectDefinition> values_;
};

//00445D90's version201 path for the required missions. Placed records are real
//typed inputs to the mandatory instance producer00445AF0; an empty root calls
//no instance service. Version101 needs the legacy frame-copy reader first.
template<class Backend>
void read_mission_effects(SerializedBytes bytes,Backend& backend) {
    auto root=next_mission_chunk(bytes);
    if(!root||root->id!=201)throw std::runtime_error("Unsupported mission effects version");
    while(auto chunk=next_mission_chunk(root->bytes)) {
        if(chunk->id!=1000)throw std::runtime_error("Unknown placed effect record");
        auto& data=chunk->bytes;
        std::array<std::uint32_t,16> transform;
        for(auto& word:transform)word=data.word();
        const auto definition=data.word();auto frame_name=next_mission_chunk(data);
        std::string name;
        if(frame_name)name=frame_name->bytes.section(std::min<std::size_t>(frame_name->bytes.remaining(),999)).text();
        const auto frame=frame_name?backend.find_effect_frame(name):typename Backend::Frame{};
        backend.create_placed_effect(definition,transform,frame);
    }
}

} //namespace hd2::game
#endif
