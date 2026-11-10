#ifndef HD2_GAME_SESSION_LOCAL_PACKETS_H
#define HD2_GAME_SESSION_LOCAL_PACKETS_H

#include "frontend/control_frame.h"
#include "game/prepared_spawn_packets.h"
#include <list>

namespace hd2::game {

//0072C170/0072C230/0072C3B0/0072C5D0/0072CE70 ordinary offline local packet
//ownership. Borrow the SAME containing control state, process timer and three
//broadcast streams. Dispatcher supplies real loaded14/module packet receivers.
//This does not enable the host's currently cold control/world entry path.
template<class Backend,class Dispatcher>
class SessionLocalPackets {
public:
    struct Packet {std::uint32_t timestamp,flags;std::vector<unsigned char> bytes;};
    SessionLocalPackets(frontend::ControlStartupState& state,frontend::ColdControlFrameState& frame,
                        SessionBroadcastPackets& streams,HumanInputShared& shared,
                        std::uint32_t& mode,Backend& backend,Dispatcher& dispatcher)
        :state_(state),frame_(frame),streams_(streams),shared_(shared),mode_(mode),backend_(backend),dispatcher_(dispatcher){}
    SessionLocalPackets(const SessionLocalPackets&)=delete;
    SessionLocalPackets& operator=(const SessionLocalPackets&)=delete;
    void enqueue(bool authority,const std::vector<unsigned char>& bytes,std::uint32_t flags) {
        //Native allocation, clock and byte copy precede the queue lock.
        std::vector<unsigned char> copy(bytes.size());const auto timestamp=backend_.control_packet_clock();
        std::copy(bytes.begin(),bytes.end(),copy.begin());
        Lock lock(backend_,1);auto& queue=authority?authority_:presentation_;
        queue.push_back(Packet{timestamp,flags,std::move(copy)}); //Append at tail, preserve FIFO.
        frame_.queue_counts[authority?0u:1u]=checked_count(queue.size());
    }
    void phase(bool authority) { //0072C3B0: drain selected queue, unlock BEFORE any decoder call.
        require_offline();if(authority&&!(mode_&1u))return;
        for(;;) {
            std::optional<Packet> packet;
            {Lock lock(backend_,1);auto& queue=authority?authority_:presentation_;
                if(queue.empty())return;
                packet.emplace(std::move(queue.front()));queue.pop_front();
                frame_.queue_counts[authority?0u:1u]=checked_count(queue.size());}
            if(packet->flags&0x20u)throw std::runtime_error("Local packet requires actual input history decoder");
            SessionPacketStream reader(shared_);reader.receive(packet->bytes);
            while(reader.bit_count()<reader.received_bit_count()) {
                const auto module=reader.read(5); //0072C0B0, native20 message-kind bound→5bits.
                if(module==19)break;
                if(module==20)throw std::runtime_error("Local packet requires actual application control-message decoder");
                if(!dispatcher_.world_loaded())break; //Native missing/unloaded game returnsfalse.
                if(!dispatcher_.dispatch_packet(module,reader,authority))break; //0048DC50/70; absent module genuinelyfalse.
            }
            //Packet payload retires even on callback exception; later queued
            //packets remain owned. Recheck queue after callbacks may append.
        }
    }
    void maintain(std::uint32_t elapsed) { //0072CE70 offline7 reached branch.
        require_offline();
        if(mode_&1u) {
            Lock lock(backend_,0); //0072CD50, real record08/6C/74 gates.
            for(std::size_t n=0;n<state_.records.size();++n) {
                const auto active=state_.records[n].byte(8);
                if(!active)throw std::runtime_error("Local peer record has no actual active-byte producer");
                if(*active&&state_.records[n].word(0x6c))
                    throw std::runtime_error("Local maintenance requires actual connected peer receiver");
            }
        }
        flush(elapsed);
    }
    void flush(std::uint32_t elapsed) { //0072C5D0 offline7; network peers/input writes are genuine nonoffline branches.
        require_offline();state_.word5058+=elapsed;
        const auto period=state_.word5054;
        if(signed_word(period)>signed_word(state_.word5058))return;
        state_.word5058-=period;
        if(signed_word(period*2u)<signed_word(state_.word5058))state_.word5058=0;
        if(!frame_.timer_initialized) {
            frame_.timer_started=backend_.control_milliseconds();frame_.timer_initialized=true; //0073EA70(3000) writes enabled0.
        }
        if(frame_.timer_enabled)throw std::runtime_error("Local maintenance timer has an unrecovered enabled producer");
        frame_.packet_maintenance_seen=true; //0072CA10's0097C90C=1; disabled timer skips all remote work.
        for(auto* stream:{&streams_.first,&streams_.second,&streams_.third})if(stream->bit_count()) {
            stream->write(19,5); //Terminator is added only at FLUSH, not0072BF90 target0 finish.
            if((mode_&1u)&&(mode_&2u))enqueue(false,stream->written_bytes(),0);
            stream->position(0,0); //Reset cursor only AFTER successful enqueue.
        }
        if(frame_.request_words||frame_.request_bits)
            throw std::runtime_error("Local authority request stream requires its actual producer/decoder");
        frame_.pending_words=frame_.pending_bits=0; //Native2F8 scratch cursor reset.
    }
    void clear() { //0072E4E0 queue portion, same430 lock; no implicit decoder/flush.
        Lock lock(backend_,1);authority_.clear();presentation_.clear();frame_.queue_counts.fill(0);
    }
    const auto& authority_queue()const{return authority_;}const auto& presentation_queue()const{return presentation_;}
private:
    struct Lock {Backend& backend;unsigned index;Lock(Backend& value,unsigned slot):backend(value),index(slot){backend.enter_control_lock(index);}~Lock(){backend.leave_control_lock(index);}};
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    static std::uint32_t checked_count(std::size_t count){if(count>std::numeric_limits<std::uint32_t>::max())throw std::length_error("Local packet queue exceeds native count");return static_cast<std::uint32_t>(count);}
    void require_offline()const {if((mode_&7u)!=7u)throw std::runtime_error("Session packet pump requires actual nonoffline transport");}
    frontend::ControlStartupState& state_;frontend::ColdControlFrameState& frame_;SessionBroadcastPackets& streams_;HumanInputShared& shared_;
    std::uint32_t& mode_;Backend& backend_;Dispatcher& dispatcher_;std::list<Packet> authority_,presentation_;
};
} //namespace hd2::game
#endif
