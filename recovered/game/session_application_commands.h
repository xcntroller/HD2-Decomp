#ifndef HD2_GAME_SESSION_APPLICATION_COMMANDS_H
#define HD2_GAME_SESSION_APPLICATION_COMMANDS_H
#include <cstdint>
#include <list>
#include <limits>
#include <stdexcept>

namespace hd2::game {
//0072A0B0/0097C8F0 process FIFO, scalar commands reached by Human death/spawn rejection.
//No Human, mission or caller-buffer borrower survives enqueue. Other native
//command families have owning payloads and require their separate producers.
class SessionApplicationCommands {
public:
    struct Command {std::uint32_t kind,first,second,target;};
    template<class Backend>void enqueue(Command command,std::uint32_t mode,Backend& backend){
        if(command.kind!=10&&command.kind!=15&&command.kind!=17)
            throw std::runtime_error("Required application command producer is not reconstructed");
        const bool locked=(mode&7u)!=7u;
        if(locked)backend.enter_application_command_lock(); //Actual application+2C, NOT control418/430 or peerB4.
        try{
            if(commands_.size()==std::numeric_limits<std::uint32_t>::max())throw std::length_error("Application command FIFO count");
            commands_.push_back(command);
        }catch(...){if(locked)backend.leave_application_command_lock();throw;}
        if(locked)backend.leave_application_command_lock();
        //Native offline false also only appends; drain is the distinct ELSE.
    }
    template<class Backend>void drain(Backend& backend){
        if(draining_)throw std::logic_error("Application command drain re-entered its process owner");
        draining_=true;
        struct Reset{bool& flag;~Reset(){flag=false;}}reset{draining_};
        while(!commands_.empty()){
            const auto command=commands_.front();
            backend.flush_application_command_packets(1000); //Before EACH envelope; failure retains current node.
            auto& packet=backend.begin_application_command(command.target,command.kind); //0072BDE0→BE00, local=true.
            packet.write(command.first,command.kind==10||command.kind==15?5:32);
            packet.write(command.second,command.kind==10?4:command.kind==15?1:32);
            backend.finish_application_command(packet); //0072BF90 precedes unlink/free.
            commands_.pop_front();
        }
    }
    std::size_t size()const noexcept{return commands_.size();}
private:
    std::list<Command> commands_;bool draining_=false;
};
} //namespace hd2::game
#endif
