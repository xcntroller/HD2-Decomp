#ifndef HD2_GAME_INTEGRITY_VALIDATION_H
#define HD2_GAME_INTEGRITY_VALIDATION_H

#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//Scalar predicates from00994DF0. These borrow private producer outputs; they
//do not allocate/populate them or infer observations from underlying API EAX.
//The containing dispatcher still owns the native clock/debug side effects.
struct IntegrityValidation11Inputs {
    const std::array<std::uint32_t,16>* scores=nullptr; //00B3BD8C
    const std::array<std::uint32_t,4>* selected=nullptr; //00B4EE80
    const std::uint32_t* length=nullptr; //00B52F6C
    std::uint32_t comparison=0; //00B4C2CC, meaningful only with actual length
};

//Native IDIV-style signed remainder, followed by DWORD counter increment.
//Unsigned storage preserves original wrapping without signed C++ overflow.
inline std::uint32_t integrity_validation_selector(std::uint32_t& counter,
    std::uint32_t modulus,std::uint32_t forced=0xffffffffu) {
    if(modulus!=1&&modulus!=2&&modulus!=4)throw std::logic_error("Unsupported integrity counter modulus");
    const auto signed_counter=counter<0x80000000u?static_cast<std::int64_t>(counter):
        static_cast<std::int64_t>(counter)-0x100000000LL;
    const auto selected=static_cast<std::uint32_t>(signed_counter%modulus);
    ++counter;
    return forced==0xffffffffu?selected:forced;
}

inline bool evaluate_integrity_validation11(std::uint32_t& counter,
    const IntegrityValidation11Inputs& input,std::uint32_t forced=0xffffffffu) {
    const auto selected=integrity_validation_selector(counter,2,forced);
    if(selected==0){
        if(!input.scores)return false; //Native null private pointer fails.
        if(!input.selected)throw std::runtime_error("Integrity11 requires original selected indices");
        for(const auto index:*input.selected){
            //Source bounds guard replaces native invalid-pointer access only.
            if(index>=input.scores->size())throw std::runtime_error("Integrity11 private index out of range");
            if((*input.scores)[index]!=0)return false;
        }
        return true;
    }
    return selected==1&&input.length&&input.comparison==*input.length-0x96u;
}

//00994DF0 validation20: unlike11/21, the normal counter selector is always0.
//The caller's checksum callback must read the actual executable region from
//009F7020's returned DWORD to DWORD00B4FFF2; the cached WORD is00B4C8D0.
template<class Checksum>
bool evaluate_integrity_validation20(std::uint32_t& counter,std::uint32_t flag52620,
    std::uint32_t flag4c004,std::uint16_t expected,std::uint16_t& cached,
    Checksum checksum,std::uint32_t forced=0xffffffffu) {
    if(integrity_validation_selector(counter,1,forced)!=0)return false;
    const bool bypass_flag=flag52620==1||flag4c004==1; //Actual configuration gates, not invented defaults.
    if(!bypass_flag)cached=checksum();
    return bypass_flag||expected==cached;
}

struct IntegrityValidation21Inputs {
    std::uint32_t environment=0; //00B3BD7C
    std::uint32_t flag52620=0,flag4c004=0;
    const std::uint32_t* configuration_word=nullptr; //00B555B0, actual producer allocation
    std::uint16_t expected_word=0; //00B4FFEC
};

//Required query endpoints are explicit: unavailable private key/import evidence
//must throw at the containing binding, never become a successful null callback.
//Queries expose the actual009F8C60/009F8D40 result; callers do not negate twice.
template<class Environment>
bool evaluate_integrity_validation21(std::uint32_t& counter,
    const IntegrityValidation21Inputs& input,Environment& environment,
    std::uint32_t forced=0xffffffffu) {
    switch(integrity_validation_selector(counter,4,forced)){
    case 0:return !environment.integrity_query_009f8c60(input.environment);
    case 1:
        if(input.flag52620==1||input.flag4c004==1)return true;
        if(!input.configuration_word)throw std::runtime_error("Integrity21 requires original configuration allocation");
        return (*input.configuration_word&0xffffu)==input.expected_word;
    case 2:return !environment.integrity_query_009f8d40(input.environment);
    case 3:return true; //Actual branch3; does not establish the other checks.
    default:return false; //Signed negative remainder or explicit unsupported selector.
    }
}

} //namespace hd2::game
#endif
