#ifndef HD2_GAME_SESSION_LIFECYCLE_H
#define HD2_GAME_SESSION_LIFECYCLE_H
#include "game/session_activation.h"
#include <exception>
namespace hd2::game {
//Source ownership coordinator; native status/loaded publication stays in the
//original loading/activation/finish/file policies. Services must be concrete.
template<class Services>class SessionLifecycleBoundary {
public:
    SessionLifecycleBoundary(SessionActivationState& state,Services& services):state_(state),services_(services){}
    SessionLifecycleBoundary(const SessionLifecycleBoundary&)=delete;
    SessionLifecycleBoundary& operator=(const SessionLifecycleBoundary&)=delete;
    bool activate(){
        if(state_.status==5)return false;
        if(attempted_)throw std::logic_error("Close this session lifecycle before another activation attempt");
        attempted_=true;
        bool result;
        try{
            result=services_.activate_prefix(progress_);
        }catch(...){
            const auto original=std::current_exception();
            try{unload();}catch(...){rollback_failure_=std::current_exception();}
            std::rethrow_exception(original);
        }
        if(!result){
            try{unload();}catch(...){rollback_failure_=std::current_exception();throw;}
            return false;
        }
        activated_=true;return true;
    }
    bool finish(){
        services_.retire_presentation();
        if(state_.status==1&&!activated_)throw std::logic_error("Status1 finish requires a completed retained activation");
        const auto result=services_.finish_prefix(progress_);
        if(state_.status==1&&progress_.finished)state_.status=6; //Repeat guard after original transition was already consumed.
        return result;
    }
    void unload(){
        (void)finish();services_.discard_loading_screen();services_.unload_files();services_.rethrow_cleanup_failure();
    }
    const auto& activation_progress()const noexcept{return progress_;}
    std::exception_ptr rollback_failure()const noexcept{return rollback_failure_;}
private:
    SessionActivationState& state_;Services& services_;
    SessionActivationProgress progress_;bool attempted_=false,activated_=false;
    std::exception_ptr rollback_failure_;
};
} //namespace hd2::game
#endif
