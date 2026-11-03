#ifndef HD2_INPUT_BINDING_EDITOR_H
#define HD2_INPUT_BINDING_EDITOR_H
#include "input/binding_configuration.h"
#include <optional>

namespace hd2::input {
//006A8230: original stack DWORD is compared as a low byte; stdcall/AL/RET4.
HD2_CLIENT_BODY inline bool HD2_CLIENT_STDCALL reserved_binding_scan(std::uint32_t scan) {
    const auto byte=static_cast<std::uint8_t>(scan);
    return (byte>=0x3b && byte<=0x44) || byte==0x57 || byte==0x58 || byte==1 || byte==0xc5;
}
//006A8480: no eager selected-index read on an empty vector; AL is always zero.
inline std::uint8_t HD2_CLIENT_THISCALL BindingConfiguration::remove_conflicts(std::uint32_t index) {
    for(std::size_t i=0;settings.data()!=nullptr && i<settings.size();++i) {
        auto& other=settings[i];
        if(i==index || other.group==0xffffffffu) continue;
        const auto& selected=settings[index];
        if(other.code!=selected.code || other.device_kind!=selected.device_kind ||
                other.direction!=selected.direction) continue;
        if((other.caption==0xb28 && selected.caption==0xb20) ||
                (other.caption==0xb20 && selected.caption==0xb28)) continue;
        const bool overlap=selected.mode==1 ? other.mode>=1 && other.mode<=3
            : selected.mode==2 ? other.mode==1 || other.mode==2
            : selected.mode==3 && (other.mode==1 || other.mode==3);
        if(overlap) { other.device_kind=0;other.code=0;other.dirty=1; }
    }
    return 0;
}
//006A83C0 checks full arguments BEFORE truncating record fields.
inline bool HD2_CLIENT_THISCALL BindingConfiguration::edit(std::uint32_t index,std::uint32_t kind,
                                                          std::uint32_t code,std::uint32_t direction) {
    if((index&0x80000000u) || settings.data()==nullptr || index>=settings.size() ||
            (kind==1 && reserved_binding_scan(code)) || (kind==2 && (code==0x100 || code==0x200)))
        return false;
    auto& record=settings[index];
    record.device_kind=static_cast<std::uint16_t>(kind);
    record.code=static_cast<std::uint16_t>(code);
    record.direction=direction;record.dirty=1;
    remove_conflicts(index);
    return true;
}
//006A85B0: includes hidden records, DWORD index or FFFFFFFF, ECX/RET0.
inline std::uint32_t HD2_CLIENT_THISCALL BindingConfiguration::first_unassigned() const {
    for(std::size_t i=0;settings.data()!=nullptr && i<settings.size();++i)
        if(!settings[i].device_kind && !settings[i].code) return static_cast<std::uint32_t>(i);
    return 0xffffffffu;
}
//006A8290: write -1 before checking index, then one-based contiguous ordinal.
inline void HD2_CLIENT_THISCALL BindingConfiguration::group_ordinal(std::uint32_t index,std::int32_t* out) const {
    *out=-1;
    if((index&0x80000000u) || settings.data()==nullptr || index>=settings.size()) return;
    const auto group=settings[index].group;
    auto first=index;
    while(first && settings[first-1].group==group) --first;
    *out=static_cast<std::int32_t>(index-first+1);
}
// Native convenience adapters; optional/size_t/error policy is not original ABI.
inline void remove_binding_conflicts(BindingConfiguration& c,std::size_t index) {
    if(index>0xffffffffu || index>=c.settings.size()) {
        if(c.settings.empty()) return;
        throw std::out_of_range("Binding conflict index");
    }
    c.remove_conflicts(static_cast<std::uint32_t>(index));
}
inline bool edit_binding(BindingConfiguration& c,std::size_t index,std::uint32_t kind,
                         std::uint32_t code,std::uint32_t direction) {
    return index<=0xffffffffu && c.edit(static_cast<std::uint32_t>(index),kind,code,direction);
}
inline std::optional<std::size_t> first_unassigned_binding(const BindingConfiguration& c) {
    const auto index=c.first_unassigned();
    return index==0xffffffffu ? std::optional<std::size_t>{} : std::optional<std::size_t>{index};
}
inline std::size_t binding_group_ordinal(const BindingConfiguration& c,std::size_t index) {
    std::int32_t result=-1;
    if(index<=0xffffffffu) c.group_ordinal(static_cast<std::uint32_t>(index),&result);
    if(result<0) throw std::out_of_range("Binding group index");
    return static_cast<std::size_t>(result);
}
struct BindingCandidate {
    std::uint16_t kind, code;
    std::uint32_t direction;
};
// The Options mask0x9A in006C84A0 excludes all joystick paths. Keyboard
// modifiers precede the scan; mouse axes precede wheel, although edit rejects X/Y.
inline std::optional<BindingCandidate> options_binding_candidate(std::uint32_t modifiers, std::uint32_t scan,
                                                                 std::uint32_t buttons, std::int32_t x,
                                                                 std::int32_t y, std::int32_t z) {
    if (modifiers || (scan & 0xff)) {
        for (const auto pair : {std::pair<std::uint32_t, std::uint16_t>{0x20, 0x2a},
                                {0x40, 0x36},
                                {0x80, 0x1d},
                                {0x100, 0x9d},
                                {0x200, 0x38},
                                {0x400, 0xb8}})
            if (modifiers & pair.first)
                return BindingCandidate{1, pair.second, 0};
        return BindingCandidate{1, static_cast<std::uint16_t>(scan & 0xff), 0};
    }
    for (std::uint16_t bit = 1; bit <= 16; bit <<= 1)
        if (buttons & bit)
            return BindingCandidate{2, bit, 0};
    for (const auto pair : {std::pair<std::uint16_t, std::int32_t>{0x100, x}, {0x200, y}, {0x300, z}})
        if (pair.second > 10 || pair.second < -10)
            return BindingCandidate{2, pair.first, pair.second > 0 ? 1u : 2u};
    return {};
}
} // namespace hd2::input
#endif
