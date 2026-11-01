#ifndef HD2_FRONTEND_APPLICATION_STATES_H
#define HD2_FRONTEND_APPLICATION_STATES_H

#include "frontend/opening_playback.h"
#include "frontend/cleanup.h"
#include "frontend/game_data.h"
#include <memory>
#include <cstring>
#include <string>

namespace hd2::frontend {

// Native state ownership, not original vtable/object ABI. The pending word's
// sentinel2 is distinct from current status and from the virtual type query.
class ApplicationState {
public:
    explicit ApplicationState(std::uint32_t type, std::uint32_t status = 0)
        : type_(type), status_(status) {}
    virtual ~ApplicationState() = default;
    virtual bool initialize(const void*) = 0;
    virtual void shutdown() = 0;
    virtual void input(const std::uint32_t*, const std::uint32_t*) = 0;
    // Actual0067EC60 and frontend4C target0067EAA0 are empty RET4 bodies.
    virtual void fixed_update(std::uint32_t) {}
    virtual void update(std::uint32_t milliseconds) = 0;
    virtual void render() = 0;
    virtual void begin_video(const char*, std::uint32_t) {
        throw std::logic_error("Application state is not an opening movie owner");
    }
    virtual std::uint32_t video_sequence() const {
        throw std::logic_error("Application state has no opening sequence");
    }
    virtual std::string selected_video_filename() const { return {}; }
    std::uint32_t type() const { return type_; }
    std::uint32_t status() const { return status_; } // 0067EA00
    void status(std::uint32_t value) { status_ = value; } // 0067EA10
    std::uint32_t pending() const { return pending_; } // 0067EA20
    void pending(std::uint32_t value) { pending_ = value; } // 0067EA30
    void after_update() { // 0067EA40
        if (pending_ != 2) { status_ = pending_; pending_ = 2; }
    }
private:
    std::uint32_t type_, status_, pending_ = 2;
};

// Ordinary single-movie part of0066EAE0/ED30/ED50/ED80. The controller, not this
// owner, advances the four-movie sequence and replaces it with the frontend.
// Backend owns concrete engine resources and cleans partially initialized data.
// Special credits/outros retain their separate delivery boundary.
template<class Backend>
class OpeningApplicationState final : public ApplicationState {
public:
    explicit OpeningApplicationState(std::unique_ptr<Backend> backend)
        : ApplicationState(10), backend_(std::move(backend)) {
        if (!backend_) throw std::invalid_argument("Opening state needs an actual backend");
    }
    bool initialize(const void*) override {
        throw std::logic_error("Opening initialization requires an owned movie descriptor");
    }
    void begin_video(const char* filename, std::uint32_t sequence) override {
        if (live_ || !filename) throw std::logic_error("Invalid opening state lifetime");
        filename_ = filename;
        sequence_ = sequence;
        // Mark ownership before acquisition so a failing native backend still
        // owns its partial resources. The backend destructor reclaims them.
        live_ = true;
        try { backend_->begin_movie(filename_.c_str()); }
        catch (...) {
            // Partial acquisition belongs to the backend's release destructor;
            // it must not persist an unstarted movie or run the normal fade.
            live_ = false;
            throw;
        }
        status(1); // 0066EAE0 ignores initializer result and returns AL0.
    }
    void shutdown() override {
        if (!live_) return;
        live_ = false;
        backend_->close_movie(filename_.c_str());
        filename_.clear();
    }
    void input(const std::uint32_t*, const std::uint32_t*) override {
        if (backend_->next_scan() == 1) complete(); // Ordinary00688BB0; Space is special-outro only.
        promote_completion();
    }
    void update(std::uint32_t milliseconds) override {
        backend_->update_sound(milliseconds); // Actual00688C10 prefix.
        const auto playback = backend_->movie_status();
        if (playback == -1 || playback == 1) complete();
        backend_->update_subtitles(); //00688C10 continues after ordinary completion.
        promote_completion();
    }
    void render() override { backend_->draw_movie(); }
    std::uint32_t video_sequence() const override { return sequence_; }
private:
    void complete() {
        backend_->stop_movie();
        backend_->clear_present();
        status(2);
    }
    void promote_completion() { if (status() == 2) status(13); }
    std::unique_ptr<Backend> backend_;
    std::string filename_; // Owned descriptor strings replace original COW ABI.
    std::uint32_t sequence_ = 0;
    bool live_ = false;
};

//0066EAE0/00688BB0/00688C10/0066ED50 special Credits state. The actual
// text/music backend owns partial acquisition; sequence0 returns its saved owner.
template<class Backend>
class CreditsApplicationState final : public ApplicationState {
public:
    explicit CreditsApplicationState(std::unique_ptr<Backend> backend)
        : ApplicationState(10), backend_(std::move(backend)) {
        if (!backend_) throw std::invalid_argument("Credits state needs a backend");
    }
    bool initialize(const void*) override { throw std::logic_error("Credits needs its descriptor"); }
    void begin_video(const char* filename,std::uint32_t sequence) override {
        if (live_ || !filename || std::strcmp(filename,"credits.bik") || sequence)
            throw std::logic_error("Invalid Credits descriptor/lifetime");
        // Partial Credits acquisition can restore its music bookmark and release
        // fonts safely. Unwind it before the controller clears shared caches.
        live_=true;
        try { backend_->begin(); }
        catch(...) {
            live_=false;
            CleanupErrors errors(std::current_exception());
            errors.attempt([&] { backend_->close(); });
            errors.rethrow();
        }
        status(1);
    }
    void shutdown() override { if(live_) {live_=false;backend_->close();} }
    void input(const std::uint32_t*,const std::uint32_t*) override {
        if(backend_->next_scan()==1) {backend_->clear_present();status(13);}
    }
    void update(std::uint32_t milliseconds) override {
        backend_->update_sound(milliseconds);
        if(backend_->advance(milliseconds))status(13);
    }
    void render() override {backend_->draw();}
    std::uint32_t video_sequence() const override {return 0;}
private:
    std::unique_ptr<Backend> backend_;
    bool live_=false;
};

//00688040/00688BB0/00688C10's ordinary kind5 receiver. Its status2 belongs
//to descriptor traversal; it never enters the opening/retained status13 path.
//The descriptor is copied before initialization so controller replacement does
//not leave filename/flag storage borrowed from a destroyed screen owner.
template<class Backend>
class BriefingApplicationState final : public ApplicationState {
public:
    explicit BriefingApplicationState(std::unique_ptr<Backend> backend)
        : ApplicationState(7), backend_(std::move(backend)) {
        if (!backend_) throw std::invalid_argument("Briefing needs its actual movie backend");
    }
    bool initialize(const void* descriptor) override {
        if (live_ || !descriptor) throw std::logic_error("Briefing needs a kind5 descriptor");
        const auto& node=*static_cast<const GameDataNode*>(descriptor);
        if (node.kind!=5) throw std::logic_error("Briefing received a different descriptor kind");
        filename_=node.text10;
        subtitle_name_=node.text14;
        movie_flag_=node.flag18;
        live_=true;
        try { backend_->begin_briefing(filename_.c_str(),subtitle_name_.c_str(),movie_flag_); }
        catch (...) { live_=false; throw; }
        status(1);
        return true;
    }
    void shutdown() override {
        if (!live_) return;
        live_=false;
        backend_->close_movie(filename_.c_str());
    }
    void input(const std::uint32_t*,const std::uint32_t*) override {
        if (backend_->next_scan()==1) complete();
    }
    void update(std::uint32_t milliseconds) override {
        backend_->update_sound(milliseconds);
        const auto playback=backend_->movie_status();
        if (playback==-1 || playback==1) complete();
        backend_->update_subtitles();
    }
    void render() override { backend_->draw_movie(); }
private:
    void complete() { backend_->stop_movie();backend_->clear_present();status(2); }
    std::unique_ptr<Backend> backend_;
    std::string filename_,subtitle_name_;
    std::uint8_t movie_flag_=0;
    bool live_=false;
};

//006C0EF0's kind6 owners use the common menu lifecycle with their original
//state type (hero9, team3, equipment4). Runtime owns the derived screen set and
//borrows the shared preparation session across successive controller states.
template<class Runtime>
class PreMissionApplicationState final : public ApplicationState {
public:
    PreMissionApplicationState(std::uint32_t type,std::unique_ptr<Runtime> runtime)
        : ApplicationState(type,4),runtime_(std::move(runtime)) {
        if ((type!=9 && type!=3 && type!=4 && type!=2) || !runtime_)
            throw std::invalid_argument("Invalid pre-mission application owner");
    }
    bool initialize(const void* descriptor) override {
        if (!descriptor || (static_cast<const GameDataNode*>(descriptor)->kind!=6 &&
                            !(type()==2 && static_cast<const GameDataNode*>(descriptor)->kind==7)))
            throw std::logic_error("Setup owner needs its actual preparation descriptor");
        runtime_->initialize(*static_cast<const GameDataNode*>(descriptor),
            [this](std::uint32_t value){status(value);});
        status(1);
        return true;
    }
    void shutdown() override { runtime_->shutdown(); }
    void input(const std::uint32_t* first,const std::uint32_t* second) override {runtime_->input(first,second);}
    void update(std::uint32_t milliseconds) override {runtime_->update(milliseconds);}
    void render() override {runtime_->render();}
private:
    std::unique_ptr<Runtime> runtime_;
};

// Concrete runtime is constructed AFTER this state has been installed by the
// controller. It owns the bounded compiled Main/Exit menu, not a second loop.
template<class Runtime>
class FrontendApplicationState final : public ApplicationState {
public:
    explicit FrontendApplicationState(std::unique_ptr<Runtime> runtime)
        : ApplicationState(2, 4), runtime_(std::move(runtime)) {
        if (!runtime_) throw std::invalid_argument("Frontend state needs a runtime owner");
    }
    bool initialize(const void* descriptor) override {
        if (descriptor) throw std::logic_error("Only normal null-descriptor menu entry is recovered");
        runtime_->initialize([this] { status(4); }, [this](std::uint32_t value) { status(value); });
        status(1);
        return true;
    }
    std::string selected_video_filename() const override { return runtime_->selected_video_filename(); }
    void shutdown() override { runtime_->shutdown(); }
    void input(const std::uint32_t* first, const std::uint32_t* second) override {
        runtime_->input(first, second);
    }
    void update(std::uint32_t milliseconds) override { runtime_->update(milliseconds); }
    void render() override { runtime_->render(); }
private:
    std::unique_ptr<Runtime> runtime_;
};

} // namespace hd2::frontend
#endif
