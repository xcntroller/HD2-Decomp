#ifndef HD2_GAME_APPLICATION_FRAME_RUNTIME_H
#define HD2_GAME_APPLICATION_FRAME_RUNTIME_H
#include <memory>
#include <stdexcept>
#include <utility>
namespace hd2::game {
//Own ONLY the actual Runtime borrower used by the lifecycle's existing frame
//coordinator. The lifecycle/World outlives this member. No second coordinator,
//input manager, scheduler or readiness publication is created.
template<class Lifecycle,class Runtime>class ApplicationFrameRuntime {
public:
    explicit ApplicationFrameRuntime(Lifecycle& lifecycle):lifecycle_(lifecycle){
        static_assert(noexcept(lifecycle.retire_frames()),"Frame retirement must not throw");
    }
    ~ApplicationFrameRuntime(){retire();}
    ApplicationFrameRuntime(const ApplicationFrameRuntime&)=delete;
    ApplicationFrameRuntime& operator=(const ApplicationFrameRuntime&)=delete;
    template<class External,class Make>void bind(External& external,Make make){
        if(runtime_)throw std::logic_error("Application frame Runtime is already bound");
        auto candidate=make();if(!candidate)throw std::logic_error("Application frame Runtime construction returned NULL");
        //Lifecycle publishes only a complete coordinator; a failed constructor
        //cannot retain the candidate. unique_ptr publication after bind cannot fail.
        lifecycle_.bind_frames(external,*candidate);runtime_=std::move(candidate);
    }
    void retire()noexcept{lifecycle_.retire_frames();runtime_.reset();} //BEFORE World/callback retirement, also on failure/destruction.
private:
    Lifecycle& lifecycle_;std::unique_ptr<Runtime> runtime_;
};
}
#endif
