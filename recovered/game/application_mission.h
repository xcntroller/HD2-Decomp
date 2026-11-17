#ifndef HD2_GAME_APPLICATION_MISSION_H
#define HD2_GAME_APPLICATION_MISSION_H
#include "game/session_activation.h"
#include <memory>
#include <exception>
#include <stdexcept>

namespace hd2::game {
//The application owns this complete session before its existing SessionOwner
//can disappear. This boundary breaks the host World -> parent header cycle;
//it supplies no alternate World, status, scheduler or successful receiver.
class ApplicationMissionSession {
public:
    virtual ~ApplicationMissionSession()=default;
    virtual bool activate()=0;
    virtual void close()=0;
    virtual void input(std::uint32_t*,std::uint32_t*)=0;
    virtual void fixed_update(std::uint32_t)=0;
    virtual void update(std::uint32_t)=0;
    virtual void render()=0;
    virtual void show_session_warning(std::uint32_t,bool)=0;
    virtual std::uint32_t dispatch_scene(std::uint32_t)=0;
};

class ApplicationMission {
public:
    explicit ApplicationMission(std::uint32_t& status):state_(status){}
    ApplicationMission(const ApplicationMission&)=delete;
    ApplicationMission& operator=(const ApplicationMission&)=delete;
    SessionActivationState& state()noexcept{return state_;}
    void install(std::unique_ptr<ApplicationMissionSession> session){
        if(session_||attempted_||state_.loaded)throw std::logic_error("Application mission session is already owned or retired");
        if(!session)throw std::invalid_argument("Application mission factory returned no session");
        session_=std::move(session);
    }
    bool activate(){
        auto& session=required();
        if(attempted_)throw std::logic_error("Application mission activation repeated");
        attempted_=true;
        bool result;
        try{
            result=session.activate();
            if(result&&(!state_.loaded||state_.status!=5))
                throw std::logic_error("Session activation did not publish its actual loaded14/status5");
        }catch(...){
            const auto original=std::current_exception();
            try{close();}catch(...){cleanup_failure_=std::current_exception();}
            std::rethrow_exception(original);
        }
        if(!result)close();
        activated_=result;
        return result;
    }
    void close(){
        if(!session_)return;
        retiring_=true; //A failed retirement never permits further frame dispatch.
        try{
            session_->close(); //Retain graph and parent on failure for explicit retry.
            if(state_.loaded)throw std::logic_error("Session close left loaded14 published");
        }catch(...){cleanup_failure_=std::current_exception();throw;}
        session_.reset();
    }
    void close_preserving(std::exception_ptr primary){
        try{close();}catch(...){if(primary)std::rethrow_exception(primary);throw;}
    }
    void input(std::uint32_t* a,std::uint32_t* b){live().input(a,b);}
    void fixed_update(std::uint32_t elapsed){live().fixed_update(elapsed);}
    void update(std::uint32_t elapsed){live().update(elapsed);}
    void render(){live().render();}
    void show_session_warning(std::uint32_t caption,bool zero_argument){live().show_session_warning(caption,zero_argument);}
    std::uint32_t dispatch_scene(std::uint32_t stage){
        if(retiring_)throw std::logic_error("Application mission callback is retired");
        return required().dispatch_scene(stage); //Loading callbacks may precede status5.
    }
    std::exception_ptr cleanup_failure()const noexcept{return cleanup_failure_;}
private:
    ApplicationMissionSession& required(){
        if(!session_)throw std::logic_error("Application mission session is not installed");
        return *session_;
    }
    ApplicationMissionSession& live(){
        if(retiring_||!activated_)
            throw std::logic_error("Application mission is not active");
        return required();
    }
    //Session dies before its aliased activation state. The host declares this
    //owner after its parent so natural unwinding also preserves that lifetime.
    SessionActivationState state_;
    std::unique_ptr<ApplicationMissionSession> session_;
    std::exception_ptr cleanup_failure_;
    bool attempted_=false,activated_=false,retiring_=false;
};
}
#endif
