#ifndef HD2_FRONTEND_CONTROL_STARTUP_H
#define HD2_FRONTEND_CONTROL_STARTUP_H

#include "frontend/settings_sound.h"
#include "frontend/multiplayer_server.h"
#include "frontend/control_connection_records.h"
#include "game/session_network_parameters.h"
#include <bitset>
#include <optional>
#include <stdexcept>
#include <vector>

namespace hd2::frontend {

// Original 0xC4-byte record. Stack construction leaves many bytes unwritten.
// Backing zeros are inaccessible until defined, not invented original defaults.
class ControlStartupRecord {
public:
    static constexpr std::size_t size = 0xc4;
    std::optional<std::uint8_t> byte(std::size_t offset) const {
        if (offset >= size) throw std::out_of_range("Original control record byte");
        return defined_[offset] ? std::optional<std::uint8_t>(bytes_[offset]) : std::nullopt;
    }
    void byte(std::size_t offset, std::uint8_t value) {
        if (offset >= size) throw std::out_of_range("Original control record byte");
        bytes_[offset] = value;
        defined_.set(offset);
    }
    void word(std::size_t offset, std::uint32_t value) {
        if (offset > size - 4) throw std::out_of_range("Original control record word");
        if(offset==0x3c){
            if(value)throw std::logic_error("Control Human borrower cannot be manufactured from a DWORD");
            human3c_=nullptr;
        }
        if(offset==0x6c){
            if(value)throw std::logic_error("Control connection cannot be manufactured from a DWORD");
            connection6c_=nullptr;
        }
        for (unsigned i = 0; i != 4; ++i) byte(offset + i, static_cast<std::uint8_t>(value >> (i * 8)));
    }
    std::uint32_t word(std::size_t offset)const {
        if(offset>size-4)throw std::out_of_range("Original control record word");
        if(offset==0x3c&&human3c_)throw std::logic_error("Control Human borrower requires its typed access");
        if(offset==0x6c&&connection6c_)throw std::logic_error("Control connection requires its typed access");
        std::uint32_t value=0;
        for(unsigned n=0;n!=4;++n){const auto v=byte(offset+n);
            if(!v)throw std::runtime_error("Control record word has no original producer");
            value|=std::uint32_t{*v}<<(8*n);}
        return value;
    }
    void* human_actor()const{(void)byte(0x3c).value();return human3c_;}
    void human_actor(void* actor){word(0x3c,0);human3c_=actor;} //00612A60 borrows;0059E3E0/reset clears, never owns.
    ControlConnectionRecord* connection()const{
        for(unsigned n=0;n<4;++n)if(byte(0x6c+n).value())
            throw std::logic_error("Control connection requires its actual typed owner");
        return connection6c_;
    }
    void connection(ControlConnectionRecord* value){word(0x6c,0);connection6c_=value;}
    std::string name()const {
        std::string value;
        for(std::size_t n=8;n!=size;++n){const auto c=byte(n);
            if(!c)throw std::runtime_error("Control record name has no original producer");
            if(!*c)return value;
            value.push_back(static_cast<char>(*c));}
        throw std::runtime_error("Unterminated original control record name");
    }
private:
    std::array<std::uint8_t, size> bytes_{};
    std::bitset<size> defined_;
    void* human3c_=nullptr;
    ControlConnectionRecord* connection6c_=nullptr;
};

// 00725A50 via00725A40: only these bytes are written; reset preserves the rest.
inline void initialize_control_record(ControlStartupRecord& record) {
    record.byte(8, 0);
    for (std::size_t at = 0x40; at != 0x64; at += 4) record.word(at, 0);
    for (auto at : {0xb4, 0x6c, 0x74, 0x00, 0x3c, 0x7c}) record.word(at, 0);
    record.byte(0xa1, 0);
    record.byte(0xa2, 0);
    for (auto at : {0xa4, 0xac, 0xb0, 0xbc}) record.word(at, 0);
    record.word(0x68, 2);
    record.word(0x64, 2);
    record.word(0x70, 0xffffffffu);
    record.word(0xb8, 50);
}

// Records the original callback identity, not an executable original-client
// address. Actual0072C360 routes packed events to0072C170/0072C230; its consumer
// and those destinations must be reconstructed before enabling that service.
enum class ControlStartupCallback { packed_event_0072c360 };

// Consumed fields of singleton0097D070, not its complete constructor or ABI.
// Pointer owners are borrowed here. Initialization overwrites without release,
// matching the original; normal startup constructs the full owner beforehand.
struct ControlStartupState {
    game::SessionNetworkParameters network_parameters; //Process-shared packet/console values, not per-world copies.
    std::uint8_t byte08 = 0;
    SettingsControlInterface* wrapper0c = nullptr;
    void* wrapper10 = nullptr;
    void* service14 = nullptr;
    std::vector<ControlStartupRecord> records; // +1DC, elements at+1E0
    ControlConnectionRecord* client334=nullptr; //Distinct from transport client04 and voice0C. Original process BSS; borrowed here.
    std::optional<ControlStartupCallback> callback414;
    std::uint32_t word448 = 0, word4e4 = 0;
    //0097D070+44C/464 lie beyond .data's raw extent in the original image:
    //the loader zero-fills them. These are the SAME three process team records.
    std::array<float,3> team_scores44c{};
    std::array<std::array<std::uint32_t,9>,3> team_statistics464{};
    std::int32_t handle4d3c = 0;
    std::uint8_t byte4df8 = 0;
    MultiplayerServer server4e00; //0072DE00 constructs this distinct inline descriptor through00724EE0.
    void* object4ee0 = nullptr;
    std::uint8_t locks5018 = 0;
    std::uint32_t word503c = 0, word5044 = 0, word5054 = 0, word5058 = 0;
    std::uint32_t word505c = 0;
    std::array<std::array<std::uint32_t,7>,3> world_buffers{}; //4FB4/4FD0/4FEC,00724EC0.
    std::uint8_t byte5060 = 0;
    std::uint32_t word5064 = 0, word5068 = 0, word506c = 0;
};

// 0072E1A0, called at006DC1AA with ECX0097D070 before graphics initialization.
// Deliberately no gate test, device creation, interface factory or release.
// Backend owns the two real critical sections at original+418/+430.
template<class Backend>
void initialize_control_startup(ControlStartupState& state, std::uint32_t& gate,
                                Backend& backend) {
    state.object4ee0 = nullptr;
    state.word503c = 0;
    gate = 7;
    state.word5054 = 50;
    state.word5058 = 0;
    state.byte5060 = 1;
    state.word448 = 0;
    state.word4e4 = 0;
    backend.initialize_control_critical_section(0);
    backend.initialize_control_critical_section(1);
    state.locks5018 = 1;
    state.word5044 = 0;
    state.byte4df8 = 0;
    state.handle4d3c = -1;
    state.service14 = nullptr;
    state.word5064 = 0;
    state.word5068 = 50;
    state.word506c = 300000;
    state.wrapper10 = nullptr;
    state.wrapper0c = nullptr;
    state.byte08 = 0;
    state.callback414 = ControlStartupCallback::packed_event_0072c360;
    // APPEND, not clear/resize or insert at beginning. Original doubles capacity
    // when full. Native length/allocation failures throw instead of overflowing.
    ControlStartupRecord record;
    for (unsigned i = 0; i != 32; ++i) {
        initialize_control_record(record);
        if (state.records.size() == state.records.capacity()) {
            const auto count = state.records.size();
            const auto extra = count ? count : 1;
            if (extra > state.records.max_size() - count)
                throw std::length_error("Original control record capacity");
            state.records.reserve(count + extra);
        }
        state.records.push_back(record);
    }
}

}
#endif
