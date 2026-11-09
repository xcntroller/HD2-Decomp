#ifndef HD2_GAME_MISSION_ITEM_RECORDS_H
#define HD2_GAME_MISSION_ITEM_RECORDS_H

#include "game/serialized_bytes.h"
#include <array>
#include <utility>

namespace hd2::game {

struct MissionItemRecord {
    std::string frame_name;
    std::int32_t slot=0,definition=-1;
    std::uint32_t word_after_definition=0;
    bool placed=false,flag1d=false;
    std::array<float,4> position{},rotation{};
    std::uint32_t field16=0;
};

//007D81A0's actual version16 items.dat framing. Decode before mutating the
//world, preserving record order, padded names up to their FIRST NUL, and all
//four pose lanes. This is the file reader, not a successful transfer/spawn
//service. The Items owner must instantiate the selected catalogue definition,
//allocate its identity and consume EVERY record through the actual receivers.
inline std::vector<MissionItemRecord> read_mission_item_records(SerializedBytes& input) {
    if(input.word()!=16)throw std::runtime_error("Unsupported mission item version");
    std::vector<MissionItemRecord> result;
    while(input.remaining()) {
        if(result.size()>=1000000u)throw std::runtime_error("Mission item count exceeds bounds");
        const auto length=input.word();
        //The original reads length+1 into a128-byte stack buffer. Reject its
        //overflow/unterminated cases instead of consulting adjacent stack data.
        if(length>=128)throw std::runtime_error("Mission item frame name exceeds bounds");
        auto name=input.section(length+1u);
        MissionItemRecord record;
        record.frame_name=name.text(); //Padding after the first NUL is allowed.
        const auto integer=[&] {const auto word=input.word();std::int32_t value;std::memcpy(&value,&word,4);return value;};
        record.slot=integer();record.definition=integer();
        record.word_after_definition=input.word();
        record.placed=input.word()!=0;
        if(record.placed) {
            for(auto& lane:record.position)lane=input.real();
            for(auto& lane:record.rotation)lane=input.real();
        }
        record.flag1d=input.word()!=0;
        record.field16=input.word();
        result.push_back(std::move(record));
    }
    return result;
}

} //namespace hd2::game
#endif
