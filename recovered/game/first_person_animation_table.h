#ifndef HD2_GAME_FIRST_PERSON_ANIMATION_TABLE_H
#define HD2_GAME_FIRST_PERSON_ANIMATION_TABLE_H

#include "game/overlay_owner.h"
#include "game/serialized_bytes.h"

namespace hd2::game {

//00490C80 and00415C90/004476C0/00415D00: tables\FpvAnims.sav. Store names
//and raw weights into the SAME500×13×4 view table. No clip/model load or
//first-person playback is fabricated by this reader. Runtime Files remains
//authoritative for loose/Patch/archive selection.
class FirstPersonAnimationTable {
public:
    struct Result {std::size_t weapons=0,animations=0,variants=0,populated=0;};
    template<class Owner>
    static Result read(SerializedBytes data,Owner& owner) {
        auto root=next_mission_chunk(data);
        if(!root||root->id!=12345)throw std::runtime_error("Unknown first-person animation table root");
        struct Entry {std::size_t weapon,animation,variant;std::shared_ptr<const std::string> name;std::uint32_t weight;};
        std::vector<Entry> entries;Result result;
        while(auto weapon=next_mission_chunk(root->bytes)) {
            if(weapon->id<100||weapon->id>=100+Owner::capacity)
                throw std::runtime_error("First-person animation weapon exceeds its native table");
            const auto slot=std::size_t(weapon->id-100);++result.weapons;
            for(unsigned n=0;n!=13;++n) { //Original consumes exactly13 row headers, then pops the weapon chunk.
                auto animation=next_mission_chunk(weapon->bytes);
                if(!animation||animation->id<2000||animation->id>=2013)
                    throw std::runtime_error("First-person animation row is missing or exceeds its native table");
                const auto row=std::size_t(animation->id-2000);++result.animations;
                for(unsigned variant=0;variant!=4&&animation->bytes.remaining();++variant) {
                    auto record=next_mission_chunk(animation->bytes);++result.variants;
                    //Variant IDs are not used by the native loader; position
                    //in the row supplies the slot, including empty records.
                    if(!record->bytes.remaining())continue;
                    auto name=next_mission_chunk(record->bytes);
                    if(name->id!=1000||!name->bytes.remaining())continue;
                    if(name->bytes.remaining()>999)throw std::runtime_error("First-person animation name exceeds native string buffer");
                    auto text=name->bytes.text(); //Require termination instead of native unchecked strlen.
                    auto value=std::make_shared<const std::string>(std::move(text));
                    const auto weight=record->bytes.word(); //Raw DWORD AFTER the name child, not another chunk.
                    entries.push_back(Entry{slot,row,variant,std::move(value),weight});++result.populated;
                }
                //Original pops any remaining row/weapon payload after its fixed loops.
            }
        }
        //Publish only after validation/allocation succeeds. Empty variants,
        //omitted weapons and previously cached clips remain untouched.
        for(auto& entry:entries) {
            auto& row=owner.record(entry.weapon).rows[entry.animation];
            row.text[entry.variant]=std::move(entry.name);row.weights[entry.variant]=entry.weight;
        }
        return result;
    }
    template<class Files,class Owner>
    static std::optional<Result> load(Files& files,Owner& owner) {
        const auto handle=files.open("tables\\FpvAnims.sav",0);
        if(handle==0xffffffffu)return std::nullopt; //Original missing-file diagnostic returns without changing table.
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>64u*1024u*1024u)throw std::runtime_error("Invalid first-person animation table size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind first-person animation table");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read first-person animation table");
        return read(SerializedBytes(bytes),owner);
    }
};
} //namespace hd2::game
#endif
