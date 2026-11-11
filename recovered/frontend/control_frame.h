#ifndef HD2_FRONTEND_CONTROL_FRAME_H
#define HD2_FRONTEND_CONTROL_FRAME_H

#include "frontend/control_startup.h"
#include "frontend/cleanup.h"
#include "game/session_broadcast_packets.h"
#include <cstring>

namespace hd2::frontend {

// Cold, standalone branch of0072C3B0/0072CD50/0072C5D0/0072CE70 and0072E4E0.
// Actual constructor0072DE00 starts both queues empty;0073D370 starts streams
// empty. The independent native launch has no network producer. Populated queues,
// interfaces/peers or a mode change are unsupported, never treated as success.
struct ColdControlFrameState {
    std::array<std::uint32_t, 2> queue_counts{}; // +1F8/+204
    std::uint32_t pending_words = 0, pending_bits = 0; // +300/+304
    std::uint32_t saved_graphics_flags = 0; // +4DFC, static zero until mode-switch writer.
    std::uint32_t timer_started = 0;
    bool timer_initialized = false, timer_enabled = false; // 0073EA70
    bool packet_maintenance_seen = false; //0097C90C,0072CA10; containing session/process lifetime.
    bool firewall_changed = false; // static0097CC02, untouched in standalone launch.
    bool worker_active = false;
    void* server = nullptr;
    void* client = nullptr;
    std::int32_t local_peer = 0; //00982104, SAME signed Human/player identity, bit pattern preserved.
};

inline std::int32_t control_signed_word(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

template<class Backend>
class ColdControlFrame {
public:
    ColdControlFrame(ControlStartupState& startup, std::uint32_t& gate, Backend& backend,const game::SessionBroadcastPackets& broadcasts)
        : startup_(startup), gate_(gate), backend_(backend),broadcasts_(broadcasts) {}
    ColdControlFrameState& state() { return state_; }
    const ColdControlFrameState& state()const{return state_;}

    void phase(std::uint32_t, bool fixed) {
        require_cold();
        if (fixed && !(gate_ & 1)) return;
        Lock lock(backend_, 1); // Original+430 selected-phase queue lock.
        if (state_.queue_counts[fixed ? 0 : 1])
            throw std::logic_error("Control queue producer/decoder is not reconstructed");
    }
    void maintain(std::uint32_t milliseconds) {
        require_cold();
        if (gate_ & 1) {
            Lock lock(backend_, 0);
            for (const auto& record : startup_.records) {
                const auto active = record.byte(8);
                if (!active || *active) throw std::logic_error("Control peer activation is not reconstructed");
            }
        }
        flush(milliseconds);
    }
    void reset() {reset_impl(false,[&]{flush(1000);},[&]{Lock queues(backend_,1);state_.queue_counts.fill(0);});}
    //Actual packet owners flush/reset through the SAME native control fields.
    //Each stage runs ONCE, including1000ms flush; no preliminary cold reset.
    template<class Flush,class Clear>void reset_packets(Flush flush_packets,Clear clear_packets){
        reset_impl(true,[&]{flush_packets(1000);},clear_packets);
    }
    void shutdown(){shutdown_impl(false,[&]{reset();});}
    template<class Flush,class Clear>void shutdown_packets(Flush flush_packets,Clear clear_packets){
        shutdown_impl(true,[&]{reset_packets(flush_packets,clear_packets);});
    }
private:
    struct Lock {
        Backend& backend;
        unsigned index;
        Lock(Backend& value, unsigned slot) : backend(value), index(slot) { backend.enter_control_lock(index); }
        ~Lock() { backend.leave_control_lock(index); }
    };
    template<class Flush,class Clear>void reset_impl(bool packets,Flush flush_packets,Clear clear_packets){
        require_cold(packets); //Reject live transport or streams lacking their packet pump BEFORE mutation.
        backend_.restore_cold_firewall(state_.firewall_changed);
        backend_.restore_graphics_flags(state_.saved_graphics_flags);
        flush_packets();
        startup_.word448=0;startup_.byte4df8=0;startup_.word5044=0;gate_=7;
        {Lock peers(backend_,2);}
        {Lock records(backend_,0);startup_.records.clear();}
        clear_packets();state_.local_peer=0;
    }
    template<class Reset>void shutdown_impl(bool packets,Reset reset_control){
        if(closed_)return;
        require_cold(packets);
        if(startup_.object4ee0)throw std::logic_error("Control final resource owner is not reconstructed");
        closed_=true;CleanupErrors errors;errors.attempt(reset_control);
        if(startup_.locks5018){
            errors.attempt([&]{backend_.delete_control_critical_section(0);});
            errors.attempt([&]{backend_.delete_control_critical_section(1);});startup_.locks5018=0;
        }
        gate_=7;errors.rethrow();
    }
    void require_cold(bool packets=false) const {
        if (gate_ != 7 || state_.server || state_.client || state_.worker_active ||
                state_.firewall_changed || startup_.wrapper0c || startup_.wrapper10 ||
                startup_.service14 || startup_.handle4d3c != -1 || state_.timer_enabled)
            throw std::logic_error("Only standalone cold control lifetime is reconstructed");
        if(!packets&&broadcasts_.pending())throw std::logic_error("Control streams require their actual packet pump");
        if(!packets&&(state_.queue_counts[0]||state_.queue_counts[1]))
            throw std::logic_error("Control packet queues require their actual packet pump");
        for (const auto& record : startup_.records)
            if (!record.byte(8) || *record.byte(8))
                throw std::logic_error("Control peer ownership is not reconstructed");
    }
    void flush(std::uint32_t milliseconds) {
        startup_.word5058 += milliseconds; // x86 wrapping addition/signed comparison.
        const auto period = startup_.word5054;
        if (control_signed_word(period) > control_signed_word(startup_.word5058)) return;
        startup_.word5058 -= period;
        if (control_signed_word(period * 2) < control_signed_word(startup_.word5058))
            startup_.word5058 = 0;
        if (!state_.timer_initialized) {
            state_.timer_started = backend_.control_milliseconds();
            state_.timer_initialized = true;
            // 0073EA70 writes enabled0;0073EA90 returnsfalse without reading time.
        }
        state_.pending_words = 0;
        state_.pending_bits = 0;
    }
    ControlStartupState& startup_;
    std::uint32_t& gate_;
    Backend& backend_;
    const game::SessionBroadcastPackets& broadcasts_;
    ColdControlFrameState state_;
    bool closed_ = false;
};

} // namespace hd2::frontend
#endif
