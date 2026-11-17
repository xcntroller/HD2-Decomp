#ifndef HD2_GAME_SESSION_FRAME_REQUESTS_H
#define HD2_GAME_SESSION_FRAME_REQUESTS_H
#include "game/prepared_spawn_packets.h"
#include <list>
namespace hd2::game {
//0072B1C0, process0097C8DC/E0/E4. FRAME's pending90 producers enqueue
//commands0/1 only. They do not retain a soldier pointer: drain reads the SAME
//live application squad afresh, after beginning the actual authority stream.
class SessionFrameRequests {
public:
    struct Command {std::uint32_t kind,first,second;};
    template<class Backend>void enqueue(Command command,std::uint32_t mode,Backend& backend){
        if(command.kind>1)throw std::runtime_error("Request FIFO requires its actual non-FRAME command payload owner");
        const bool locked=(mode&7u)!=7u;
        if(locked)backend.enter_application_command_lock();
        try{
            if(commands_.size()==std::numeric_limits<std::uint32_t>::max())throw std::length_error("Application request FIFO count");
            commands_.push_back(command);
        }catch(...){if(locked)backend.leave_application_command_lock();throw;}
        if(locked)backend.leave_application_command_lock();
    }
    template<class Backend>void drain(Backend& backend){
        if(draining_)throw std::logic_error("Application request drain re-entered its process owner");
        draining_=true;struct Reset{bool& value;~Reset(){value=false;}}reset{draining_};
        while(!commands_.empty()){
            const auto command=commands_.front();
            auto& packet=backend.begin_application_request(command.kind); //No A0B0-style1000ms preflush.
            packet.write(static_cast<std::uint32_t>(backend.application_local_peer()),32);
            auto& squad=backend.application_live_squad();
            if(squad.empty())throw std::logic_error("Native request0/1 requires its actual first soldier allocation");
            write_prepared_soldier(packet,squad.front(),false);
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
