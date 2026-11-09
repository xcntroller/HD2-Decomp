#ifndef HD2_GAME_MISSION_SCRIPT_VISIBILITY_H
#define HD2_GAME_MISSION_SCRIPT_VISIBILITY_H

#include "game/mission_script_context.h"
#include "game/mission_script_frame_snapshots.h"
#include "game/prepared_spawn_packets.h"

namespace hd2::game {

//00424540/00424580 writer and00424590→0042B650 decoder, required SetOn5F
//branch. Share the actual session broadcast streams/FIFO, not a local direct
//call. Model/Human visibility requires its real mesh/effect/player receivers.
template<class Frames,class Snapshots,class Models>class MissionScriptVisibility {
public:
    MissionScriptVisibility(Frames& frames,Snapshots& snapshots,Models& models,SessionBroadcastPackets& packets,
                            std::uint32_t& mode):frames_(frames),snapshots_(snapshots),models_(models),packets_(packets),mode_(mode){}
    template<class Context,class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        if(instruction.opcode!=0x5f)return std::nullopt;
        if(!(mode_&1u))return false;
        const auto index=instruction.operands.at(0),value=instruction.operands.at(1);
        //Native invalid POOL INDEX branches returntrue WITHOUT advancing.
        if(index>=context.slot_count()||value>=context.program().integers.size())return true;
        const auto frame=context.slot(index);
        if(frame) {
            if(frames_.frame_kind(frame)==4)queue_light(frame); //00423980 before scene update and packet creation.
            frames_.update_scene_object(frame);
            if((mode_&7u)!=7u)throw std::runtime_error("Script visibility requires actual nonoffline transport");
            auto& packet=packets_.begin(4,0); //DL1 in00424540 selects priorityZERO.
            packet.write(0x5f,8);packet.write_string(frames_.frame_name(frame));
            packet.write(context.program().integers[value].value!=0,1);packet.finish_broadcast();
        }
        context.advance(runtime);return true;
    }
    bool receive(SessionPacketStream* packet) { //00424590's null receiver returnsfalse.
        if(!packet)return false;
        const auto opcode=packet->read(8);
        if(!(mode_&2u))return true; //Wrapper consumes opcode;0042B650 genuinely does nothing without presentation.
        if(opcode!=0x5f)throw std::runtime_error("Required script presentation opcode is unavailable: "+std::to_string(opcode));
        const auto name=packet->read_string(0xfe);const bool enabled=packet->read(1)!=0;
        if(!frames_.has_scene())return true; //Payload is consumed even without the scene.
        auto frame=frames_.find_scene_frame(name,0xcffff); //Decoder reverses the compiler's lookup order.
        if(!frame)frame=frames_.resolve_scene_path(name);
        if(!frame)return true;
        const auto kind=frames_.frame_kind(frame);snapshots_.remember(frame,kind==4?0x30u:0x20u);
        if(kind==4)frames_.script_light_enabled(frame,enabled);
        else if(kind==9)models_.script_model_visibility(frame,enabled); //Actual0042BBxx Human/mesh/effects path, no success fallback.
        else {frames_.object_visible(frame,enabled);frames_.update_scene_object(frame);}
        return true;
    }
    void queue_light(typename Frames::Frame frame) { //00423980 borrowed, reverse identity dedup, then append.
        for(std::size_t n=lights_.size();n;)if(lights_[--n]==frame)return;
        lights_.push_back(frame);
    }
    const auto& pending_lights()const{return lights_;}
    void clear_lights(){lights_.clear();} //Containing module clears8AC BEFORE frame restoration.
private:
    Frames& frames_;Snapshots& snapshots_;Models& models_;SessionBroadcastPackets& packets_;std::uint32_t& mode_;
    std::vector<typename Frames::Frame> lights_;
};
} //namespace hd2::game
#endif
