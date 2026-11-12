#ifndef HD2_GAME_ITEM_ICON_CACHE_H
#define HD2_GAME_ITEM_ICON_CACHE_H
#include "frontend/pre_mission_data.h"
#include "frontend/cleanup.h"
#include <array>
#include <optional>
#include <utility>
namespace hd2::game {
//007DF650/native definition28. One cache in the SAME Items model-services
//owner; catalogue and graphics outlive it. Failed optional loads retry later.
template<class Frames>class ItemIconCache {
public:
    using Definition=frontend::PreMissionItem;using Catalogue=frontend::PreMissionItemCatalogue;
    using Texture=decltype(std::declval<Frames&>().load_item_icon_texture(std::declval<const std::string&>()));
    ItemIconCache(Frames& frames,const Catalogue& catalogue):frames_(frames),catalogue_(catalogue){}
    auto get(const Definition& definition){
        if(definition.identifier>=Catalogue::capacity||catalogue_.item(static_cast<std::int32_t>(definition.identifier))!=&definition)
            throw std::logic_error("Item icon definition is outside SAME selected catalogue");
        auto& owner=icons_[definition.identifier];
        if(owner&&*owner)return owner->get();
        if(definition.resources[1].empty())return typename Frames::Frame{};
        const auto path=definition.resources[1]+".bmp";
        if(path.size()>=128)throw std::length_error("Item icon name exceeds original128-byte buffer");
        const auto first=std::uint32_t(frames_.hud_driver_state(0x15)!=0),second=std::uint32_t(frames_.hud_driver_state(0x19)!=0);
        frontend::CleanupErrors errors;
        try{
            frames_.hud_driver_state(0x15,0);frames_.hud_driver_state(0x19,0);
            auto texture=frames_.load_item_icon_texture(path);
            if(texture)owner.emplace(std::move(texture));
        }catch(...){errors=frontend::CleanupErrors(std::current_exception());}
        errors.attempt([&]{frames_.hud_driver_state(0x15,first);});errors.attempt([&]{frames_.hud_driver_state(0x19,second);});errors.rethrow();
        return owner?owner->get():typename Frames::Frame{};
    }
    void clear_definition(std::size_t index)noexcept{icons_[index].reset();}
private:
    Frames& frames_;const Catalogue& catalogue_;std::array<std::optional<Texture>,Catalogue::capacity> icons_{};
};
}
#endif
