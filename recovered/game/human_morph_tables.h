#ifndef HD2_GAME_HUMAN_MORPH_TABLES_H
#define HD2_GAME_HUMAN_MORPH_TABLES_H

#include "game/serialized_bytes.h"
#include <array>
#include <exception>

namespace hd2::game {

//00781B00, globals00984130/00983550. Shared process lifetime, lazy on the
//first00780990 constructor. Missing file leaves the original zero tables;
//a malformed present file fails rather than exposing partial native memory.
class HumanMorphTables {
public:
    struct Record {
        std::array<float,64> coefficients{};
        std::uint32_t loaded=0,tail=0;
        std::array<unsigned char,32> metadata{};
    };
    template<class Files>void load(Files& files) {
        if(failure_)std::rethrow_exception(failure_);
        if(attempted_)return;
        attempted_=true; //Original latch precedes open, including missing files.
        try {
            const auto handle=files.open("Tables\\morph.sav",0);
            if(handle==0xffffffffu)return;
            struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
            std::array<unsigned char,0x61f8> values{};
            std::array<unsigned char,0xbe0> metadata{};
            const auto first=files.read(handle,values.data(),values.size());
            if(first==0xffffffffu)return; //Native does not issue the second read.
            if(first!=values.size())throw std::runtime_error("Truncated Human morph coefficients");
            if(files.read(handle,metadata.data(),metadata.size())!=metadata.size())
                throw std::runtime_error("Truncated Human morph metadata");
            SerializedBytes input(values.data(),values.size());
            for(std::size_t index=0;index!=records_.size();++index) {
                auto& record=records_[index];
                for(auto& coefficient:record.coefficients)coefficient=input.real();
                record.loaded=input.word();record.tail=input.word();
                std::memcpy(record.metadata.data(),metadata.data()+index*32,32);
            }
        }catch(...) {failure_=std::current_exception();throw;}
    }
    const Record& record(std::int32_t human,std::int32_t expression) const {
        const auto index=static_cast<std::int64_t>(human)*19+expression;
        if(index<0||index>=static_cast<std::int64_t>(records_.size()))
            throw std::runtime_error("Human morph selection exceeds the original five tables");
        return records_[static_cast<std::size_t>(index)];
    }
    bool attempted() const {return attempted_;}
private:
    std::array<Record,95> records_{};
    bool attempted_=false;
    std::exception_ptr failure_;
};

//00782C80 copies these strings into BSS0098A348 with stride12.
inline constexpr std::array<const char*,25> human_physics_bone_names{{
    "u","q","t","p","f1","d1","d","c1","g","k","h","l","i",
    "m","j","n","a","b","c","e","f","o","r","s","v"
}};
inline constexpr std::int32_t human_face_delay_base=3000; //00780850: trunc(4200/float1.4).
inline constexpr std::int32_t human_face_delay_range=857; //00780880: trunc(1200/float1.4).

} //namespace hd2::game
#endif
