#ifndef HD2_GAME_INTEGRITY_DESCRIPTORS_H
#define HD2_GAME_INTEGRITY_DESCRIPTORS_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace hd2::game {

//00994810 / table00A8D258. DWORDs are original ABI words, never host pointers.
struct IntegrityDescriptor { std::uint32_t flags, first, second, third; };
struct IntegrityCallWords {
    std::array<std::uint32_t,6> registers{}; //original params2..7
    std::array<std::uint32_t,5> arguments{}; //original params9..13
    //Original param8: scans begin at +4 and may read through +0x48.
    std::array<std::uint32_t,19> caller_frame{};
};
struct IntegrityDescriptorMatch {
    std::size_t slot=32;
    std::array<std::uint32_t,3> words{};
    explicit operator bool()const noexcept{return slot!=32;}
};

inline IntegrityDescriptorMatch match_integrity_descriptor(
    const std::array<IntegrityDescriptor,32>& descriptors,const IntegrityCallWords& call) noexcept {
    for(std::size_t slot=0;slot<descriptors.size();++slot){
        const auto& d=descriptors[slot];if(!(d.flags&1u))continue;
        const auto selector=d.flags>>24;
        const auto matches=[&](const std::uint32_t* words){
            return d.first==words[0]&&(!(d.flags&4u)||d.second==words[1])&&
                (!(d.flags&8u)||d.third==words[2]);
        };
        const std::uint32_t* words=nullptr;
        if(selector<=3)words=call.registers.data()+selector;
        else if(selector>=0x10&&selector<=0x12)words=call.arguments.data()+selector-0x10;
        else if(selector==0x13||selector==0x14){
            const auto count=selector==0x13?12u:16u;
            for(unsigned n=0;n<count;++n){
                const auto* candidate=call.caller_frame.data()+n+1;
                if(matches(candidate)){words=candidate;break;}
            }
        }
        if(words&&matches(words))return {slot,{words[0],words[1],words[2]}};
    }
    return {};
}

//009FB050: null private state is not blocked; shift uses x86's low five bits.
inline bool integrity_descriptor_blocked(std::uint32_t key,std::uint8_t slot,
    const std::array<std::uint32_t,2>* state) noexcept {
    return state&&(((*state)[1]^((*state)[0]>>1)^key)&(std::uint32_t{1}<<(slot&31u)))!=0;
}

} //namespace hd2::game
#endif
