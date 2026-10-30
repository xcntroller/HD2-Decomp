#ifndef HD2_INPUT_BINDING_CONFIGURATION_H
#define HD2_INPUT_BINDING_CONFIGURATION_H

#include "input/binding_installation.h"

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
struct BindingConfiguration {
    std::vector<BindingSetting> settings;
    std::uint32_t mode = 2; // Constructor006A81C0/+10, before profile/default loading.
};

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
    configuration.settings.clear();
}

}
#endif
