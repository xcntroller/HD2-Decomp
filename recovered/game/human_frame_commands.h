#ifndef HD2_GAME_HUMAN_FRAME_COMMANDS_H
#define HD2_GAME_HUMAN_FRAME_COMMANDS_H
#include <utility>

#include "game/human_packet_commands.h"
#include "game/human_loading_stats.h"
#include "game/human_damage_commands.h"

namespace hd2::game {
//00592220 Remote virtualFC forwards only7/8/10, then clears the WHOLE owning
//queue. A borrowed range preserves queued objects until successful publication;
//it allocates no substitute queue and never calls prepare() for discarded kinds.
template<class Commands>class HumanFrameCommandRange {
    using Cursor=decltype(std::declval<const Commands&>().begin());
public:
    class Iterator {
    public:
        Iterator(Cursor current,Cursor end,bool remote):current_(current),end_(end),remote_(remote){skip();}
        const auto& operator*()const{return *current_;}
        Iterator& operator++(){++current_;skip();return *this;}
        bool operator==(const Iterator& other)const{return current_==other.current_;}
        bool operator!=(const Iterator& other)const{return !(*this==other);}
    private:
        void skip(){while(current_!=end_&&remote_&&current_->kind!=7&&current_->kind!=8&&current_->kind!=10)++current_;}
        Cursor current_,end_;bool remote_;
    };
    HumanFrameCommandRange(const Commands& commands,bool remote):commands_(commands),remote_(remote){}
    Iterator begin()const{return {commands_.begin(),commands_.end(),remote_};}
    Iterator end()const{return {commands_.end(),commands_.end(),remote_};}
    bool empty()const{return begin()==end();}
private:
    const Commands& commands_;bool remote_;
};
template<class Packet>HumanDamageCommand read_human_damage_command(Packet& packet){return human_damage_command_value(read_human_packet_command(packet,1,7));}
template<class Packet>HumanDropCommand read_human_drop_command(Packet& packet){return human_drop_command_value(read_human_packet_command(packet,10,7));}
template<class Packet,class Damage,class Drop,class Rank>void receive_human_presentation_commands(Packet& packet,Damage&& damage,Drop&& drop,Rank&& rank){ //005406C0/005418E0 reached damage/drop/rank grammar.
    for(;;){const auto kind=packet.read(5);if(kind==24)return;
        if(kind==1)damage(read_human_damage_command(packet));
        else if(kind==10)drop(read_human_drop_command(packet));
        else if(kind==16)rank(packet.read(4)); //005B0E50→005B0EC0.
        else throw std::runtime_error("Human presentation packet requires its actual command payload receiver");
    }
}
template<class Packet,class Damage,class Drop>void receive_human_presentation_commands(Packet& packet,Damage&& damage,Drop&& drop){
    receive_human_presentation_commands(packet,std::forward<Damage>(damage),std::forward<Drop>(drop),[](std::uint32_t){throw std::runtime_error("Rank packet requires its actual presentation receiver");});
}

//00577B30/Player0053F370: lazily opened command groups, motion4 uses native
//booleanfalse→priority3(clamped2), all other supported commands true→priority0.
//Native finalization is motion THEN ordinary. Authority FIRST may alias both
//groups to request2BC; retain its exact append order rather than invent a split.
//Prepare rereads actual referenced inventory at serialization time for13/20.
template<class Commands,class Prepare>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands,Prepare&& prepare){
    if(commands.empty())return;
    struct Snapshot{
        SessionPacketStream* stream;std::size_t before;
        std::uint32_t active,priority,target,kind;bool request;
    };
    std::array<Snapshot,2> snapshots{};unsigned touched=0;
    SessionPacketStream* ordinary=nullptr;SessionPacketStream* motion=nullptr;
    const auto begin=[&](unsigned priority)->SessionPacketStream&{
        auto& stream=second?streams.broadcast_stream(priority):streams.request_stream(priority,mode);
        bool saved=false;for(unsigned n=0;n<touched;++n)if(snapshots[n].stream==&stream)saved=true;
        if(!saved)snapshots[touched++]={&stream,stream.bit_count(),stream.word1c,stream.word20,stream.word24,stream.word28,stream.flag18};
        stream.begin(13,priority,0,second);stream.write(static_cast<std::uint32_t>(identity),10);return stream;
    };
    try{
        for(const auto& command:commands){
            auto*& stream=command.kind==4?motion:ordinary;
            if(!stream)stream=&begin(command.kind==4?3u:0u);
            write_human_packet_command(*stream,prepare(command),mode);
        }
        for(auto* stream:{motion,ordinary})if(stream){stream->write(24,5);stream->finish_broadcast();}
    }catch(...){
        for(unsigned n=0;n<touched;++n){const auto& saved=snapshots[n];auto& stream=*saved.stream;
            stream.position(saved.before/32,static_cast<unsigned>(saved.before%32));
            stream.word1c=saved.active;stream.word20=saved.priority;stream.word24=saved.target;stream.word28=saved.kind;stream.flag18=saved.request;
        }
        throw;
    }
}

template<class Commands>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands){
    write_human_frame_commands(streams,mode,identity,second,commands,[](const HumanPacketCommand& value)->const auto&{return value;});
}

} //namespace hd2::game
#endif
