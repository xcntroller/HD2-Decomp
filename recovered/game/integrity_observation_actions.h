#ifndef HD2_GAME_INTEGRITY_OBSERVATION_ACTIONS_H
#define HD2_GAME_INTEGRITY_OBSERVATION_ACTIONS_H

#include <cstdint>
#include <optional>
#include <stdexcept>

namespace hd2::game {

//Scalar continuation state for00996330/009977FE/00997837. The containing
//thread/caller owner must loan this state per invocation and retire the loan
//on return/exception; this type is not a second process observation table.
struct IntegrityReturnOverride {
    std::optional<std::uint32_t> value;
    bool xor_api_result=false;
};

//Call only after the real validation and blocked-state check. Action62 writes
//before the underlying API call; actions02/03 affect its later returned EAX.
//Returns false without side effects on rejected/blocked validation.
inline bool apply_integrity_observation_action(std::uint8_t action,bool validated,
    bool blocked,std::uint32_t matched_word,std::uint32_t* destination,
    IntegrityReturnOverride& returned) {
    if(!validated||blocked)return false;
    switch(action){
    case 0:returned.value=0;returned.xor_api_result=false;return true;
    case 1:returned.value=1;returned.xor_api_result=false;return true;
    case 2:returned.value=matched_word;returned.xor_api_result=false;return true;
    case 3:returned.value=matched_word;returned.xor_api_result=true;return true;
    case 0x60:case 0x61:case 0x62:case 0x63:
        if(!destination)throw std::runtime_error("Integrity action requires its actual matched destination");
        if(action==0x60)*destination=0;
        else if(action==0x61)*destination=1;
        else if(action==0x62)*destination=matched_word;
        else *destination^=matched_word;
        return true;
    default:throw std::runtime_error("Integrity action requires its original non-scalar receiver");
    }
}

inline std::uint32_t restore_integrity_api_return(std::uint32_t actual_api_result,
    const IntegrityReturnOverride& returned) noexcept {
    if(!returned.value)return actual_api_result;
    return returned.xor_api_result?actual_api_result^*returned.value:*returned.value;
}

//00996750 has no action for any currently active table descriptor (02/62).
//Its conditional32/33/41 branches still need their real message/caller owner;
//declining those branches explicitly prevents a successful empty receiver.
inline void fallback_integrity_observation_action(std::uint8_t action) {
    if(action==0x32||action==0x33||action==0x41)
        throw std::runtime_error("Integrity fallback requires its original message/caller receiver");
}

} //namespace hd2::game
#endif
