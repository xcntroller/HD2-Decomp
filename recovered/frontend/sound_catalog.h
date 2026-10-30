#ifndef HD2_FRONTEND_SOUND_CATALOG_H
#define HD2_FRONTEND_SOUND_CATALOG_H

#include "io/chunk_file.h"
#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hd2::frontend {

// Native ownership replaces the original COW strings and growing pointer arrays.
// Offsets name fields whose interpretation has not yet been recovered. The
// original variant constructor copies an undefined byte to +20: no value is
// invented for it here. Catalog parsing never acquires sample resources.
struct CatalogSoundVariant {
    std::string name;
    std::uint32_t word04 = 100, word08 = 0;
    float float0c = 20, float10 = 0.5f, float14 = 1, float18 = 0.5f;
    void* sound = nullptr; // +1c, owned reference
    std::optional<std::uint8_t> unknown20;
    std::vector<void*> sounds; // +24..2c, each nonnull item owns a reference
};
struct CatalogSoundEntry {
    std::string name = "#name error";
    std::vector<std::unique_ptr<CatalogSoundVariant>> variants;
    std::uint8_t flag = 0; // +14
};
struct CatalogSoundGroup {
    std::string name;
    std::vector<std::unique_ptr<CatalogSoundEntry>> entries;
};

// 0043A630, 0043A5E0, 00431FD0, 00431F80. Samples release primary first,
// then extra references in reverse; variants and entries also destroy in reverse.
// Backend must perform the real sample virtual release and outlive this owner.
template<class Backend>
void destroy_catalog_variant(std::unique_ptr<CatalogSoundVariant>& variant, Backend& backend) {
    if (!variant) return;
    if (variant->sound) backend.release_catalog_sound(variant->sound);
    for (auto at = variant->sounds.rbegin(); at != variant->sounds.rend(); ++at)
        if (*at) backend.release_catalog_sound(*at);
    variant.reset();
}
template<class Backend>
void clear_catalog_entry(CatalogSoundEntry& entry, Backend& backend) {
    for (auto at = entry.variants.rbegin(); at != entry.variants.rend(); ++at)
        destroy_catalog_variant(*at, backend);
    decltype(entry.variants){}.swap(entry.variants);
}
template<class Backend>
void destroy_catalog_entry(std::unique_ptr<CatalogSoundEntry>& entry, Backend& backend) {
    if (!entry) return;
    clear_catalog_entry(*entry, backend);
    entry.reset();
}
template<class Backend>
void clear_catalog_group(CatalogSoundGroup& group, Backend& backend) {
    for (auto at = group.entries.rbegin(); at != group.entries.rend(); ++at)
        destroy_catalog_entry(*at, backend);
    decltype(group.entries){}.swap(group.entries); // Name deliberately survives.
}

template<class Files>
std::int32_t catalog_chunk_remaining(Files& files, const io::ChunkFile<Files>& file) {
    const auto& state = file.state();
    if (!state.opened || !state.depth) return 0;
    const auto position = files.seek(state.handle, 0, 1);
    if (position == 0xffffffffu) return 0;
    const auto bits = state.offsets[state.depth] - position;
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}

// The original ignores these read results and can consume uninitialized stack
// words/unterminated text. Native guards reject those undefined paths, and short
// headers, instead of supplying fabricated catalog values or stale chunk tags.
template<class Files, class T>
void read_catalog_value(io::ChunkFile<Files>& file, T& value) {
    if (!file.read(&value, sizeof(value)))
        throw std::runtime_error("Incomplete original sound catalog value");
}
template<class Files>
std::uint16_t enter_catalog_chunk(Files& files, io::ChunkFile<Files>& file) {
    std::uint16_t tag = 0;
    if (!file.enter(tag)) throw std::runtime_error("Incomplete original sound catalog header");
    if (catalog_chunk_remaining(files, file) < 0)
        throw std::runtime_error("Backward original sound catalog chunk");
    return tag;
}
template<class Files>
std::string read_catalog_name(Files& files, io::ChunkFile<Files>& file) {
    const auto count = std::min(catalog_chunk_remaining(files, file), 999);
    if (count <= 0) throw std::runtime_error("Undefined original sound catalog name");
    std::array<char, 999> bytes{};
    if (!file.read(bytes.data(), static_cast<std::uint32_t>(count)))
        throw std::runtime_error("Incomplete original sound catalog name");
    const auto end = std::find(bytes.begin(), bytes.begin() + count, '\0');
    if (end == bytes.begin() + count) throw std::runtime_error("Unterminated original sound catalog name");
    return std::string(bytes.begin(), end);
}

// 0043AA50: strict variant tags; 1460 is NOT accepted. Unknown tags return
// false with their chunk still active. Successful fields each leave their child.
template<class Files>
bool read_catalog_variant(CatalogSoundVariant& variant, Files& files, io::ChunkFile<Files>& file) {
    while (catalog_chunk_remaining(files, file) != 0) {
        switch (enter_catalog_chunk(files, file)) {
        case 1400: variant.name = read_catalog_name(files, file); break;
        case 1410: read_catalog_value(file, variant.word04); break;
        case 1420: read_catalog_value(file, variant.word08); break;
        case 1430: read_catalog_value(file, variant.float0c); break;
        case 1440: read_catalog_value(file, variant.float10); break;
        case 1450: read_catalog_value(file, variant.float14); break;
        case 1470: read_catalog_value(file, variant.float18); break;
        default: return false;
        }
        file.leave();
    }
    return true;
}

// 0043A6F0: clear variants, preserving name/flag. Unknown tags fail here too.
template<class Files, class Backend>
bool read_catalog_entry(CatalogSoundEntry& entry, Files& files,
                        io::ChunkFile<Files>& file, Backend& backend) {
    clear_catalog_entry(entry, backend);
    while (catalog_chunk_remaining(files, file) != 0) {
        switch (enter_catalog_chunk(files, file)) {
        case 1210: entry.name = read_catalog_name(files, file); break;
        case 1310:
            file.read(&entry.flag, 1); // Defined prior byte survives refused/short read.
            break;
        case 1300: {
            auto variant = std::make_unique<CatalogSoundVariant>();
            if (!read_catalog_variant(*variant, files, file)) return false;
            entry.variants.push_back(std::move(variant));
            break;
        }
        default: return false;
        }
        file.leave();
    }
    return true;
}

// 00432040: clear entries, preserve group name, SKIP unknown group children.
template<class Files, class Backend>
bool read_catalog_group(CatalogSoundGroup& group, Files& files,
                        io::ChunkFile<Files>& file, Backend& backend) {
    clear_catalog_group(group, backend);
    while (catalog_chunk_remaining(files, file) != 0) {
        switch (enter_catalog_chunk(files, file)) {
        case 1110: group.name = read_catalog_name(files, file); break;
        case 1200: {
            auto entry = std::make_unique<CatalogSoundEntry>();
            if (!read_catalog_entry(*entry, files, file, backend)) return false;
            group.entries.push_back(std::move(entry));
            break;
        }
        default: break;
        }
        file.leave();
    }
    return true;
}

template<class Backend>
class SoundCatalog {
public:
    // 004394D0 and static table00838A8C. Names are constructor data, not
    // replacements for a missing table. A valid root discards these groups.
    explicit SoundCatalog(Backend& backend) : backend_(backend) {
        for (const auto* name : {"Puppet Movement", "Object Manipulation", "Weapon Shooting",
                "Weapon Manipulation", "Bullet Hit", "Falling Hit", "Explosion", "Version",
                "Vehicle Not Car", "Speech", "Interface & Menu", "Action", "Music",
                "Weapon Shooting Dist"}) {
            auto group = std::make_unique<CatalogSoundGroup>();
            group->name = name;
            groups.push_back(std::move(group));
        }
    }
    ~SoundCatalog() { clear(); }
    SoundCatalog(const SoundCatalog&) = delete;
    SoundCatalog& operator=(const SoundCatalog&) = delete;

    // 004326D0: ungated lookup of already-owned data; no implicit catalog load.
    // Reject negative/out-of-range indices. A null in-range group was undefined
    // in the original; native rejects it rather than inventing an empty group.
    CatalogSoundEntry* sound_entry(std::int32_t group, std::int32_t index) const {
        if (group < 0 || static_cast<std::uint32_t>(group) >= groups.size() || index < 0)
            return nullptr;
        const auto& value = groups[static_cast<std::uint32_t>(group)];
        if (!value) throw std::runtime_error("Null original sound catalog group");
        if (static_cast<std::uint32_t>(index) >= value->entries.size()) return nullptr;
        return value->entries[static_cast<std::uint32_t>(index)].get();
    }

    // 004398B0: reverse destruction, reset groups/map, loaded last.
    void clear() {
        for (auto at = groups.rbegin(); at != groups.rend(); ++at) {
            if (*at) clear_catalog_group(**at, backend_);
            at->reset();
        }
        decltype(groups){}.swap(groups);
        mappings.clear();
        loaded = false;
    }

    // 00439080 returns TRUE only on cached early exit; even a successful fresh
    // load returns FALSE. Missing/bad root preserves old data and loaded flag.
    template<class Files>
    bool load(Files& files, std::uint8_t force = 0) {
        if (loaded && !force) return true;
        io::ChunkFile<Files> file(files);
        if (!file.open("Tables\\IngameSounds.def") && !file.open("Tables\\IngameSounds.dat")) {
            backend_.catalog_diagnostic(" @SOUND_TABLE : tablefile not found");
        } else {
            std::uint16_t tag = 0;
            if (!file.enter(tag) || tag != 1000) {
                backend_.catalog_diagnostic(" @SOUND_TABLE : bad tablefile format");
            } else {
                clear();
                bool parsed = true;
                while (catalog_chunk_remaining(files, file) > 0) {
                    switch (enter_catalog_chunk(files, file)) {
                    case 1100: {
                        auto group = std::make_unique<CatalogSoundGroup>();
                        if (!read_catalog_group(*group, files, file, backend_)) {
                            backend_.catalog_diagnostic(" @SOUND_TABLE : chapter not loaded; loading stopped");
                            parsed = false;
                        } else groups.push_back(std::move(group));
                        break;
                    }
                    case 1050:
                        while (catalog_chunk_remaining(files, file) > 0) {
                            std::int32_t key = 0;
                            std::uint32_t value = 0;
                            read_catalog_value(file, key);
                            read_catalog_value(file, value);
                            mappings[key] = value; // Signed order, last duplicate wins.
                        }
                        break;
                    default: break; // Root also skips unknown children.
                    }
                    if (!parsed) break; // Failed child remains active until close.
                    file.leave();
                }
                if (parsed) loaded = true;
            }
        }
        // Includes missing files, bad roots and an ordinary parser false result.
        while (groups.size() < 14) groups.push_back(std::make_unique<CatalogSoundGroup>());
        file.close();
        return false;
    }

    bool loaded = false;
    std::vector<std::unique_ptr<CatalogSoundGroup>> groups;
    std::map<std::int32_t, std::uint32_t> mappings;
private:
    Backend& backend_;
};

}
#endif
