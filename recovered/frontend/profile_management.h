#ifndef HD2_FRONTEND_PROFILE_MANAGEMENT_H
#define HD2_FRONTEND_PROFILE_MANAGEMENT_H
#include "frontend/control_events.h"
#include <cctype>
#include <string>
#include <vector>

namespace hd2::frontend {
// 00621470 trims trailing ANSI whitespace only. The original edit excludes
// these path characters and limits input to17 bytes; reject bypasses as well.
inline std::string profile_edit_name(std::string value) {
    while (!value.empty() && static_cast<unsigned char>(value.back()) < 128 &&
           std::isspace(static_cast<unsigned char>(value.back())))
        value.pop_back();
    return value;
}
inline bool profile_component(const std::string &name) {
    return !name.empty() && name.size() <= 17 && name.find('\0') == std::string::npos &&
           name.find_first_of("./\\*?|<>:\"") == std::string::npos;
}
// 006211E0/00621470/00623780/00623AE0/00622AF0. Own the selected spelling
// across modal replies and filesystem mutation; services own actual profile data.
template <class Services> class ProfileManagement {
  public:
    explicit ProfileManagement(Services &services) : services_(services) {}
    const std::string &selected() const { return selected_; }
    unsigned mode() const { return mode_; }
    void shown() {
        selected_ = services_.profile_name();
        mode_ = 0;
        services_.refresh_profiles();
        restore();
    }
    void choose(const std::string &name) {
        if (name.empty() || mode_ == 1 || mode_ == 2)
            return;
        selected_ = name;
        services_.select_profile(selected_);
        restore();
    }
    bool back() const { return mode_ != 3; }
    void begin(bool rename) {
        if (rename && selected_.empty())
            return;
        mode_ = rename ? 2 : 1;
        for (unsigned i = 2; i <= 10; ++i)
            enable(i, false);
        enable(1, true);
        text(rename ? selected_ : std::string{});
        if (!rename)
            for (unsigned i = 3; i <= 6; ++i)
                send(i, 0x02000076, 0);
        send(1, 0x02000016);
    }
    void cancel() {
        mode_ = 0;
        restore();
    }
    void submit(const std::string &edited) {
        const auto value = profile_edit_name(edited);
        if (mode_ != 1 && mode_ != 2)
            return;
        if (mode_ == 2 && value.empty()) {
            cancel();
            return;
        }
        const bool creating = mode_ == 1;
        const int result = !profile_component(value) ? -2
                           : creating                ? services_.create_profile(value)
                                                     : services_.rename_profile(selected_, value);
        if (result < 0) {
            services_.prompt(2,
                             creating ? (result == -1 ? 0x251c : 0x251d) : (result == -1 ? 0x2526 : 0x2527),
                             0x020000c3, 0);
            return;
        }
        selected_ = value;
        if (creating) {
            services_.select_profile(selected_);
            services_.save_profile();
        }
        services_.refresh_profiles();
        mode_ = creating ? 3 : 0;
        restore();
    }
    void difficulty(unsigned value) {
        if (mode_ != 3 || value < 1 || value > 4)
            return;
        services_.difficulty(value);
        services_.save_profile();
        mode_ = 0;
        restore();
    }
    void request_delete() {
        if (selected_.empty())
            return;
        pending_delete_ = selected_;
        services_.prompt(1, 2004, 0x02000017, 0);
    }
    bool dispatch(ControlEvent &event) {
        switch (event.code & 0x03000fffu) {
        case 0x0200000a:
            return !back(); // Consume Back while choosing new difficulty.
        case 0x02000017:
            if (pending_delete_.empty())
                return true;
            if (services_.delete_profile(pending_delete_)) {
                pending_delete_.clear();
                services_.refresh_profiles();
                selected_ = services_.first_profile();
                services_.select_profile(selected_);
                mode_ = 0;
                restore();
            }
            return true;
        case 0x020000c3:
            send(1, 0x02000016);
            return true;
        default:
            return false;
        }
    }
    void restore() {
        text(selected_);
        if (selected_.empty()) {
            ControlEvent e{0x08100000, 0x01000015, 13, 2002, 0, {}};
            services_.enqueue(e);
        }
        enable(1, false);
        enable(2, mode_ != 3);
        enable(7, mode_ != 3);
        enable(8, mode_ != 3 && !selected_.empty());
        enable(9, mode_ != 3 && !selected_.empty());
        enable(10, mode_ != 3);
        for (unsigned i = 3; i <= 6; ++i) {
            enable(i, mode_ == 3);
            send(i, 0x02000076, !selected_.empty() && services_.difficulty() == i - 2);
        }
        services_.select_list_name(selected_);
    }

  private:
    void send(unsigned control, unsigned code, unsigned argument = 0) {
        services_.enqueue({0x08000000u | (control << 20), code, argument, 0, 0, {}});
    }
    void enable(unsigned control, bool value) { send(control, value ? 0x01000002 : 0x01000001); }
    void text(const std::string &value) {
        ControlEvent e{0x08100000, 0x01000014, 13, 0, 0, {}};
        e.text = std::make_shared<const std::string>(value);
        services_.enqueue(e);
    }
    Services &services_;
    std::string selected_, pending_delete_;
    unsigned mode_ = 0;
};
} // namespace hd2::frontend
#endif
