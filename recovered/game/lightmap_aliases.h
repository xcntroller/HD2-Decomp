#ifndef HD2_GAME_LIGHTMAP_ALIASES_H
#define HD2_GAME_LIGHTMAP_ALIASES_H

#include "game/serialized_bytes.h"
#include <algorithm>

namespace hd2::game {

//0048CD70 /0048D100 /0048D1D0 /0041D5E0. B810 owns case-sensitive
//light-map name groups. It has no dependency on the script VM or scene lookup.
class LightmapAliases {
public:
    struct Group {std::string name;std::vector<std::string> aliases;};
    const std::vector<Group>& groups() const {return groups_;}
    const Group* find(const std::string& name) const {
        for(auto i=groups_.rbegin();i!=groups_.rend();++i)if(i->name==name)return &*i;
        return nullptr;
    }
    void clear() noexcept {
        while(!groups_.empty())groups_.pop_back();
    }
    void read(SerializedBytes& bytes) {
        clear(); //Primary groups are replaced even by an empty B810.
        while(auto chunk=next_mission_chunk(bytes)) {
            if(chunk->id!=0x100&&chunk->id!=0x110)
                throw std::runtime_error("Unsupported light-map alias field");
            auto name=chunk->bytes.section(std::min<std::size_t>(chunk->bytes.remaining(),999)).text();
            if(chunk->id==0x100) {
                if(!name.empty()&&!find(name))groups_.push_back({std::move(name),{}});
            } else {
                //The original selects the physical last group, including when
                //a preceding duplicate/empty primary name did not append one.
                if(groups_.empty())throw std::runtime_error("Light-map alias has no primary group");
                auto& aliases=groups_.back().aliases;
                if(std::find(aliases.rbegin(),aliases.rend(),name)==aliases.rend())aliases.push_back(std::move(name));
            }
        }
    }
private:
    std::vector<Group> groups_;
};

} //namespace hd2::game
#endif
