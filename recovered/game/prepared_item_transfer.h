#ifndef HD2_GAME_PREPARED_ITEM_TRANSFER_H
#define HD2_GAME_PREPARED_ITEM_TRANSFER_H

#include "game/loading_item_transfer.h"

namespace hd2::game {

//007D7810→007D8B40 no-source placement into a newly constructed Human. Unlike
//mission-file placements, module7E4 may be enabled: native publishes178 and hand
//cooldown, but A8/B0/current-player consumers have no cold constructor producer.
//Do not reuse this for replacing an active Human's inventory or a displaced item.
template<class Frames,class Human,class Items,class Limits>
std::int32_t transfer_prepared_cold_item(Frames& frames,Human& human,Items& items,Limits& limits,
                                        typename Frames::Entry entry,std::int32_t slot,
                                        std::uint32_t game28,bool enabled) {
    if(human.flag90||human.loading_state||human.fielda8)
        throw std::logic_error("Prepared placement requires its actual cold Human without A8/B0 consumers");
    if(slot<0||slot>6)return -2;
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Prepared placement has no actual Human storage");
    if(auto* holder=storage->holder(static_cast<std::size_t>(slot));holder&&enabled&&holder->cooling_down(game28))return -10;
    typename Frames::Entry displaced{};
    const auto result=transfer_loaded_item(frames,items,limits,entry,displaced,human.base(),slot,game28);
    if(displaced)throw std::logic_error("Cold prepared placement unexpectedly displaced an owned entry");
    if(result<1)return result;
    if(enabled) {
        human.activation.flag178=true;
        if(slot==3)storage->holder(3)->field18(game28+0x398u);
        //Native230 notification, B0 refresh and current-player HUD are absent
        //for this cold registered Human; its containing activation runs later.
    }
    items.runtime_item(entry).field14=game28+0x398u; //All slots; unlike007D81A0's post-transfer zero.
    return result;
}
} //namespace hd2::game
#endif
