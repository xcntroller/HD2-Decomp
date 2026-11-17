#ifndef HD2_GAME_HEAVY_FIRE_DEFINITIONS_H
#define HD2_GAME_HEAVY_FIRE_DEFINITIONS_H
#include "game/serialized_bytes.h"
#include "frontend/pre_mission_data.h"
#include <array>
namespace hd2::game {
//004721C0/00472210, native16 records of18 bytes. Config names belong to
//the process owner; request objects borrow/copy them before World retirement.
struct HeavyFireDefinition {
    std::string model;
    float speed=30.f;
    std::uint32_t effect=0;
    std::int32_t count=2;
    float power=1000.f,radius=10.f;
};
class HeavyFireDefinitions {
public:
    const auto& records()const{return records_;}
    const HeavyFireDefinition& at(std::size_t index)const{return records_.at(index);}
    //004732B0 resets the records only AFTER reading the first root header.
    //A missing file or empty file therefore preserves the existing config.
    bool read(SerializedBytes input){
        auto root=next_mission_chunk(input);if(!root)return false;
        records_.fill(HeavyFireDefinition{});
        std::int32_t index=-1;
        while(auto group=next_mission_chunk(root->bytes)){
            while(auto field=next_mission_chunk(group->bytes)){
                if(field->id==1000){
                    if(++index>=16)throw std::length_error("HeavyFire table exceeds original sixteen records");
                    const auto size=std::min<std::size_t>(field->bytes.remaining(),999);
                    const auto* text=field->bytes.take(size);
                    const auto* end=static_cast<const unsigned char*>(std::memchr(text,0,size));
                    records_[static_cast<unsigned>(index)].model.assign(reinterpret_cast<const char*>(text),end?static_cast<std::size_t>(end-text):size);
                }else if(field->id>=2000&&field->id<=6000&&field->id%1000==0){
                    if(index<0)throw std::runtime_error("HeavyFire field precedes its original name producer");
                    auto& record=records_[static_cast<unsigned>(index)];
                    switch(field->id){
                    case 2000:record.speed=field->bytes.real();break;
                    case 3000:record.effect=field->bytes.word();break;
                    case 4000:{const auto value=field->bytes.word();std::memcpy(&record.count,&value,4);break;}
                    case 5000:record.radius=field->bytes.real();break;
                    case 6000:record.power=field->bytes.real();break;
                    }
                }
            }
        }
        return true;
    }
    template<class Files>bool load(Files& files){
        const auto bytes=frontend::pre_mission_detail::read_file(files,"tables\\airsupport.tbl");
        return bytes&&read(SerializedBytes(*bytes));
    }
private:
    std::array<HeavyFireDefinition,16> records_{};
};
} //namespace hd2::game
#endif
