#ifndef HD2_GAME_HUMAN_FRAME_COMMANDS_H
#define HD2_GAME_HUMAN_FRAME_COMMANDS_H

#include "game/session_broadcast_packets.h"
#include "game/human_loading_stats.h"

namespace hd2::game {

//00577B30/Player0053F370's actual kind6/005B0F90 and damage1/005B1540 serializers. Remaining command
//payload families require their own recovered producers and serializers.
template<class Commands>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands){
    if(commands.empty())return;
    for(const auto& command:commands){
        if(command.kind!=6&&command.kind!=1&&command.kind!=10)throw std::runtime_error("Human frame command requires its actual payload serializer");
        if(command.kind==1&&!command.damage)throw std::logic_error("Human damage frame command lost its actual payload");
        if(command.kind==10&&!command.drop)throw std::logic_error("Human drop frame command lost its actual payload");
    }
    auto& stream=second?streams.broadcast_stream(0):streams.request_stream(0,mode);
    const auto before=stream.bit_count();
    const auto active=stream.word1c,priority=stream.word20,target=stream.word24,kind=stream.word28;
    const bool request=stream.flag18;
    try{
        stream.begin(13,0,0,second);stream.write(static_cast<std::uint32_t>(identity),10);
        for(const auto& command:commands){
            stream.write(command.kind,5); //Trunc(log2(25)+1), original command family width.
            if(command.kind==6){stream.write(static_cast<std::uint32_t>(command.actor_identity),10);continue;}
            if(command.kind==10){ //005B2020, absent frozen; serializer does not clamp the original speed.
                const auto& drop=*command.drop;stream.write(static_cast<std::uint32_t>(drop.item),16);stream.write_direction(drop.direction,-1);
                stream.write(human_loading_stat_word((static_cast<long double>(drop.speed)-1.f)*.25f*15),4);
                stream.write(drop.actor<0?0u:1u,1);if(drop.actor>=0)stream.write(static_cast<std::uint32_t>(drop.actor),16);
                continue;
            }
            const auto& damage=*command.damage;
            stream.write(damage.health,10);stream.write(damage.kind,4);stream.write(damage.hit_kind,4);
            stream.write(static_cast<std::uint32_t>(damage.source),16);
            const bool detailed=damage.kind==1;stream.write(detailed?1u:0u,1);
            if(detailed){
                (void)stream.write_vector(damage.point,stream.primary_quantizer(2),7);
                stream.write_direction(damage.direction,3);
                const auto& value=damage.direction;
                const float square=static_cast<float>((static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2]);
                std::int32_t bits;std::memcpy(&bits,&square,4);
                const auto length=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
                stream.write(human_loading_stat_word(length),8);
            }
        }
        stream.write(24,5);stream.finish_broadcast();
    }catch(...){
        stream.position(before/32,static_cast<unsigned>(before%32));
        stream.word1c=active;stream.word20=priority;stream.word24=target;stream.word28=kind;stream.flag18=request;
        throw;
    }
}

//00540DB0 case6, reached through actual Scene2 authority dispatch. Reserved
//missing/nonplayer owners use00541CB0 and still consume the actor identity.
template<class Packet,class Request>
void receive_human_ladder_commands(Packet& packet,Request& request){
    for(;;){
        const auto kind=packet.read(5);
        if(kind==24)return;
        if(kind!=6)throw std::runtime_error("Human authority packet requires its actual command payload receiver");
        request(packet.read(10));
    }
}

} //namespace hd2::game
#endif
