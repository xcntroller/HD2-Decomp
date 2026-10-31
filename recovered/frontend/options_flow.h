#ifndef HD2_FRONTEND_OPTIONS_FLOW_H
#define HD2_FRONTEND_OPTIONS_FLOW_H
#include "frontend/control_events.h"
#include "frontend/profile_settings.h"
#include "input/binding_editor.h"

namespace hd2::frontend {
// 0061BEA0; native services retain the actual selected profile, menu and input
// manager. In-game owner/family2 remains a separate delivery item.
template <class Services> class OptionsFlow {
  public:
    explicit OptionsFlow(Services &services) : services_(services) {}
    void profile_changed() { first_entry_=true; }
    bool dispatch(ControlEvent &event) {
        switch (event.code & 0x03000fffu) {
        case 0x0200000a:
            if (!services_.options_current())
                return false;
            send(0, 0x020000c4);
            return true;
        case 0x0200006f:
            services_.save_options(); // Original ignores open/write result.
            services_.apply_graphics_options();
            services_.apply_bindings();
            return true;
        case 0x02000071:
            if (first_entry_) {
                auto &settings = services_.profile_settings();
                send(0x604000, services_.gamma_available() ? 0x01000002 : 0x01000001);
                for (const auto pair : {std::pair<std::uint32_t, std::size_t>{0x604000, 4},
                                        {0x706000, 8},
                                        {0x707000, 12},
                                        {0x806000, 0},
                                        {0xb06000, 16}})
                    send(pair.first, 0x0200002b, settings_truncated_word(settings.real(pair.second)));
                send(0x605000, 0x0100006d, settings.word(0x18));
                for (const auto pair : {std::pair<std::uint32_t, std::size_t>{0x804000, 0x39},
                                        {0xa02000, 0x3a},
                                        {0xb04000, 0x38},
                                        {0xa03000, 0x3b}})
                    send(pair.first, 0x01000076, settings.bytes[pair.second]);
                send(0xb06000, settings.bytes[0x38] ? 0x01000002 : 0x01000001);
                services_.refresh_binding_lists();
                first_entry_ = false;
            }
            send(0, 0x02000001, 6);
            return true;
        case 0x020000c4:
            if (input::first_unassigned_binding(services_.profile_bindings()))
                services_.request_unassigned_dialog();
            else {
                for (std::uint32_t page = 6; page <= 11; ++page)
                    send(page << 20, 0x01000004);
                send(0, 0x0600106f);
                send(0, 0x06001003);
            }
            return true;
        case 0x020000c7: {
            auto &bindings = services_.profile_bindings();
            const auto missing = input::first_unassigned_binding(bindings);
            if (!missing)
                return true; // Native guard against a changed reply context.
            const auto group = bindings.settings[*missing].group;
            if (group > 4)
                throw std::logic_error("Unassigned hidden binding has no original Options row");
            for (std::uint32_t page = 6; page <= 11; ++page)
                send(page << 20, page == 9 ? 0x01000003 : 0x01000004);
            send(0x902000, 0x0100006d, group);
            send(0x900000 | ((group + 4) << 12), 0x020000aa,
                 static_cast<std::uint32_t>(input::binding_group_ordinal(bindings, *missing) - 1));
            return true;
        }
        default:
            return false;
        }
    }

  private:
    void send(std::uint32_t target, std::uint32_t code, std::uint32_t argument = 0) {
        services_.enqueue({target ? 0x18000000u | target : 0, code, argument, 0, 0, {}});
    }
    Services &services_;
    bool first_entry_ = true;
};
} // namespace hd2::frontend
#endif
