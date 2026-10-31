#ifndef HD2_FRONTEND_OPTIONS_SAVE_H
#define HD2_FRONTEND_OPTIONS_SAVE_H
#include "frontend/profile_settings.h"

namespace hd2::frontend {
// 006B2950,006A8770,006B3530: three sequential top-level chunks. Preserve
// dirty bytes and padding as saved BEFORE006A86C0 installs pending bindings.
template <class Files>
bool save_profile_options(const ProfileOptions &options, const ProfileDirectories &profiles, Files &files,
                          bool wide) {
    io::ChunkFile<Files> file(files);
    const auto path = profiles.current_directory(wide) + "Options.bin";
    if (!file.open(path.c_str(), 1))
        return false;
    const auto word = [&](std::uint32_t value) {
        std::array<std::uint8_t, 4> bytes{};
        for (unsigned i = 0; i < 4; ++i)
            bytes[i] = static_cast<std::uint8_t>(value >> (8 * i));
        file.write(bytes.data(), 4);
    };
    file.begin(0x25a0);
    word(0x141);
    file.leave();
    file.begin(0x25a6);
    file.begin(0x22b8);
    word(0x42);
    file.leave();
    file.begin(0x22b9);
    for (const auto &binding : options.bindings.settings) {
        word(static_cast<std::uint32_t>(binding.index));
        word(binding.device_kind | (std::uint32_t{binding.code} << 16));
        word(binding.direction);
        word(binding.mode);
        word(binding.group);
        word(binding.caption);
        word(binding.dirty | (std::uint32_t{binding.padding[0]} << 8) |
             (std::uint32_t{binding.padding[1]} << 16) | (std::uint32_t{binding.padding[2]} << 24));
    }
    file.leave();
    file.leave();
    file.begin(0x25a5);
    file.begin(0x1201);
    file.write(options.settings.bytes.data(), 0x3c);
    file.leave();
    file.leave();
    file.close();
    return true;
}
} // namespace hd2::frontend
#endif
