#ifndef HD2_GAME_DOG_ANIMATION_TABLES_H
#define HD2_GAME_DOG_ANIMATION_TABLES_H

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::game {

//005D6700 /008AC4B0:23 triples, backed by23 serialized name20/metadata4
//rows. Unlike00783BC0, publication is incremental: retries preserve clips
//already obtained and only revisit nonempty rows whose clip is NULL.
template<class Resources>class DogAnimationTables {
public:
    using Reference=typename Resources::Reference;
    struct Row {
        Reference animation{};
        std::uint32_t metadata=0;
        float stride=0;
        int load_status=0; //Source diagnostic, outside the native triple.
    };
    static constexpr std::size_t row_count=23,serialized_size=row_count*24;
    bool activation_gate()const{return bool(rows_[1].animation);} //005D5290 tests008AC4BC only.
    const Row& row(std::size_t index)const{return rows_.at(index);}
    template<class Frames>void load(const std::vector<unsigned char>& bytes,Resources& resources,Frames& frames){
        //A short successful native read exposes uninitialized stack bytes.
        //Reject it before any row mutation; it is not a successful empty table.
        if(bytes.size()<serialized_size)throw std::runtime_error("Tables\\dog_anim.sav is truncated");
        for(std::size_t n=0;n!=row_count;++n){
            const auto* p=bytes.data()+n*24;auto& row=rows_[n];
            if(!p[0]||row.animation)continue;
            //Metadata is copied BEFORE native strcat.0084E8E4 points to an
            //empty suffix in this image; do not invent .i3d/.anm extensions.
            row.metadata=std::uint32_t(p[20])|(std::uint32_t(p[21])<<8)|
                (std::uint32_t(p[22])<<16)|(std::uint32_t(p[23])<<24);
            const auto* end=static_cast<const unsigned char*>(std::memchr(p,0,20));
            if(!end)throw std::runtime_error("Unterminated Dog animation-table resource name");
            const std::string name(reinterpret_cast<const char*>(p),static_cast<std::size_t>(end-p));
            row.load_status=resources.resolve_animation(name,row.animation);
            if(!row.animation)continue; //Native returns true after the table even if a resource failed.
            const auto bone=frames.human_clip_bone_index(row.animation.get(),"anim");
            if(bone<0||frames.human_clip_position_count(row.animation.get(),bone)<0){row.stride=0;continue;}
            std::uint32_t time=0;std::array<float,4> position{};
            frames.human_clip_position_key(row.animation.get(),bone,0,time,position);
            //0040E0B0: XYZ sum in this order, FLOAT spill, signed bit-pattern
            //tiny comparison, unit tolerance, extended sqrt. Z selects sign.
            const float square=static_cast<float>((static_cast<long double>(position[0])*position[0]+
                static_cast<long double>(position[1])*position[1])+static_cast<long double>(position[2])*position[2]);
            std::int32_t bits;std::memcpy(&bits,&square,4);
            const long double length=bits<0x322bcc77?0.L:
                std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
            row.stride=static_cast<float>(position[2]<0?-length:length);
        }
    }
    void clear()noexcept{
        for(auto& row:rows_){row.animation.reset();row.metadata=0;row.stride=0;row.load_status=0;}
    } //005D5390 releases all23 refs then zeroes69DWORDs;005D4B00 zeroes on construction.
private:
    std::array<Row,row_count> rows_;
};

} //namespace hd2::game
#endif
