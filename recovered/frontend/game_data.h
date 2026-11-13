#ifndef HD2_FRONTEND_GAME_DATA_H
#define HD2_FRONTEND_GAME_DATA_H

#include "io/chunk_file.h"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hd2::frontend {

// Native representation of original kinds1..7, objective12 and inline MP mission13. Offset labels refer to each
// original derived layout; overlapping fields belong to different kinds.
struct GameDataNode {
    explicit GameDataNode(std::uint32_t type) : kind(type), word10(type == 4 || type == 12 ? 4u : type == 6 ? 0xffffffffu : 0u) {
        if(type==13)network_mission.emplace(); //006B5620, actual inline mission constructor.
    }
    ~GameDataNode() {
        while (!related.empty()) related.pop_back();
        //006B7F50 only resets the related end cursor; its allocation survives
        //the MP table/children/string tail and retires in the base mission.
        if(network_mission)network_mission->table74.clear();
        while (!children.empty()) children.pop_back();
        if(network_mission){ //006B6100 inline MP tail: storage, name6C, metadata68, then base strings.
            decltype(network_mission->table74)().swap(network_mission->table74);
            network_mission->name6c.reset();network_mission->metadata68.reset();
        }
    }
    std::uint32_t kind, word8 = 0, word10, word20 = 0xffffffffu;
    std::uint8_t flag14 = 0, flag15 = 0, flag18 = 0, flag2c = 0;
    std::string text10, text14, text24, text28;
    std::array<std::uint32_t, 4> words40{{0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu}};
    std::vector<std::array<unsigned char, 16>> entries;
    std::vector<std::uint32_t> items;
    std::vector<std::unique_ptr<GameDataNode>> children, related;
    struct NetworkObjectiveFields {std::uint32_t side18,field1c,field20,score24,selector28,field2c;};
    //Only006B8700's actual map-row producer defines these native kind12 words.
    //Offline flag18/text24/text28 have different layouts and cannot substitute.
    std::optional<NetworkObjectiveFields> network_objective;
    struct NetworkMissionFields {
        //Mission50/54/58/5C are initializedFFFFFFFF by006B5620;60/64 have
        //no cold producer. These are GameData64/68/6C/70/74/78 respectively.
        std::array<std::optional<std::uint32_t>,6> words50{{0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu,std::nullopt,std::nullopt}};
        struct TableRow {std::array<std::uint32_t,3> words;std::optional<std::uint32_t> fourth;};
        //006B8700 defines three stack words;006BA0F0 also copies an unwritten
        //fourth DWORD into its16-byte element. Do not invent a value for it.
        std::vector<TableRow> table74;
        std::shared_ptr<const std::string> name6c,metadata68;
    };
    std::optional<NetworkMissionFields> network_mission;
};

// Original application data manager's loaded roots, independent of its other
// settings/runtime members. 006B4AE4 initializes format +EC to two and +E8 to
// zero. Loading appends; it does not silently reset previously loaded roots.
class GameData {
public:
    GameData()=default;
    GameData(GameData&&)=default;
    GameData& operator=(GameData&& other)noexcept{
        if(this!=&other){
            while(!roots_.empty()){roots_.pop_back();}multiplayer_mission_.reset();
            roots_=std::move(other.roots_);multiplayer_mission_=std::move(other.multiplayer_mission_);
            format=other.format;field_e4=other.field_e4;field_e8=other.field_e8;rejected_root_=other.rejected_root_;
        }
        return *this;
    }
    ~GameData(){while(!roots_.empty())roots_.pop_back();} //006B6100 roots before inline mission; vector allocation retires after inline member.
    void construct_multiplayer_mission(){
        if(multiplayer_mission_)throw std::logic_error("Application inline MP mission was already constructed");
        multiplayer_mission_=std::make_unique<GameDataNode>(13);
    }
    GameDataNode& multiplayer_mission(){if(!multiplayer_mission_)throw std::logic_error("GameData has no application inline MP mission owner");return *multiplayer_mission_;}
    const GameDataNode& multiplayer_mission()const{if(!multiplayer_mission_)throw std::logic_error("GameData has no application inline MP mission owner");return *multiplayer_mission_;}
    std::uint32_t format = 2, field_e4 = 0xffffffffu, field_e8 = 0;
    const auto& roots() const { return roots_; }
    std::optional<std::uint16_t> rejected_root() const { return rejected_root_; }
    //006B8EC0 through006B8570/006B8480/006B85D0. The native root bounds
    //check PRECEDES the standalone ordinal. Ordinary selection uses the actual
    //container cursors, counting only kind2 campaigns and kind3 missions.
    std::uint32_t selected_mission_ordinal(std::uint32_t root,bool standalone)const{
        if(root>=roots_.size())return 0xffffffffu;
        if(standalone)return field_e4;
        const auto& container=*roots_[root];
        if(container.word20>=0x80000000u||container.word20>=container.children.size())return 0xffffffffu;
        const auto* campaign=container.children[container.word20].get();
        if(!campaign||campaign->kind!=2||campaign->word20>=0x80000000u||campaign->word20>=campaign->children.size())return 0xffffffffu;
        const auto* selected=campaign->children[campaign->word20].get();
        if(!selected||selected->kind!=3)return 0xffffffffu;
        std::uint32_t ordinal=0;
        for(const auto& entry:container.children){
            if(entry->kind!=2)continue;
            for(const auto& mission:entry->children){
                if(mission->kind!=3)continue;
                if(mission.get()==selected)return ordinal;
                ++ordinal;
            }
        }
        return 0xffffffffu;
    }
    // 006B9030 and read-only lookup006B8D20: count/flatten only kind2
    // campaigns and kind3 missions, preserving order among mixed children.
    std::int32_t mission_count(std::uint32_t root) const {
        if (root >= roots_.size()) return -1;
        std::uint32_t count = 0;
        for (const auto& campaign : roots_[root]->children) if (campaign->kind == 2)
            for (const auto& mission : campaign->children) if (mission->kind == 3) {
                if (count == 0x7fffffffu) throw std::length_error("GameData mission count overflow");
                ++count;
            }
        return static_cast<std::int32_t>(count);
    }
    std::uint32_t mission_caption(std::uint32_t root, std::int32_t ordinal) const {
        if (root < roots_.size() && ordinal >= 0)
            for (const auto& campaign : roots_[root]->children) if (campaign->kind == 2)
                for (const auto& mission : campaign->children)
                    if (mission->kind == 3 && ordinal-- == 0) return mission->word8;
        return 0xffffffffu;
    }
    template<class Files>
    bool load(Files& files) {
        rejected_root_.reset(); // Native diagnostic only; no new loader policy.
        io::ChunkFile<Files> file(files); // Same object reused across numbered files.
        for (std::int32_t index = 0;;) {
            char path[256];
            const auto size = std::snprintf(path, sizeof(path), "GameData\\Gamedata%02d.gdt", index);
            if (size < 0 || size >= static_cast<int>(sizeof(path))) throw std::length_error("GameData path overflow");
            if (!file.open(path)) return !roots_.empty();
            auto root = std::make_unique<GameDataNode>(1);
            auto tag = enter(file);
            bool chunked = false;
            if (tag == 3) {
                chunked = word(file) == format;
                file.leave();
                tag = enter(file);
            }
            if (tag == 1) {
                if (chunked) read_chunked(files, file, *root);
                else {
                    read_flat(files, file, *root);
                    read_flat_children(files, file, *root);
                }
                roots_.push_back(std::move(root));
                file.leave();
                file.close();
            } else rejected_root_ = tag;
            // Original leaves an invalid root/file open; next open rejects it
            // before calling the backend and returns whether old roots exist.
            if (index == std::numeric_limits<std::int32_t>::max()) throw std::length_error("GameData file index overflow");
            ++index;
        }
    }
private:
    template<class Files>
    static std::uint16_t enter(io::ChunkFile<Files>& file) {
        std::uint16_t id = 0;
        if (!file.enter(id)) throw std::runtime_error("Truncated GameData chunk header");
        return id;
    }
    template<class Files>
    static std::int32_t remaining(Files& files, const io::ChunkFile<Files>& file) {
        const auto& state = file.state();
        if (!state.opened || !state.depth) return 0;
        const auto pos = files.seek(state.handle, 0, 1);
        if (pos == 0xffffffffu) return 0;
        const auto bits = state.offsets[state.depth] - pos;
        std::int32_t count;
        std::memcpy(&count, &bits, 4);
        return count;
    }
    template<std::size_t Size, class Files>
    static std::array<unsigned char, Size> bytes(io::ChunkFile<Files>& file) {
        std::array<unsigned char, Size> result{};
        if (!file.read(result.data(), Size)) throw std::runtime_error("Truncated GameData field");
        return result;
    }
    template<class Files>
    static std::uint32_t word(io::ChunkFile<Files>& file) {
        const auto b = bytes<4>(file);
        return std::uint32_t{b[0]} | (std::uint32_t{b[1]} << 8) |
               (std::uint32_t{b[2]} << 16) | (std::uint32_t{b[3]} << 24);
    }
    template<class Files>
    static std::string text(Files& files, io::ChunkFile<Files>& file) {
        const auto count = std::min<std::int32_t>(999, remaining(files, file));
        if (count <= 0) throw std::runtime_error("Invalid GameData string size");
        std::array<char, 999> value{};
        if (!file.read(value.data(), static_cast<std::uint32_t>(count))) throw std::runtime_error("Truncated GameData string");
        const auto end = std::find(value.begin(), value.begin() + count, '\0');
        if (end == value.begin() + count) throw std::runtime_error("Unterminated GameData string");
        return std::string(value.begin(), end);
    }
    template<class Files>
    static void read_chunked(Files& files, io::ChunkFile<Files>& file, GameDataNode& node) {
        // Original kind7 +8 (006B9C90) returns true without consuming payload.
        if (node.kind == 7) return;
        while (remaining(files, file) != 0) {
            const auto tag = enter(file);
            switch (node.kind) {
            case 1:
                if (tag == 0x47) node.word8 = word(file);
                else if (tag == 5) read_children(files, file, node);
                break;
            case 2:
                if (tag == 0x3d) node.word8 = word(file);
                else if (tag == 0x3e) node.text24 = text(files, file);
                else if (tag == 5) read_children(files, file, node);
                break;
            case 3:
                if (tag == 0x33) node.word8 = word(file);
                else if (tag == 0x34) node.flag2c = bytes<1>(file)[0];
                else if (tag == 0x35) node.text28 = text(files, file);
                else if (tag == 0x36) node.text24 = text(files, file);
                else if (tag == 0x37) for (auto& value : node.words40) value = word(file);
                else if (tag == 5) read_children(files, file, node);
                else if (tag == 0x28) {
                    auto related = std::make_unique<GameDataNode>(4);
                    read_chunked(files, file, *related);
                    file.leave(); // Original appends related AFTER leaving its chunk.
                    node.related.push_back(std::move(related));
                    continue;
                }
                break;
            case 4:
                if (tag == 0x29) node.word8 = word(file);
                else if (tag == 0x2a) node.word10 = word(file);
                else if (tag == 0x2b) node.flag14 = bytes<1>(file)[0];
                break;
            case 5:
                if (tag == 0x0b) node.text10 = text(files, file);
                else if (tag == 0x0c) node.text14 = text(files, file);
                else if (tag == 0x0d) node.flag18 = bytes<1>(file)[0];
                break;
            case 6:
                if (tag == 0x15) node.word10 = word(file);
                else if (tag == 0x16) node.flag14 = bytes<1>(file)[0];
                else if (tag == 0x17) node.flag15 = bytes<1>(file)[0];
                else if (tag == 0x18) node.entries.push_back(bytes<16>(file));
                else if (tag == 0x19) node.items.push_back(word(file));
                break;
            default: throw std::logic_error("Invalid native GameData node kind");
            }
            file.leave(); // All original GameData readers skip unknown fields.
        }
    }
    template<class Files>
    static void read_children(Files& files, io::ChunkFile<Files>& file, GameDataNode& parent) {
        while (remaining(files, file) != 0) {
            const auto tag = enter(file);
            const std::uint32_t kind = tag == 10 ? 5 : tag == 20 ? 6 : tag == 30 ? 7 : tag == 50 ? 3 : tag == 60 ? 2 : 0;
            if (kind) {
                auto child = std::make_unique<GameDataNode>(kind);
                read_chunked(files, file, *child);
                parent.children.push_back(std::move(child));
            }
            file.leave();
        }
    }
    template<class Files>
    static void flat_text(Files& files, io::ChunkFile<Files>& file, std::string& output) {
        if (enter(file) == 1000) output = text(files, file);
        file.leave();
    }
    template<class Files>
    static void read_flat(Files& files, io::ChunkFile<Files>& file, GameDataNode& node) {
        switch (node.kind) {
        case 1:
            for (unsigned i = 0; i < 3; ++i) word(file); // Discard three old words.
            node.word8 = word(file);
            break;
        case 2:
            node.word8 = word(file); flat_text(files, file, node.text24); break;
        case 3:
            node.word8 = word(file);
            for (unsigned i = 0; i < 4; ++i) word(file);
            node.flag2c = bytes<1>(file)[0];
            flat_text(files, file, node.text28);
            flat_text(files, file, node.text24);
            break;
        case 4:
            node.word8 = word(file); node.word10 = word(file); break;
        case 5:
            node.flag18 = bytes<1>(file)[0];
            flat_text(files, file, node.text10); flat_text(files, file, node.text14);
            break;
        case 6:
            node.word10 = word(file);
            node.flag15 = bytes<1>(file)[0]; node.flag14 = bytes<1>(file)[0];
            while (remaining(files, file) != 0) {
                const auto tag = enter(file);
                if (tag == 10) node.entries.push_back(bytes<16>(file));
                else if (tag == 11) node.items.push_back(word(file));
                file.leave();
            }
            break;
        case 7: break; // Original 006B9C80 genuinely consumes nothing.
        default: throw std::logic_error("Invalid native GameData node kind");
        }
    }
    template<class Files>
    static void read_flat_children(Files& files, io::ChunkFile<Files>& file, GameDataNode& parent) {
        while (remaining(files, file) != 0) {
            const auto tag = enter(file);
            const bool allowed = parent.kind == 1 ? tag == 2 || tag == 5 || tag == 6 :
                parent.kind == 2 ? tag == 3 || tag == 5 || tag == 6 :
                parent.kind == 3 && (tag == 4 || tag == 5 || tag == 6 || tag == 7);
            if (allowed) {
                auto child = std::make_unique<GameDataNode>(tag);
                read_flat(files, file, *child);
                if (tag == 2 || tag == 3) read_flat_children(files, file, *child);
                if (tag == 4) parent.related.push_back(std::move(child));
                else parent.children.push_back(std::move(child));
            }
            file.leave();
        }
    }
    std::vector<std::unique_ptr<GameDataNode>> roots_;
    //Only application+04 constructs this native inline+14 lifetime. The existing
    //preparation-only loaded graph does not allocate another MP mission owner.
    std::unique_ptr<GameDataNode> multiplayer_mission_;
    std::optional<std::uint16_t> rejected_root_;
};

}
#endif
