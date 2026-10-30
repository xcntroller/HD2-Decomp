#ifndef HD2_FRONTEND_STARTUP_CONFIGURATION_H
#define HD2_FRONTEND_STARTUP_CONFIGURATION_H

#include "frontend/legacy_text.h"
#include "frontend/startup_language.h"
#include <utility>

namespace hd2::frontend {

// Actual three-string application+08 owner, separate from application+20's
// current profile/options/payload. Native strings replace its 12-byte COW ABI.
// 006B4A60 zeroes the three pointers then calls load006A60A0 while constructing
// the application, BEFORE constructing profile/HumanStore and before startup
// text/archive setup. Do not defer this load until profile selection.
struct StartupConfiguration {
    std::string cd_key;      // Original +00; never logged by this component.
    std::string parent_lock; // +04; read key really differs from the write key.
    std::string name;        // +08, queried by 006A6560.

    StartupConfiguration() = default;
    explicit StartupConfiguration(std::string initial_name) : name(std::move(initial_name)) {}
    const std::string& current_profile() const { return name; }
    bool has_parent_lock() const { return !parent_lock.empty(); } // 006A64B0

    // 006A60A0 ignores the file and lookup booleans, clearing scratch before
    // EACH lookup, assigns all three strings and returns true even on no file.
    // Its 100-byte buffer was unbounded; reject >=100 native string results.
    template<class Files, class Directory>
    bool load(Files& files, Directory& directory) {
        std::vector<std::string> lines;
        const auto path = directory.get() + "\\game.cfg";
        load_legacy_text_lines(files, path.c_str(), lines);
        const auto read = [&](const char* key) {
            const auto* value = legacy_text_value(lines, key);
            const std::string result(value ? value : "");
            if (result.size() >= 100) throw std::length_error("Original game.cfg scratch exceeded");
            return result;
        };
        name = read("LASTPROFIL");
        parent_lock = read("PARENTLOC");
        cd_key = read("NETCDKEY");
        return true;
    }

    // 006A5F80 reads and writes RELATIVE game.cfg, unlike load's absolute path.
    // Preserve old unknown/duplicate lines, update these three keys in order,
    // perform real write and ignore its result. The unconditional true is the
    // recovered outer behavior, not a substituted successful persistence API.
    template<class Files>
    bool save(Files& files) const {
        std::vector<std::string> lines;
        load_legacy_text_lines(files, "game.cfg", lines);
        set_legacy_text_value(lines, "LASTPROFIL", name.c_str());
        set_legacy_text_value(lines, "PARENTLOCK", parent_lock.c_str());
        set_legacy_text_value(lines, "NETCDKEY", cd_key.c_str());
        save_legacy_text_lines(files, "game.cfg", lines);
        return true;
    }
    // 006A6320/64E0/6570 mutate before persistence; no rollback on file failure.
    template<class Files> bool set_parent_lock(Files& files, const std::string& value) {
        parent_lock = value.c_str(); return save(files);
    }
    template<class Files> bool set_cd_key(Files& files, const std::string& value) {
        cd_key = value.c_str(); return save(files);
    }
    template<class Files> bool set_profile(Files& files, const std::string& value) {
        name = value.c_str(); return save(files);
    }
    // 006A63A0: case-insensitive comparison, clear lock BEFORE saving.
    template<class Files> bool clear_parent_lock(Files& files, const std::string& value) {
        if (!startup_case_equal(parent_lock, value)) return false;
        parent_lock.clear();
        return save(files);
    }
};

// Existing bounded accessor users keep their name, now backed by the actual
// configuration owner instead of an isolated supplied profile string.
using StartupProfileName = StartupConfiguration;

}
#endif
