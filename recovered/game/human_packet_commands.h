#ifndef HD2_GAME_HUMAN_PACKET_COMMANDS_H
#define HD2_GAME_HUMAN_PACKET_COMMANDS_H

#include "game/prepared_spawn_packets.h"
#include "game/human_frame_inputs.h"

namespace hd2::game {

//Native command objects own values, not borrowed packet/actor pointers. Scalar
//offsets match the original command payload, without its vtable/allocator ABI.
//Command18 stores its scalar88/8C/90 at8/C/10; text has its own typed owner.
//The only identity shortcut is the already-published kind6 producer's +8 word.
struct HumanPacketCommand {
    std::uint32_t kind=0;
    std::int32_t actor_identity=-1;
    HumanJointWords<0x60> fields{};
    std::array<HumanFrameInputBuffer,2> inputs{};
    struct InventoryClaim {std::uint32_t slot,definition,identity;};
    std::vector<InventoryClaim> inventory;
    std::string text;
    frontend::PreMissionSoldier equipment;
};

enum class HumanPacketGrammar {authority_discard,authority_player,presentation_discard,presentation_player,presentation_remote};
inline bool human_packet_payload(std::uint32_t kind,HumanPacketGrammar grammar){
    if(kind>=24||kind==5)return false; //Original switch default: consume tag only.
    if(grammar==HumanPacketGrammar::authority_discard||grammar==HumanPacketGrammar::authority_player){
        if(kind>=18&&kind<=20)return false;
        if(grammar==HumanPacketGrammar::authority_player&&(kind==1||kind==13||kind==14))return false;
    }else{
        if(kind==17||kind==21)return false;
        if(grammar==HumanPacketGrammar::presentation_player&&kind==18)return false;
    }
    return true;
}
inline std::int32_t human_packet_signed16(std::uint32_t value){
    const auto word=static_cast<std::uint16_t>(value);std::int16_t result;std::memcpy(&result,&word,2);return result;
}
inline std::uint32_t human_packet_ftol(long double value){
    if(!std::isfinite(value)||value<-9223372036854775808.L||value>=9223372036854775808.L)
        throw std::runtime_error("Human command exceeds native __ftol domain");
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(std::trunc(value)));
}

//005B10C0..005B3060. See p22-pack-implementation.md for paired entries/fields.
//Mode7 is significant for command12. Command20's actual effect is deliberately
//separate from decoding: even005418E0 must invoke it after the full equipment read.
inline HumanPacketCommand read_human_packet_command(SessionPacketStream& packet,std::uint32_t kind,std::uint32_t mode){
    HumanPacketCommand command;command.kind=kind;auto& f=command.fields;
    const auto word=[&](unsigned offset,unsigned width){f.set(offset,packet.read(width));};
    const auto short_word=[&](unsigned offset){f.set(offset,human_packet_signed16(packet.read(16)));};
    switch(kind){
    case 0:break; //005AC760 is MOV AL,1;RET4. Both 0x100-byte inputs remain zero.
    case 1:
        word(0x10,10);word(8,4);word(0xc,4);short_word(0x14);
        if(packet.read(1)){
            f.set(0x20,packet.read_vector(packet.primary_quantizer(2),7));
            auto direction=packet.read_direction(3);const auto length=packet.read(8);
            for(unsigned n=0;n<3;++n)direction[n]=static_cast<float>(static_cast<long double>(direction[n])*length);
            f.set(0x30,direction);
        }else f.set(0x30,HumanVector{{1,0,0,0}});
        break;
    case 2:short_word(8);break;
    case 3:word(8,6);short_word(0xc);word(0x10,8);break;
    case 4:{ //005AC660, unlike the no-payload input command0.
        f.set(0x10,packet.read_vector(packet.primary_quantizer(-1),7));
        auto velocity=packet.read_vector(packet.primary_quantizer(-1),7);
        constexpr double epsilon=0.01; //00811968, checked against original bytes.
        if(std::abs(velocity[0])<epsilon&&std::abs(velocity[1])<epsilon&&std::abs(velocity[2])<epsilon)
            velocity[0]=velocity[1]=velocity[2]=0;
        f.set(0x20,velocity);f.set(0x30,packet.read_direction(-1));word(0x50,9);break;
    }
    case 6:command.actor_identity=static_cast<std::int32_t>(packet.read(10));f.set(8,command.actor_identity);break;
    case 7:
        short_word(8);word(0x10,3);word(0x14,3);word(0x18,4);
        f.set(0xc,packet.read(1)?static_cast<std::int32_t>(packet.read(10)):-1);break;
    case 8:
        short_word(8);short_word(0x14);word(0xc,3);word(0x18,3);
        f.set(0x10,packet.read(1)?static_cast<std::int32_t>(packet.read(10)):-1);break;
    case 9:short_word(8);word(0xc,3);break;
    case 10:
        short_word(8);f.set(0x10,packet.read_direction(-1));
        f.set(0x20,std::clamp(static_cast<float>(static_cast<long double>(packet.read(4))/15.L*4.L+1.L),1.f,5.f));
        f.set(0x24,packet.read(1)?human_packet_signed16(packet.read(16)):-1);break;
    case 11:
        word(8,10);word(0xc,4);
        //Native omits these stores when absent. Source uses the writer's slot7
        //sentinel instead of exposing indeterminate heap/stack bytes.
        f.set(0x10,std::int32_t{-1});f.set(0x14,7u);
        if(packet.read(1)){short_word(0x10);word(0x14,3);}break;
    case 12:
        for(auto offset:{0x10u,0x14u,0x18u})word(offset,32);
        word(0x40,6);for(auto offset:{0x30u,0x34u,0x38u})word(offset,32);
        f.set(0x24,3u);f.set(0x44,0xffffffffu);f.set(0x4c,0xffffffffu);
        if((mode&7u)==7u)for(auto offset:{0x50u,0x48u,0x44u,0x4cu})word(offset,32);
        break;
    case 13:
        for(;;){const auto slot=packet.read(3);if(slot==7)break;
            const auto definition=packet.read(32),identity=packet.read(32);
            command.inventory.push_back({slot,definition,identity});}
        break;
    case 14:word(8,10);short_word(0xc);word(0x10,10);word(0x14,1);break;
    case 15:case 16:word(8,4);break;
    case 17:word(0x10,10);word(8,4);word(0xc,4);word(0x14,10);break;
    case 18:command.text=packet.read_string(0x80);short_word(8);short_word(0xc);word(0x10,1);break;
    case 19:word(8,8);word(0xc,1);break;
    case 20:short_word(8);read_prepared_soldier(packet,command.equipment,true);break;
    case 21:short_word(8);break;
    case 22:case 23:word(8,1);break;
    default:throw std::logic_error("No Human command payload for switch hole");
    }
    return command;
}

inline void write_human_packet_command(SessionPacketStream& packet,const HumanPacketCommand& command,std::uint32_t mode){
    const auto kind=command.kind;const auto& f=command.fields;
    if(kind>=24||kind==5)throw std::invalid_argument("No native Human command serializer for switch hole");
    const auto word=[&](unsigned offset,unsigned width){packet.write(f.get<std::uint32_t>(offset),width);};
    packet.write(kind,5);
    switch(kind){
    case 0:break; //005B1100→005AC650 returnsfalse without writing any input.
    case 1:
        word(0x10,10);word(8,4);word(0xc,4);word(0x14,16);
        if(f.get<std::uint32_t>(8)!=1){packet.write(0,1);break;}
        packet.write(1,1);(void)packet.write_vector(f.get<HumanVector>(0x20),packet.primary_quantizer(2),7);
        {const auto value=f.get<HumanVector>(0x30);packet.write_direction(value,3);
            const float square=static_cast<float>((static_cast<long double>(value[0])*value[0]+
                static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2]);
            //005B1540 compares stored length² as an IEEE signed word, then as FLOAT.
            std::int32_t square_word;std::memcpy(&square_word,&square,4);
            packet.write(human_packet_ftol(square_word<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9L?
                1.L:std::sqrt(static_cast<long double>(square))),8);}
        break;
    case 2:word(8,16);break;
    case 3:word(8,6);word(0xc,16);packet.write(static_cast<std::uint8_t>(f.get<std::uint32_t>(0x10)),8);break;
    case 4:
        (void)packet.write_vector(f.get<HumanVector>(0x10),packet.primary_quantizer(-1),7);
        (void)packet.write_vector(f.get<HumanVector>(0x20),packet.primary_quantizer(-1),7);
        packet.write_direction(f.get<HumanVector>(0x30),-1);word(0x50,9);break;
    case 6:packet.write(static_cast<std::uint32_t>(command.actor_identity),10);break;
    case 7:
        word(8,16);word(0x10,3);word(0x14,3);word(0x18,4);
        packet.write(f.get<std::int32_t>(0xc)>=0,1);if(f.get<std::int32_t>(0xc)>=0)word(0xc,10);break;
    case 8:
        word(8,16);word(0x14,16);word(0xc,3);word(0x18,3);
        packet.write(f.get<std::int32_t>(0x10)>=0,1);if(f.get<std::int32_t>(0x10)>=0)word(0x10,10);break;
    case 9:word(8,16);word(0xc,3);break;
    case 10:
        word(8,16);packet.write_direction(f.get<HumanVector>(0x10),-1);
        //At20A7 [S+08]=mask15; PUSH at20AB makes ESP=S-4.
        //FIMUL [ESP+0C] at20B2 reads [S+08], still15, not high zero[S+0C].
        packet.write(human_packet_ftol((static_cast<long double>(f.get<float>(0x20))-1)*.25L*15.L),4);
        packet.write(f.get<std::int32_t>(0x24)>=0,1);if(f.get<std::int32_t>(0x24)>=0)word(0x24,16);break;
    case 11:
        word(8,10);word(0xc,4);packet.write(f.get<std::uint32_t>(0x14)!=7u,1);
        if(f.get<std::uint32_t>(0x14)!=7u){word(0x10,16);word(0x14,3);}break;
    case 12:
        for(auto offset:{0x10u,0x14u,0x18u})word(offset,32);
        word(0x40,6);for(auto offset:{0x30u,0x34u,0x38u})word(offset,32);
        if((mode&7u)==7u)for(auto offset:{0x50u,0x48u,0x44u,0x4cu})word(offset,32);
        break;
    case 13:
        for(const auto& value:command.inventory){if(value.slot>=7)throw std::invalid_argument("Inventory claim slot exceeds native grammar");
            packet.write(value.slot,3);packet.write(value.definition,32);packet.write(value.identity,32);}
        packet.write(7,3);break;
    case 14:word(8,10);word(0xc,16);word(0x10,10);packet.write(f.get<std::uint32_t>(0x14)!=0,1);break;
    case 15:case 16:word(8,4);break;
    case 17:packet.write(static_cast<std::uint32_t>(std::clamp(f.get<std::int32_t>(0x10),0,1023)),10);
        word(8,4);word(0xc,4);word(0x14,10);break;
    case 18:packet.write_string(command.text);word(8,16);word(0xc,16);packet.write(f.get<std::uint32_t>(0x10)!=0,1);break;
    case 19:word(8,8);packet.write(f.get<std::uint32_t>(0xc)!=0,1);break;
    case 20:{word(8,16);auto soldier=command.equipment;write_prepared_soldier(packet,soldier,true);break;}
    case 21:word(8,16);break;
    case 22:case 23:packet.write(f.get<std::uint32_t>(8)!=0,1);break;
    default:throw std::logic_error("Unreachable Human serializer kind");
    }
}

//Receiver supplies a fresh grammar before EVERY command and applies completed
//values only. No raw packet/actor/state borrower is retained by decoded storage.
template<class Receiver>void receive_human_packet_commands(SessionPacketStream& packet,std::uint32_t mode,Receiver& receiver){
    for(;;){const auto kind=packet.read(5);if(kind==24)return;
        const auto grammar=receiver.grammar();if(!human_packet_payload(kind,grammar))continue;
        auto command=read_human_packet_command(packet,kind,mode);
        receiver.receive(command,grammar);
    }
}
} //namespace hd2::game
#endif
