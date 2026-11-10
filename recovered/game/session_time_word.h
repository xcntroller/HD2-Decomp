#ifndef HD2_GAME_SESSION_TIME_WORD_H
#define HD2_GAME_SESSION_TIME_WORD_H
#include "game/session_activation.h"
namespace hd2::game {
//Native-width borrower of SAME game+28. Source containing state stores that
//word in scalar bytes; this view rereads them without aliasing a uint32_t& onto
//byte storage, caching a time copy or constructing a second clock.
class SessionTimeWord {
public:
    SessionTimeWord(const std::uint32_t& word):word_(&word){}
    SessionTimeWord(const SessionActivationState& session):session_(&session){}
    operator std::uint32_t()const{return session_?session_->words.get<std::uint32_t>(0x28):*word_;}
private:
    const std::uint32_t* word_=nullptr;const SessionActivationState* session_=nullptr;
};
} //namespace hd2::game
#endif
