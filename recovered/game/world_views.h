#ifndef HD2_GAME_WORLD_VIEWS_H
#define HD2_GAME_WORLD_VIEWS_H

#include <stdexcept>
namespace hd2::frontend { struct SoundWorldView; }
namespace hd2::game {

// Borrow only an actually constructed game owner/scene. A scene without its
// loaded camera is not yet an audio listener or a loaded mission. The owning
// state clears these views before releasing any engine references.
class WorldViews {
public:
    void bind(void* owner, void* scene) {
        if (!owner || !scene || owner_) throw std::logic_error("Invalid game owner publication");
        owner_ = owner;
        scene_ = scene;
    }
    void unbind(void* owner) noexcept {
        if (owner_ != owner) return;
        sound = nullptr;
        scene_ = nullptr;
        owner_ = nullptr;
    }
    void* owner() const { return owner_; }
    void* scene() const { return scene_; }
    frontend::SoundWorldView* sound = nullptr;
private:
    void* owner_ = nullptr;
    void* scene_ = nullptr;
};

} // namespace hd2::game
#endif
