#ifndef HD2_GAME_MISSION_TABLES_H
#define HD2_GAME_MISSION_TABLES_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace hd2::game {

//00523AB0/0051DD60/00523E10/00523B20. Native pointer-width representation
// of two0x3C datasets; no inferred record type or parser format. Their 0x24-byte
// record array has a DWORD cookie immediately before begin. The matching element
// cleanup00524610 tail-calls00524840; it frees+20 only when flag word+14 lacks
// bit0x0400. Record destruction is delegated to the required backend, allowing
// that original cookie/stride to stay at the allocation boundary on 64-bit hosts.
// No default allocator/free/destructor can silently stand in for a real owner.
struct MissionTableSet {
    std::uint16_t count0 = 0;
    void* records4 = nullptr;
    void* data8 = nullptr;
    std::uint16_t count0c = 0;
    void* data10 = nullptr;
    std::uint16_t count14 = 0;
    void* data18 = nullptr;
    std::uint16_t count1c = 0;
    void* data20 = nullptr;
    void* data24 = nullptr;
    void* data28 = nullptr;
    void* begin30 = nullptr;
    void* end34 = nullptr;
    void* limit38 = nullptr;
};

template<class Backend>
void clear_mission_table_set(MissionTableSet& set, Backend& backend) {
    if (set.records4) {
        backend.destroy_table_records(set.records4); // cookie, reverse0x24 element destructors
        backend.release_table_record_allocation(set.records4); // free(begin-4), not begin
    }
    set.records4 = nullptr;
    if (set.data8) backend.release_table_storage(set.data8);
    set.data8 = nullptr;
    set.count0 = 0;
    if (set.data10) backend.release_table_storage(set.data10);
    set.data10 = nullptr;
    set.count0c = 0;
    if (set.data18) backend.release_table_storage(set.data18);
    set.data18 = nullptr;
    set.count14 = 0;
    if (set.data20) backend.release_table_storage(set.data20);
    set.data20 = nullptr;
    set.count1c = 0;
    if (set.data24) backend.release_table_storage(set.data24);
    set.data24 = nullptr;
    if (set.data28) backend.release_table_storage(set.data28);
    set.data28 = nullptr;
    //0051DD60 deliberately preserves the vector and its capacity.
}

template<class Backend>
class MissionTables {
public:
    explicit MissionTables(Backend& backend) : backend_(backend) {}
    MissionTables(const MissionTables&) = delete;
    MissionTables& operator=(const MissionTables&) = delete;
    ~MissionTables() { close(); }
    std::array<MissionTableSet, 2>& sets() { return sets_; }
    const std::array<MissionTableSet, 2>& sets() const { return sets_; }
    void clear() {
        //00523E10 clears first then second; destructor00523B20 reverses them.
        clear_mission_table_set(sets_[0], backend_);
        clear_mission_table_set(sets_[1], backend_);
    }
    void close() {
        for (auto i = sets_.size(); i-- != 0;) {
            auto& set = sets_[i];
            clear_mission_table_set(set, backend_);
            backend_.release_table_storage(set.begin30); // Includes null.
            set.begin30 = set.end34 = set.limit38 = nullptr;
        }
    }
private:
    Backend& backend_;
    std::array<MissionTableSet, 2> sets_{};
};

} // namespace hd2::game
#endif
