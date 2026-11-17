#ifndef HD2_GAME_SESSION_CHAT_H
#define HD2_GAME_SESSION_CHAT_H
#include "frontend/control_startup.h"
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
namespace hd2::game {
//0072EA90's transferred110-byte record.108 is unwritten by that producer,
//forwarded unchanged by both FIFOs, then discarded by00728450. Initialize this
//opaque wire word to zero in source; it is NOT a target, routing or player ID.
struct SessionChatMessage {
    std::string text;std::int32_t sender=0;std::uint32_t opaque108=0;bool team=false;
};
inline auto make_session_chat(std::string_view text,std::int32_t sender,bool team){
    auto message=std::make_shared<SessionChatMessage>();
    message->text=std::string(text.substr(0,std::min<std::size_t>(text.find('\0'),99)));
    message->sender=sender;message->team=team;return message;
}
template<class Packet>void write_session_chat(Packet& packet,const SessionChatMessage& message){
    packet.write(static_cast<std::uint32_t>(message.sender),32);packet.write(message.opaque108,32);
    packet.write(message.team?1u:0u,1);packet.write_string(message.text);
}
template<class Packet>auto read_session_chat_header(Packet& packet){
    auto message=std::make_shared<SessionChatMessage>();const auto sender=packet.read(32);
    message->sender=sender<=0x7fffffffu?static_cast<std::int32_t>(sender):static_cast<std::int32_t>(static_cast<std::int64_t>(sender)-0x100000000ll);
    message->opaque108=packet.read(32);message->team=packet.read(1)!=0;return message;
}
template<class Packet,class Runtime>void receive_session_chat_request(Packet& packet,Runtime& runtime){ //00727800 case3.
    auto message=read_session_chat_header(packet);
    if(message->sender>=0)runtime.touch_application_chat_peer(static_cast<std::uint32_t>(message->sender)); //A8 multimedia tick BEFORE reading text.
    message->text=packet.read_string(0x103);
    runtime.queue_application_chat_presentation(std::move(message)); //0072A0B0 kind4, broadcast target0.
}
//0064D6C0 is a byte-structure UTF8 recognizer, true only if a multibyte
//sequence occurred. Original accepts C0/C1 and does not validate code points.
inline bool session_chat_multibyte(std::string_view text){
    bool multibyte=false;
    for(std::size_t n=0;n<text.size()&&text[n];){
        const auto c=static_cast<unsigned char>(text[n++]);if(c<0x80)continue;
        const unsigned extra=c>=0xc0&&c<0xe0?1u:c>=0xe0&&c<0xf0?2u:c>=0xf0&&c<0xf8?3u:0u;
        if(!extra||extra>text.size()-n)return false;
        for(unsigned k=0;k<extra;++k){const auto tail=static_cast<unsigned char>(text[n++]);if(tail<0x80||tail>=0xc0)return false;}
        multibyte=true;
    }
    return multibyte;
}
//00728450 case4. The second wire word is consumed but has no semantic use.
//Only the actual control record3C is borrowed, under418, through presentation.
template<class Packet,class Runtime>void receive_session_chat(Packet& packet,frontend::ControlStartupState& control,Runtime& runtime){
    const auto header=read_session_chat_header(packet);
    auto text=packet.read_string(0x104);
    if(header->sender==-1){if(runtime.has_application_game())runtime.present_application_chat("ADMIN",text,nullptr,true);return;}
    if(header->sender<0)throw std::out_of_range("Chat sender is not a native peer slot");
    if(header->team)text="<TEAM> "+text;
    runtime.enter_control_lock(0);struct Unlock{Runtime& r;~Unlock(){r.leave_control_lock(0);}}unlock{runtime};
    const auto& sender=control.records.at(static_cast<std::uint32_t>(header->sender));
    const auto& local=control.records.at(static_cast<std::uint32_t>(runtime.application_local_peer()));
    auto* local_human=local.human_actor();auto* human=sender.human_actor();
    const bool same_alive=!human||!local_human||((runtime.application_chat_health(human)>0)==(runtime.application_chat_health(local_human)>0));
    if((!header->team||sender.word(0x64)==local.word(0x64))&&same_alive&&runtime.has_application_game()){
        auto name=sender.name();
        if(!runtime.wide_characters()){
            if(session_chat_multibyte(name))name="XXX";
            if(session_chat_multibyte(text))text="????????";
        }
        runtime.present_application_chat(name,text,human,false);
    }
}
}
#endif
