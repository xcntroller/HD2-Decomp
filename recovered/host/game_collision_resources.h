#ifndef HD2_HOST_GAME_COLLISION_RESOURCES_H
#define HD2_HOST_GAME_COLLISION_RESOURCES_H

#include "host/game_scene.h"
#include "game/collision_tree.h"

namespace hd2::host {

//Concrete006EE570 frame/mesh endpoints. The containing world must keep this
//adapter and its scene alive until the native collision tree is destroyed, and
//discard both before resetting/replacing the loaded scene. Every mesh/vertex
//view is borrowed, matching the original loader's retained vertex addresses.
class GameCollisionResources {
public:
    using Frame = EngineFrame*;
    GameCollisionResources(GameScene& scene, HMODULE module) : scene_(scene), module_(module) {}
    void clear_views() noexcept {views_.clear();} //Only after the tree's borrowed vertex/frame consumers retire.
    Frame collision_frame(const std::string& name) { return scene_.resolve_frame_path(name); }
    bool collision_is_visual(Frame frame) const {
        if (!frame) return false;
        std::uint16_t kind;
        std::memcpy(&kind, reinterpret_cast<const unsigned char*>(frame) + 0xf8, sizeof(kind));
        return kind == 1;
    }
    void enable_collision(Frame frame) {
        if (!collision_is_visual(frame)) throw std::logic_error("Static collision requires a visual frame");
        //006EE734..006EE746, visual+7C(frame,1). Normal visual implementation
        //10036350 bakes mesh vertices/normals into world coordinates; mirror's
        //100570A0 is a genuine no-op. The engine supplies both actual bodies.
        using Enable = void (WINAPI *)(Frame, std::uint8_t);
        checked_engine_method<Enable>(frame, module_, 0x7c)(frame, 1);
        views_.erase(frame);
        scene_.rethrow_callback_failure();
    }
    bool collision_mesh_frame(Frame frame) {
        if (!collision_is_visual(frame)) return false;
        using Type = std::uint32_t (WINAPI *)(Frame);
        if (checked_engine_method<Type>(frame, module_, 4)(frame) == 0x5252494du) return false; //MIRR.
        auto* mesh = pointer_field(frame, 0x1e0);
        if (!mesh) return false;
        if (views_.find(frame) == views_.end()) {
            //006EEBB9..006EEBC6: mesh+04(0) selects LOD0; its+04(1) borrows read-only
            //vertices.10036350 independently confirms LOD+54 is vertex count
            //and the position/normal/UV record stride is32 bytes.
            using Lod = Frame (WINAPI *)(Frame, std::uint32_t);
            auto* lod = checked_engine_method<Lod>(mesh, module_, 4)(mesh, 0);
            if (!lod) throw std::runtime_error("Collision mesh has no LOD0");
            std::uint32_t count;
            std::memcpy(&count, reinterpret_cast<const unsigned char*>(lod) + 0x54, sizeof(count));
            using Vertices = const unsigned char* (WINAPI *)(Frame, std::uint32_t);
            auto* vertices = checked_engine_method<Vertices>(lod, module_, 4)(lod, 1);
            if (count && !vertices) throw std::runtime_error("Collision LOD0 has no vertex storage");
            views_.emplace(frame, MeshView{vertices,count});
        }
        return true;
    }
    game::CollisionPoint collision_vertex(Frame frame, std::uint32_t index) {
        const auto view = views_.find(frame);
        if (view == views_.end()) throw std::logic_error("Collision mesh view was not resolved");
        if (index >= view->second.count) throw std::runtime_error("Collision vertex index exceeds actual mesh");
        game::CollisionPoint result;
        std::memcpy(result.data(), view->second.vertices + std::size_t(index) * 32u, sizeof(result));
        return result;
    }
private:
    static Frame pointer_field(Frame frame, std::size_t offset) {
        Frame result;
        std::memcpy(&result, reinterpret_cast<const unsigned char*>(frame) + offset, sizeof(result));
        return result;
    }
    struct MeshView { const unsigned char* vertices; std::uint32_t count; };
    GameScene& scene_;
    HMODULE module_;
    std::map<Frame, MeshView> views_;
};

} // namespace hd2::host
#endif
