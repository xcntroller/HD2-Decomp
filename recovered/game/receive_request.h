#ifndef HD2_GAME_RECEIVE_REQUEST_H
#define HD2_GAME_RECEIVE_REQUEST_H

#include <memory>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//006C0EF0 kind7: install type1 before invoking virtual+04 with the actual mission,
// not the game marker. A failed/incomplete activation releases the entire owner
// before constructing the error UI. Factory supplies the existing Main-return
// dialog; it never stands in for an initialized game. Previous menu is already
// shut down/released by the enclosing descriptor transition.
template<class Controller, class GameState, class Descriptor, class FailureFactory>
bool receive_request(Controller& controller, std::unique_ptr<GameState> game,
                     const Descriptor& game_marker, FailureFactory failure_factory) {
    if (controller.active() || !game) throw std::logic_error("Game receiver needs an empty controller");
    auto* receiving = game.get();
    const auto* mission = receiving->mission_descriptor();
    if (!mission) throw std::logic_error("Game receiver has no owned mission");
    controller.install(std::move(game));
    if (controller.active()->initialize(mission)) return true;
    const auto failure = receiving->failure();
    auto failed = controller.detach();
    failed->shutdown();
    failed.reset();
    auto fallback = failure_factory(failure);
    if (!fallback) throw std::runtime_error("Game failure UI allocation failed");
    controller.install(std::move(fallback));
    if (!controller.active()->initialize(&game_marker))
        throw std::runtime_error("Game failure UI initialization failed");
    return false;
}

} // namespace hd2::game
#endif
