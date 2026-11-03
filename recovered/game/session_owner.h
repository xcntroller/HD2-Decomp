#ifndef HD2_GAME_SESSION_OWNER_H
#define HD2_GAME_SESSION_OWNER_H

#include "frontend/pre_mission_handoff.h"
#include "game/mission_tables.h"
#include "game/overlay_owner.h"
#include <array>
#include <memory>
#include <stdexcept>
#include <utility>

namespace hd2::game {

// The kind7 marker is not the initializer's mission argument.006B9190 returns
// the real kind3 mission (or the synthetic standalone copy), with child/objective
// descriptors consumed after loading. Keep that tree as well as the copied live
// soldier/equipment payload; neither may dangle after releasing preparation.
struct MissionRequest {
    frontend::PreMissionGameRequest preparation;
    std::unique_ptr<frontend::GameDataNode> mission;
};
inline MissionRequest copy_mission_request(const frontend::PreMissionGameRequest& request,
                                           const frontend::GameDataNode& mission) {
    if (mission.kind != 3 || mission.text24.empty() || mission.text24 != request.mission_resource)
        throw std::logic_error("Game receiver requires its actual selected mission");
    if (request.squad.empty()) throw std::logic_error("Game receiver requires its prepared squad");
    return {request, frontend::copy_pre_mission_node(mission)};
}

// Native construction/lifetime composition of00698510 and the unloaded
// branch of00698830/00699270. Backend supplies real HUD/text factories and a
// scene owner; it has no fallback implementations. The host now supplies their
// actual unloaded construction. Live shutdown00698F80 and successful activation
//006989B0 remain separate recovery work.
// This class cannot activate/render a game or assert successful level loading.
// Kind7 now installs this owner through GameApplicationState; desktop validation
// of that new construction/error-return path is pending.
//
// Required Backend: FrameOwner, Hud, Text, Scene; make_hud/make_mission_text/
// make_game_scene; table allocation/destruction operations; clear_game_queues.
// Scene supplies install_callback/fit_viewport. All destruction/queue/table-free
// operations must be nonthrowing and backend/registry must outlive the owner.
template<class Backend>
class SessionOwner {
public:
    using Overlay = OverlayOwner<typename Backend::FrameOwner>;
    struct Registry {
        SessionOwner* current = nullptr; //00698500 /008AE7E4
        std::array<std::uint32_t, 64> table_e6e0{}, table_e5e0{};
    };
    SessionOwner(Backend& backend, Registry& registry, MissionRequest request)
        : backend_(backend), registry_(registry), request_(std::move(request)) {
        if (registry_.current) throw std::logic_error("A game owner is already published");
        if (!request_.mission || request_.mission->kind != 3 ||
            request_.mission->text24 != request_.preparation.mission_resource ||
            request_.preparation.squad.empty())
            throw std::logic_error("Game receiver request is incomplete");
        try {
            hud_ = require(backend_.make_hud()); //006985C4→0049A420
            overlays_ = std::make_unique<Overlay>(); //00490740
            text_ = require(backend_.make_mission_text()); //0069860D..00698631
            scene_ = require(backend_.make_game_scene()); //driver+54(13)
            scene_->install_callback(); //scene+74(callback006C9260,6)
            scene_->fit_viewport(); //scene+148(0,0,physical width,height), if needed
            tables_ = std::make_unique<MissionTables<Backend>>(backend_); //00523AB0
            tables_->clear(); //00523E10
            registry_.current = this; //00698709, only after all above steps
            //00698732 constructs a real allocated empty COW string after publish.
            empty_ac_ = std::make_shared<const std::string>();
            registry_.table_e6e0.fill(0);
            registry_.table_e5e0.fill(0);
        } catch (...) {
            close(); // Also retract publication when empty-string allocation fails.
            throw;
        }
    }
    ~SessionOwner() { close(); }
    SessionOwner(const SessionOwner&) = delete;
    SessionOwner& operator=(const SessionOwner&) = delete;
    SessionOwner(SessionOwner&&) = delete; // Registry/callback identity must stay stable.
    SessionOwner& operator=(SessionOwner&&) = delete;
    const MissionRequest& request() const { return request_; }
    typename Backend::Scene* scene() const { return scene_.get(); }
    typename Backend::Hud* hud() const { return hud_.get(); }
    typename Backend::Text* text() const { return text_.get(); }
    Overlay* overlays() const { return overlays_.get(); }
    MissionTables<Backend>* tables() const { return tables_.get(); }
    bool constructed() const { return registry_.current == this; }
    bool loading_ready() const { return false; } // No activation/loader implementation in this owner.
    std::uint32_t status() const { return status_; } // Original ctor+04=0, pending+08=2.
    std::uint32_t pending_status() const { return 2; }

    void close() noexcept {
        if (closed_) return;
        closed_ = true;
        //00698830 empties the global transient queues before deleting subowners.
        // Only this owner's acquired prefix may be touched on construction failure.
        if (hud_ || overlays_ || text_ || scene_ || tables_) backend_.clear_game_queues();
        hud_.reset();
        overlays_.reset();
        text_.reset();
        // Mission-loaded byte is false here:0069A3D0 has no live mission work.
        // Scene release happens while00698500 can still return this owner.
        scene_.reset();
        if (registry_.current == this) registry_.current = nullptr;
        tables_.reset();
        empty_ac_.reset();
    }
private:
    template<class T> static std::unique_ptr<T> require(std::unique_ptr<T> value) {
        if (!value) throw std::bad_alloc();
        return value;
    }
    Backend& backend_;
    Registry& registry_;
    MissionRequest request_;
    std::uint32_t status_ = 0;
    bool closed_ = false;
    std::unique_ptr<typename Backend::Hud> hud_;
    std::unique_ptr<Overlay> overlays_;
    std::unique_ptr<typename Backend::Text> text_;
    std::unique_ptr<typename Backend::Scene> scene_;
    std::unique_ptr<MissionTables<Backend>> tables_;
    std::shared_ptr<const std::string> empty_ac_;
};

} // namespace hd2::game
#endif
