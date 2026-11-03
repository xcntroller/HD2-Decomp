#ifndef HD2_INPUT_BINDING_CONFIGURATION_H
#define HD2_INPUT_BINDING_CONFIGURATION_H

#include "input/binding_installation.h"
#include "frontend/original_abi.h"

namespace hd2::input {

// Profile/configuration record, distinct from the manager's twelve-byte binding.
// Keep group/caption as numeric metadata; their UI/string services are separate.
struct BindingSetting {
    std::int32_t index;
    std::uint16_t device_kind, code;
    std::uint32_t direction, mode, group, caption;
    std::uint8_t dirty;
    std::array<std::uint8_t, 3> padding{};
};
static_assert(sizeof(BindingSetting) == 0x1c && offsetof(BindingSetting, dirty) == 0x18);
static_assert(offsetof(BindingSetting,device_kind)==4 && offsetof(BindingSetting,code)==6 &&
              offsetof(BindingSetting,group)==0x10 && offsetof(BindingSetting,caption)==0x14);
struct BindingConfiguration {
    std::uint32_t header = 0; // Original byte/opaque prefix, not constructor recovery.
    std::vector<BindingSetting> settings;
    std::uint32_t mode = 2; // Constructor006A81C0/+10, before profile/default loading.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL clear_settings() { settings.clear(); }
    HD2_CLIENT_BODY std::uint32_t HD2_CLIENT_THISCALL first_unassigned() const;
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL group_ordinal(std::uint32_t index,std::int32_t* out) const;
    HD2_CLIENT_BODY bool HD2_CLIENT_THISCALL edit(std::uint32_t index,std::uint32_t kind,
                                                std::uint32_t code,std::uint32_t direction);
    HD2_CLIENT_BODY std::uint8_t HD2_CLIENT_THISCALL remove_conflicts(std::uint32_t index);
};
#if defined(__i386__) || defined(_M_IX86)
// These nonallocating bodies use the original pointer-triple vector ABI.
// Constructor/growth/destructor/manager ownership are separate partial bodies.
static_assert(sizeof(std::vector<BindingSetting>)==12);
static_assert(offsetof(BindingConfiguration,settings)==4 && offsetof(BindingConfiguration,mode)==0x10);
#endif

// 006a8600 ignores installer failure, forces device index zero and then clears
// the record's dirty byte. Record storage must survive the backend callbacks.
template<class Services>
void apply_binding_setting(BindingInputState& input, Services& services, BindingSetting& setting) {
    set_input_binding(input, services, setting.index, setting.device_kind,
                       setting.code, 0, setting.direction);
    setting.dirty = 0;
}

// 006a8630: reverse iteration, exact mode match, irrespective of dirty state.
// Resolve each vector element again after callbacks; appended records aren't
// visited in this pass. Callbacks must not invalidate an active borrowed record.
template<class Services>
void apply_binding_mode(BindingConfiguration& configuration, BindingInputState& input,
                         Services& services, std::uint32_t mode) {
    for (auto count = configuration.settings.size(); count != 0; --count) {
        const auto index = count - 1;
        if (configuration.settings.at(index).mode == mode)
            apply_binding_setting(input, services, configuration.settings.at(index));
    }
}

// 006a8690 installs before storing the selected mode. Invalid modes do nothing.
template<class Services>
void select_binding_mode(BindingConfiguration& configuration, BindingInputState& input,
                          Services& services, std::uint32_t mode) {
    if (mode != 2 && mode != 3) return;
    apply_binding_mode(configuration, input, services, mode);
    configuration.mode = mode;
}

// 006a86c0: mode 2/3 gate once at entry; reread current mode for each dirty
// record. Only records tagged 2/3 require that mode; other tags are shared here.
template<class Services>
void apply_dirty_bindings(BindingConfiguration& configuration, BindingInputState& input,
                           Services& services) {
    if (configuration.mode != 2 && configuration.mode != 3) return;
    for (auto count = configuration.settings.size(); count != 0; --count) {
        const auto index = count - 1;
        const auto& setting = configuration.settings.at(index);
        if (!setting.dirty) continue;
        if (setting.mode == 2 && configuration.mode != 2) continue;
        if (setting.mode == 3 && configuration.mode != 3) continue;
        apply_binding_setting(input, services, configuration.settings.at(index));
    }
}

// 006a8740 resets the settings vector's end only. It neither clears installed
// manager bindings nor resets mode; native POD records have no teardown callback.
inline void clear_binding_settings(BindingConfiguration& configuration) {
    configuration.clear_settings();
}

}
#endif
