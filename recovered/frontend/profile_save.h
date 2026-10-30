#ifndef HD2_FRONTEND_PROFILE_SAVE_H
#define HD2_FRONTEND_PROFILE_SAVE_H

#include "frontend/profile_selection.h"

namespace hd2::frontend {

// 006B09A0/006A6D80/004256A0. Actual ChunkFile writer; callers deliberately
// ignore individual transfer results, returningfalse only on open failure.
// Native strings/LE bytes replace original COW/raw object storage.
template<class Files>
bool save_profile_payload(const ProfileState& profile, Files& files, bool wide) {
    io::ChunkFile<Files> file(files);
    const auto path = profile.directories.current_directory(wide) + "profile.sav";
    if (!file.open(path.c_str(), 1)) return false;
    const auto text = [&](std::uint16_t tag, const std::string& value) {
        const auto size = std::strlen(value.c_str()) + 1;
        if (size > 0x7fffffffu) throw std::length_error("Original profile string is too large");
        file.write_chunk(tag, value.c_str(), static_cast<std::uint32_t>(size));
    };
    const auto word = [&](std::size_t offset) {
        const auto value = profile.payload.words.at(offset / 4);
        std::array<std::uint8_t, 4> bytes{};
        for (unsigned i = 0; i != 4; ++i) bytes[i] = static_cast<std::uint8_t>(value >> (i * 8));
        file.write(bytes.data(), 4);
    };
    file.begin(0x1edc);
    text(0x1edd, profile.directories.current_name);
    file.begin(0x1eff); word(0x34); file.leave();
    file.begin(0x1f05); for (std::size_t at = 0x74; at <= 0x80; at += 4) word(at); file.leave();
    file.begin(0x1f06); for (std::size_t at = 0x84; at <= 0x90; at += 4) word(at); file.leave();
    text(0x1eea, profile.payload.strings[1]);
    text(0x1eeb, profile.payload.strings[2]);
    file.begin(0x1ef5); for (std::size_t at = 0x0c; at <= 0x2c; at += 8) word(at); file.leave();
    for (std::uint16_t tag = 0x1efa; tag <= 0x1efe; ++tag)
        text(tag, profile.payload.strings[4 + (tag - 0x1efa) * 2]);
    file.begin(0x1ef0);
    file.begin(0x1ec8);
    const auto& movies = profile.payload.movies.records();
    if (movies.size() < 10) throw std::logic_error("Original ten profile movies are missing");
    for (std::size_t i = 10; i != 0; --i) file.write(&movies[i - 1].enabled, 1);
    file.leave(); file.leave(); file.leave();
    file.close();
    return true;
}

// 006B1290: a recognized filename causes persistence even if already enabled.
template<class Files>
void remember_profile_movie(ProfileState& profile, const char* filename, std::uint8_t value,
                             Files& files, bool wide) {
    if (!profile.directories.current_name.empty() && profile.payload.movies.enable(filename, value))
        save_profile_payload(profile, files, wide);
}

} // namespace hd2::frontend
#endif
