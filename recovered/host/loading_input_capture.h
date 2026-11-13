#ifndef HD2_HOST_LOADING_INPUT_CAPTURE_H
#define HD2_HOST_LOADING_INPUT_CAPTURE_H

#include "game/serialized_bytes.h"
#include "io/resource_directories.h"
#include "io/data_manifest.h"
#include <map>
#include <set>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace hd2::host {

template<class Files>
std::optional<std::vector<unsigned char>> read_loading_input(Files& files, const char* path) {
    const auto handle = files.open(path, 0);
    if (handle == 0xffffffffu) return std::nullopt;
    struct Close { Files& files; std::uint32_t handle; ~Close() { files.close(handle); } } close{files, handle};
    const auto size = files.seek(handle, 0, 2);
    if (size == 0xffffffffu || size > 128u * 1024u * 1024u)
        throw std::runtime_error("Invalid loading-input capture size");
    if (files.seek(handle, 0, 0) == 0xffffffffu)
        throw std::runtime_error("Cannot seek loading-input capture resource");
    std::vector<unsigned char> bytes(size);
    if (size && files.read(handle, bytes.data(), size) != size)
        throw std::runtime_error("Short loading-input capture read");
    return bytes;
}

// Diagnostic dependency census, following006CC760's4000/AE20 containers.
// This does not construct actors or claim that a candidate file opens in LS3DF.
// Keep model bindings across Scene2/actors/sounds, in the original document order.
class LoadingVehicleDependencies {
public:
    void read(const std::vector<unsigned char>& bytes) {
        game::SerializedBytes document(bytes);
        auto root = game::next_mission_chunk(document);
        if (!root || root->id != 0x4c53) throw std::runtime_error("Invalid capture scene root");
        while (auto group = game::next_mission_chunk(root->bytes)) {
            if (group->id == 0x4000) {
                while (auto object = game::next_mission_chunk(group->bytes)) {
                    if (object->id != 0x4010) continue;
                    std::string name;
                    while (auto field = game::next_mission_chunk(object->bytes)) {
                        if (field->id == 0x10) name = field->bytes.text();
                        if (field->id == 0x2012) models_[name] = field->bytes.text();
                    }
                }
            } else if (group->id == 0xae20) {
                while (auto actor = game::next_mission_chunk(group->bytes)) {
                    if (actor->id != 0xae21) continue;
                    std::string name;
                    while (auto field = game::next_mission_chunk(actor->bytes)) {
                        if (field->id == 0xae23) name = field->bytes.text();
                        if (field->id == 0xae22 && field->bytes.word() == 4) {
                            const auto found = models_.find(name);
                            if (found == models_.end() || found->second.empty())
                                throw std::runtime_error("Capture cannot resolve vehicle model: " + name);
                            //006CD940 strips the final extension before cache1.
                            const auto& model = found->second;
                            const auto key = model.substr(0, model.rfind('.'));
                            validate_manifest_text(name); validate_manifest_text(key);
                            vehicles_.emplace(name, key);
                        }
                    }
                }
            }
        }
    }
    const std::map<std::string, std::string>& vehicles() const { return vehicles_; }
    static void validate_manifest_text(const std::string& text) {
        if (text.find_first_of("\t\r\n") != std::string::npos)
            throw std::runtime_error("Invalid capture manifest text");
    }
private:
    std::map<std::string, std::string> models_, vehicles_;
};

// Developer-only capture of the actual rw_data resolution. Neither the native
// extractor nor the agent executes original DLLs. The user invokes this through
// the existing isolated launcher; files are read with the same registered
// loose/language/Patch/base precedence as the real application.
template<class Files, class Output>
void capture_loading_inputs(Files& files, Output& output,
                            const io::ResourceDirectoryList& model_directories) {
    std::string manifest = "resource\tbytes\tfile\n";
    std::string provenance="resource\toutcome\tbytes\tsha256\torigin\n";
    const auto capture = [&](const std::string& path, const std::string& filename = {}) {
        const auto bytes = read_loading_input(files, path.c_str());
        std::string name = filename.empty() ? path : filename;
        for (auto& c : name) if (c == '\\' || c == '/') c = '_';
        if (bytes) {
            provenance+=io::data_manifest_text(path)+"\treturned-by-resolver\t"+std::to_string(bytes->size())+"\t"+io::resource_sha256(bytes->data(),bytes->size())+"\tunavailable: rw_data selected origin\n";
            output.write(name, bytes->data(), bytes->size());
            manifest += path + "\t" + std::to_string(bytes->size()) + "\t" + name + "\n";
            std::printf("Loading input captured: %s bytes=%zu\n", path.c_str(), bytes->size());
        } else {
            provenance+=io::data_manifest_text(path)+"\tabsent-at-capture\tunknown\tunknown\tunavailable: rw_data selected origin\n";
            manifest += path + "\tmissing\t-\n";
            std::printf("Loading input absent: %s\n", path.c_str());
        }
        return bytes;
    };
    capture("Tables\\anim.sav");
    capture("Tables\\items.sav");
    capture("GameData\\Gamedata00.gdt");
    std::set<std::string> vehicle_keys;
    std::string dependencies = "mission\tactor\tmodel-key\n";
    for (const auto* mission : {"Tutorial", "ALPS1"}) {
        LoadingVehicleDependencies census;
        for (const auto* member : {"scene.I3D", "scene.4ds", "scene2.bin", "actors.bin",
                "sounds.bin", "check2.bin", "check.bin", "tree.klz", "effects.bin", "loader.4ds",
                "car_table.dat", "items.dat", "tracks.dat", "Scripts.dta", "mpscripts.dta"}) {
            const auto bytes = capture(std::string("missions\\") + mission + "\\" + member);
            if (bytes && (std::string_view(member) == "scene2.bin" ||
                          std::string_view(member) == "actors.bin" ||
                          std::string_view(member) == "sounds.bin")) census.read(*bytes);
        }
        for (const auto& vehicle : census.vehicles()) {
            dependencies += std::string(mission) + "\t" + vehicle.first + "\t" + vehicle.second + "\n";
            vehicle_keys.insert(vehicle.second);
        }
    }
    output.write("vehicle-dependencies.tsv", dependencies.data(), dependencies.size());
    std::string directories = "index\tdirectory\n";
    std::size_t directory_index = 0, candidate_index = 0;
    for (const auto& directory : model_directories.paths()) {
        LoadingVehicleDependencies::validate_manifest_text(directory);
        directories += std::to_string(directory_index++) + "\t" + directory + "\n";
        for (const auto& key : vehicle_keys)
            capture(directory + key + ".4ds", "vehicle-model-" + std::to_string(candidate_index++) + ".4ds");
    }
    if (!vehicle_keys.empty() && model_directories.paths().empty())
        throw std::runtime_error("Capture has no registered model directories");
    output.write("model-directories.tsv", directories.data(), directories.size());
    // This completion marker is written last, only after every read/output has
    // completed. A failed/partial capture is never presented as a complete one.
    output.write("resource-provenance.tsv",provenance.data(),provenance.size());
    output.write("manifest.tsv", manifest.data(), manifest.size());
}

inline void validate_loading_capture_name(std::wstring_view name) {
    constexpr std::wstring_view prefix = L"loading-inputs-";
    if (name.size() != prefix.size() + 32 || name.substr(0, prefix.size()) != prefix)
        throw std::invalid_argument("Invalid loading-input capture name");
    for (const auto c : name.substr(prefix.size()))
        if (!((c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f')))
            throw std::invalid_argument("Invalid loading-input capture name");
}

} // namespace hd2::host
#endif
