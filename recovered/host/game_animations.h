#ifndef HD2_HOST_GAME_ANIMATIONS_H
#define HD2_HOST_GAME_ANIMATIONS_H

#include "game/animation_tables.h"
#include "game/human_animation_arbitration.h"
#include "host/loaded_resources.h"
#include "host/chunk_files.h"
#include "host/loading_input_capture.h"

namespace hd2::host {

class GameAnimationResources {
public:
    struct Release {
        MenuDriver::Release method = nullptr;
        void operator()(EngineFrame* frame) const noexcept { if (frame) method(frame); }
    };
    using Reference = std::unique_ptr<EngineFrame, Release>;
    explicit GameAnimationResources(LoadedResources& resources) : resources_(resources) {}
    template<class Output>int resolve_animation(const std::string& name, Output& output) {
        MenuDriver::Owner result(nullptr, nullptr);
        //0041FC10's real category3 resource cache, not a synthetic clip ID.
        const auto status = resources_.cache(3).get(name, result);
        output = Output(result.get(), typename Output::deleter_type{result.get_deleter()});
        result.release();
        return status;
    }
private:
    LoadedResources& resources_;
};

// The original animation tables/gate are process owners, not per-mission
// owners. Startup keeps this state alive across failed/repeated game entry and
// releases its references before the actual engine/cache owners disappear.
class GameAnimations {
public:
    using Tables = game::AnimationTables<GameAnimationResources>;
    GameAnimations(ChunkFiles& files, LoadedResources& resources) : files_(files), resources_(resources) {}
    bool ensure_loaded() {
        const auto original = gate_.original_result([&] {
            const auto bytes = read_loading_input(files_, "Tables\\anim.sav");
            if (!bytes) {
                std::fputs("Animation table missing: Tables\\anim.sav\n", stderr);
                return false;
            }
            tables_.load(*bytes, resources_);
            std::size_t names = 0, missing = 0;
            for (const auto& table : tables_.tables()) for (const auto& row : table) {
                if (!row.name.empty()) ++names;
                if (row.load_status < 0) ++missing;
            }
            std::printf("Animation tables decoded: rows=%zu names=%zu failed-resources=%zu bytes=%zu\n",
                Tables::row_count, names, missing, bytes->size());
            return true;
        });
        //00783350's repeat-attempt true must never stand in for real data.
        return original && tables_.available();
    }
    bool available() const { return tables_.available(); }
    const Tables& tables() const { return tables_; }
    const auto& row(unsigned table,unsigned index)const{
        if(!tables_.available())throw std::runtime_error("Actual animation tables are unavailable");
        return tables_.tables().at(table).at(index);
    }
    auto& arbitration_process(){return arbitration_;}
    EngineFrame* resource(unsigned table,unsigned row) const {
        return this->row(table,row).animation.get();
    }
    GameAnimationResources::Reference named_clip(const std::string& name){ //0077CE10→0041FC10, actual category3 loan; no table-loading side effect.
        GameAnimationResources::Reference clip;(void)resources_.resolve_animation(name,clip);return clip;
    }
    EngineFrame* holster_pose() { //0078CE20's0098ADB0, SAME category3 cache.
        if(!holster_pose_) {
            GameAnimationResources::Reference next;
            if(resources_.resolve_animation("tecko",next)<0||!next)
                throw std::runtime_error("Required Human holster pose tecko is unavailable");
            holster_pose_=std::move(next);
        }
        return holster_pose_.get();
    }
    void release_holster_pose() noexcept {holster_pose_.reset();} //0078CDB0, after its14 groups.
    void close() noexcept { tables_.clear();release_holster_pose(); }
private:
    ChunkFiles& files_;
    GameAnimationResources resources_;
    game::AnimationLoadGate gate_;
    Tables tables_;
    game::HumanAnimationArbitrationState<EngineFrame*> arbitration_;
    GameAnimationResources::Reference holster_pose_;
};

} // namespace hd2::host
#endif
