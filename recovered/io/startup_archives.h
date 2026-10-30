#ifndef HD2_IO_STARTUP_ARCHIVES_H
#define HD2_IO_STARTUP_ARCHIVES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace hd2::io {

// Client 007e5f33's arithmetic. Caller supplies the shared per-thread CRT state;
// this does not reconstruct TLS lookup, choose a seed or create a private stream.
inline std::uint32_t next_client_random(std::uint32_t& state) {
    state = state * 0x343fdu + 0x269ec3u;
    return (state >> 16) & 0x7fffu;
}

// 006db7e6..006db82b: consume these THREE calls before language resolution and
// the original protection prelude. Each present archive later consumes one more.
template<class Random>
std::uint64_t begin_archive_key_mask(Random& random) {
    std::uint64_t mask = 0;
    for (unsigned index = 0; index != 3; ++index)
        mask = (mask << 16) | (random() & 0xffffu);
    return mask;
}

inline std::string language_archive_name(const std::string& language) {
    // Original Lang%s.dta formatting reads a C string, without case conversion.
    return "Lang" + language.substr(0, language.find('\0')) + ".dta";
}

inline constexpr std::array<std::uint64_t, 16> startup_archive_keys{{
    0xa0a0a0a0a0a0a0a4ull, 0xa0a0a0a0a0a0a0a3ull,
    0xa0a0a0a0a0a0a0a2ull, 0xa0a0a0a0a0a0a0a1ull,
    0xa0a0a0a0a0a0a0f1ull, 0xa0a0a0a0a0a0a0a0ull,
    0xa0a0a0a0a0a0a0a0ull, 0x10acb2525d805270ull,
    0x10acb2525d805259ull, 0x22bcda987654a3f0ull,
    0xf26527fab438d0a5ull, 0x8d2965ca4fe85106ull,
    0xcf7612980ab4e72dull, 0x65f7ab23ea85902aull,
    0xe26520fab038d0a0ull, 0xf26520fab038d1a1ull
}};

// Readable archive region of 006db6e0, AFTER its unresolved protection-state
// prelude, and archive-only teardown at 006dc5b0. This is not full startup.
// Backend's actual dtaCreate registers in rw_data's shared search context;
// no invented AddDir/mount calls or successful placeholder services are used.
// Backend/module must outlive this owner. Callbacks must not structurally mutate
// the slots and release must not throw. Normal original global slots start zero.
template<class Backend>
class StartupArchives {
public:
    explicit StartupArchives(Backend& backend) : backend_(backend) {}
    ~StartupArchives() { close(); }
    StartupArchives(const StartupArchives&) = delete;
    StartupArchives& operator=(const StartupArchives&) = delete;

    void create(const std::string& language_archive) {
        if (created_) throw std::logic_error("Startup archives already created");
        created_ = true; // Native exception cleanup keeps any earlier successes.
        language_archive_ = language_archive;
        slots_[5] = backend_.create_archive(language_archive_.c_str());
        if (!slots_[5]) slots_[6] = backend_.create_archive("LangEnglish.dta");
        for (std::size_t index = 7; index != slots_.size(); ++index)
            slots_[index] = backend_.create_archive(fixed_names[index]);
    }

    // Preserve ascending slot traversal, missing-slot diagnostics (including
    // reserved 0..4 and intentionally unused fallback), and ignored setter
    // results. Read through +10 BETWEEN the two +0c calls; do not shortcut it.
    template<class Random>
    void configure(std::uint64_t& mask, Random& random) {
        if (!created_) throw std::logic_error("Startup archives not created");
        for (std::size_t index = 0; index != slots_.size(); ++index) {
            auto* archive = slots_[index];
            if (!archive) {
                backend_.missing_archive(index == 5 ? language_archive_ : fixed_names[index]);
                continue;
            }
            mask = (mask << 16) | (random() & 0xffffu);
            const auto key = startup_archive_keys[index];
            backend_.set_archive_key(archive, key ^ mask);
            const auto reply = backend_.archive_key_reply(archive);
            backend_.set_archive_key(archive, reply ^ key);
        }
    }

    void* slot(std::size_t index) const { return slots_.at(index); }

    void close() {
        // Original runs this phase AFTER 3D, sound and graphics close. This
        // owner only releases archives; its caller must preserve that ordering.
        for (auto& archive : slots_) {
            if (archive) {
                backend_.release_archive(archive);
                archive = nullptr; // Callback sees the original slot until return.
            }
        }
        created_ = false;
    }

private:
    inline static constexpr std::array<const char*, 16> fixed_names{{
        "", "", "", "", "", "", "LangEnglish.dta", "Patch.dta", "Models.dta",
        "Missions.dta", "Maps.dta", "Sounds.dta", "Scripts.dta", "Others.dta",
        "Maps_C.dta", "Maps_U.dta"
    }};
    Backend& backend_;
    std::array<void*, 16> slots_{};
    std::string language_archive_;
    bool created_ = false;
};

}
#endif
