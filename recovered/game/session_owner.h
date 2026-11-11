#ifndef HD2_GAME_SESSION_OWNER_H
#define HD2_GAME_SESSION_OWNER_H

#include "frontend/pre_mission_handoff.h"
#include "frontend/game_configuration.h"
#include "game/mission_tables.h"
#include "game/overlay_owner.h"
#include <array>
#include <memory>
#include <stdexcept>
#include <utility>

namespace hd2::game {

// The kind7 marker is not the initializer's mission argument.006B9190 returns
// the real kind3 mission (or the synthetic standalone copy), with child/objective
// descriptors consumed after loading. Its copied soldier/equipment payload is
// immutable handoff metadata, not another live squad. SessionOwner separately
// borrows application+14's authoritative mutable records through all spawning.
class MissionDescriptor {
public:
    MissionDescriptor()=default;
    MissionDescriptor(std::unique_ptr<frontend::GameDataNode> owned):owned_(std::move(owned)){}
    MissionDescriptor(MissionDescriptor&& other)noexcept:owned_(std::move(other.owned_)),borrowed_(std::exchange(other.borrowed_,nullptr)){}
    MissionDescriptor& operator=(MissionDescriptor&& other)noexcept{
        if(this!=&other){owned_=std::move(other.owned_);borrowed_=std::exchange(other.borrowed_,nullptr);}return *this;
    }
    MissionDescriptor(const MissionDescriptor&)=delete;MissionDescriptor& operator=(const MissionDescriptor&)=delete;
    static MissionDescriptor borrow_network(frontend::GameData& application){
        MissionDescriptor result;result.borrowed_=&application.multiplayer_mission();return result;
    }
    frontend::GameDataNode* get()const{return owned_?owned_.get():borrowed_;}
    frontend::GameDataNode* operator->()const{return get();}
    frontend::GameDataNode& operator*()const{return *get();}
    explicit operator bool()const{return get()!=nullptr;}
    bool borrowed()const{return borrowed_!=nullptr;}
private:
    std::unique_ptr<frontend::GameDataNode> owned_;frontend::GameDataNode* borrowed_=nullptr;
};
struct MissionRequest {
    frontend::PreMissionGameRequest preparation;
    MissionDescriptor mission;
    struct NetworkContext {
        frontend::GameData& data;const frontend::GameConfiguration& configuration;
        const std::uint32_t& process_flags;const frontend::PreMissionRoute& route;
    };
    std::optional<NetworkContext> network;
};
inline void validate_mission_request(const MissionRequest& request){
    if(!request.mission||request.mission->text24.empty()||request.mission->text24!=request.preparation.mission_resource)
        throw std::logic_error("Game receiver requires its actual selected mission");
    if(request.network){
        const auto& context=*request.network;
        //006B9190 tests the existing standalone mode FIRST. The source route
        //owns that adapted mode, as in the offline selection bridge. Its saved
        //descriptor is not interchangeable with inline MP+14.
        if(context.route.standalone())throw std::runtime_error("Network request requires the actual standalone descriptor selection first");
        if((context.process_flags&7u)==7u||!context.configuration.field04||
           request.mission.get()!=&context.data.multiplayer_mission()||!request.mission.borrowed()||
           request.mission->kind!=13||!request.mission->network_mission)
            throw std::logic_error("Network request requires SAME application map/configuration/process owners");
        for(const auto& word:request.mission->network_mission->words50)
            if(!word)throw std::runtime_error("Network mission metadata lacks its actual producer");
        for(const auto& row:request.mission->related)
            if(!row||row->kind!=12||!row->network_objective)throw std::logic_error("Network mission objective lacks its actual kind12 producer");
    }else if(request.mission.borrowed()||request.mission->kind!=3||request.preparation.squad.empty())
        throw std::logic_error("Game receiver requires its prepared campaign mission and squad");
}
inline MissionRequest copy_mission_request(const frontend::PreMissionGameRequest& request,
                                           const frontend::GameDataNode& mission) {
    if (mission.kind != 3 || mission.text24.empty() || mission.text24 != request.mission_resource)
        throw std::logic_error("Game receiver requires its actual selected mission");
    if (request.squad.empty()) throw std::logic_error("Game receiver requires its prepared squad");
    return {request, frontend::copy_pre_mission_node(mission),std::nullopt};
}
//006B9190's nonzero config04 selection BORROWS application GameData+14.
//The application must outlive the request/session and its World borrowers.
//No route, peer, loading status or successful connection is manufactured here.
inline MissionRequest make_network_mission_request(frontend::GameData& data,
        frontend::GameConfiguration& configuration,std::uint32_t& process_flags,
        const frontend::PreMissionRoute& route){
    if(route.standalone())throw std::runtime_error("Network request requires the actual standalone descriptor selection first");
    MissionRequest result;result.mission=MissionDescriptor::borrow_network(data);
    result.network.emplace(MissionRequest::NetworkContext{data,configuration,process_flags,route});
    result.preparation.root=data.field_e8;result.preparation.mission_resource=result.mission->text24;
    result.preparation.mission_caption=result.mission->word8;
    validate_mission_request(result);return result;
}
inline MissionRequest make_network_mission_request(frontend::GameData&,
        frontend::GameConfiguration&,std::uint32_t&,const frontend::PreMissionRoute&&)=delete;

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
    using LiveSquad=std::vector<frontend::PreMissionSoldier>;
    struct Registry {
        SessionOwner* current = nullptr; //00698500 /008AE7E4
        std::array<std::uint32_t, 64> table_e6e0{}, table_e5e0{};
    };
    SessionOwner(Backend& backend, Registry& registry, MissionRequest request,LiveSquad& live_squad)
        : backend_(backend), registry_(registry), request_(std::move(request)),live_squad_(live_squad) {
        if (registry_.current) throw std::logic_error("A game owner is already published");
        validate_mission_request(request_);
        if (!request_.network&&live_squad_.empty())
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
    Backend& backend()const{return backend_;} //Borrow SAME original factory/services for presentation composition.
    const MissionRequest& request() const { return request_; }
    LiveSquad& live_squad(){
        if(closed_)throw std::logic_error("Live squad access after session retirement");
        return live_squad_;
    }
    const LiveSquad& live_squad()const{
        if(closed_)throw std::logic_error("Live squad access after session retirement");
        return live_squad_;
    }
    typename Backend::Scene* scene() const { return scene_.get(); }
    typename Backend::Hud* hud() const { return hud_.get(); }
    typename Backend::Text* text() const { return text_.get(); }
    Overlay* overlays() const { return overlays_.get(); }
    MissionTables<Backend>* tables() const { return tables_.get(); }
    bool constructed() const { return registry_.current == this; }
    const auto& neutral_first_input()const noexcept{return registry_.table_e6e0;}
    const auto& neutral_second_input()const noexcept{return registry_.table_e5e0;}
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
    LiveSquad& live_squad_; //Application owner outlives every session/Human/Items borrower; never copied or cleared here.
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
