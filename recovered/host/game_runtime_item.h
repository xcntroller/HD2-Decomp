#ifndef HD2_HOST_GAME_RUNTIME_ITEM_H
#define HD2_HOST_GAME_RUNTIME_ITEM_H

#include "host/game_actor_services.h"
#include "game/runtime_item.h"
#include "game/runtime_item_services.h"

namespace hd2::host {

using GameRuntimeItemServices=game::RuntimeItemServices<GameActorServices>;

//The common actor entry registry owns the actual reconstructed item instance.
//Services supplies its real model/cache and item-module consumers. Returning an
//entry does not publish a placed Item actor or report mission activation.
template<class Services>
class GameRuntimeItemEntry final : public GameActorServices::Item {
public:
    using Native=game::RuntimeItem<GameActorServices,Services>;
    GameRuntimeItemEntry(GameActorServices& backend,Services& services,
                         const frontend::PreMissionItemCatalogue& catalogue,std::int32_t id)
        : item_(Native::create(backend,services,catalogue,id)) {}
    float scalar() const override {return item_->scalar();}
    std::uint32_t definition_identifier() const override {return item_->definition().identifier;}
    std::uint32_t instance_family() const override {return static_cast<std::uint32_t>(item_->family());}
    long double volume() const override {return item_->volume();}
    Native& item() {return *item_;}
    const Native& item() const {return *item_;}
private:
    std::unique_ptr<Native> item_;
};

} //namespace hd2::host
#endif
