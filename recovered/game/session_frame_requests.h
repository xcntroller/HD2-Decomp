#ifndef HD2_GAME_SESSION_FRAME_REQUESTS_H
#define HD2_GAME_SESSION_FRAME_REQUESTS_H
#include "game/prepared_spawn_packets.h"
#include <list>
#include "game/session_chat.h"
namespace hd2::game {
//0072B1C0, process0097C8DC/E0/E4. FRAME's pending90 producers enqueue
//commands0/1 and chat3. Spawn requests do not retain a soldier pointer: drain reads the SAME
//live application squad afresh, after beginning the actual authority stream.
class SessionFrameRequests {
public:
    struct Command {std::uint32_t kind,first,second;std::shared_ptr<SessionChatMessage> chat{};};
    template<class Backend>void enqueue(Command command,std::uint32_t mode,Backend& backend){
        if(command.kind>1&&command.kind!=3)throw std::runtime_error("Request FIFO requires its actual non-FRAME command payload owner");
        if(command.kind==3&&!command.chat)throw std::logic_error("Chat request requires its transferred payload");
        const bool locked=(mode&7u)!=7u;
        if(locked)backend.enter_application_command_lock();
        try{
            if(commands_.size()==std::numeric_limits<std::uint32_t>::max())throw std::length_error("Application request FIFO count");
            commands_.push_back(std::move(command));
        }catch(...){if(locked)backend.leave_application_command_lock();throw;}
        if(locked)backend.leave_application_command_lock();
    }
    template<class Backend>void drain(Backend& backend){
        if(draining_)throw std::logic_error("Application request drain re-entered its process owner");
        draining_=true;struct Reset{bool& value;~Reset(){value=false;}}reset{draining_};
        while(!commands_.empty()){
            auto& command=commands_.front();
            auto& packet=backend.begin_application_request(command.kind); //No A0B0-style1000ms preflush.
            if(command.kind==3){
                if(!command.chat)throw std::logic_error("Chat request payload was consumed before failed packet finalization");
                write_session_chat(packet,*command.chat);command.chat.reset(); //Native FREE before0072BF90.
            }else{
                packet.write(static_cast<std::uint32_t>(backend.application_local_peer()),32);
                auto& squad=backend.application_live_squad();
                if(squad.empty())throw std::logic_error("Native request0/1 requires its actual first soldier allocation");
                write_prepared_soldier(packet,squad.front(),false);
            }
            backend.finish_application_command(packet);
            commands_.pop_front(); //Only after actual packet finalization succeeds.
        }
    }
    std::size_t size()const noexcept{return commands_.size();}
private:
    std::list<Command> commands_;bool draining_=false;
};
}
#endif
