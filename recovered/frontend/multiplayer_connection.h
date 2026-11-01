#ifndef HD2_FRONTEND_MULTIPLAYER_CONNECTION_H
#define HD2_FRONTEND_MULTIPLAYER_CONNECTION_H

#include "frontend/multiplayer_settings.h"
#include <optional>

namespace hd2::frontend {

// Own the values before a menu/controller replacement. No borrowed row, edit
// buffer, COM context or descriptor pointer crosses the application boundary.
struct MultiplayerLaunchRequest {
    enum class Mode { client = 1, host = 2 };
    Mode mode = Mode::client;
    MultiplayerSettings settings;
    std::optional<MultiplayerServer> server;
    std::string password;
    void validate() const {
        if (mode==Mode::client) {
            if (!server || !server->port || (server->port&0xffffu)==0)
                throw std::logic_error("Multiplayer join requires an owned server address");
        } else if (mode==Mode::host) {
            if (settings.style>3 || settings.preset>4 || settings.hostname.empty() ||
                settings.rotations[settings.preset][settings.style].empty())
                throw std::logic_error("Multiplayer host requires its configured map rotation");
        } else throw std::logic_error("Invalid multiplayer connection mode");
    }
};

//0068AED0: timer crossing zero consumes that update; the following update
// executes the phase. A password request waits500ms and uses regular slot2.
// Services must report actual transport results; absence is a separate result,
// never success or an emulated DirectPlay error.
class MultiplayerConnectionFlow {
  public:
    enum class Submission { accepted, failed, unavailable };
    enum class Phase { initial=0, join=1, waiting=2, host=3, enter=4,
                       password=5, held=6, failed=7 };
    explicit MultiplayerConnectionFlow(MultiplayerLaunchRequest request) : request_(std::move(request)) {
        request_.validate();
    }
    const MultiplayerLaunchRequest& request() const { return request_; }
    Phase phase() const { return phase_; }
    template<class Services> void tick(unsigned milliseconds,bool console,Services& services) {
        if (!console) return;
        if (delay_>0) {delay_-=milliseconds; return;}
        switch (phase_) {
        case Phase::initial:
            services.clear_console();
            services.label(request_.mode==MultiplayerLaunchRequest::Mode::host?0x91b:0x91c);
            if (request_.mode==MultiplayerLaunchRequest::Mode::client) services.label(0x91d);
            delay_=300;
            phase_=request_.mode==MultiplayerLaunchRequest::Mode::host?Phase::host:Phase::join;
            break;
        case Phase::join: {
            const auto result=services.join(request_);
            if (result==Submission::accepted) phase_=Phase::waiting;
            else finish_submission(result,services);
            break;
        }
        case Phase::waiting:
            switch (services.result()) {
            case 1: services.label(0x923); services.label(0x924); phase_=Phase::failed; break;
            case 3: services.label(0x925); services.label(0x926); phase_=Phase::failed; break;
            case 4: services.label(0x927); delay_=500; phase_=Phase::password; break;
            case 5: services.label(0x929); services.label(0x928); phase_=Phase::failed; break;
            case 7: services.label(0x92a); services.label(0x928); phase_=Phase::failed; break;
            default: break;
            }
            break;
        case Phase::host: {
            const auto result=services.host(request_);
            if (result==Submission::accepted) {services.label(0x922);delay_=300;phase_=Phase::enter;}
            else finish_submission(result,services);
            break;
        }
        case Phase::enter: phase_=Phase::held; services.game_entry(request_); break;
        case Phase::password: phase_=Phase::held; services.password(); break;
        default: break;
        }
    }
    //0068A710 event02000069: genuine asynchronous connection completion.
    void connected() {
        if (phase_!=Phase::waiting) throw std::logic_error("Connection completion without pending join");
        phase_=Phase::enter; delay_=0;
    }
    //0068B9F0 requires a nonempty password, pops regular slot2, resets phase0.
    bool password(std::string value) {
        if (phase_!=Phase::held || value.empty()) return false;
        request_.password=std::move(value); phase_=Phase::initial; delay_=0; return true;
    }
  private:
    template<class Services> void finish_submission(Submission result,Services& services) {
        if (result==Submission::unavailable) services.unavailable();
        else {services.label(0x920);services.label(0x921);delay_=2500;}
        phase_=Phase::failed;
    }
    MultiplayerLaunchRequest request_;
    Phase phase_=Phase::initial;
    std::int64_t delay_=0;
};

} // namespace hd2::frontend
#endif
