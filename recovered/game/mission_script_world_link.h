#ifndef HD2_GAME_MISSION_SCRIPT_WORLD_LINK_H
#define HD2_GAME_MISSION_SCRIPT_WORLD_LINK_H

#include "game/mission_script_context.h"
#include <functional>
#include <utility>

namespace hd2::game {

//A checked borrower breaks the containing Human/VM construction cycle. It owns
//no contexts, state, authority flag or registry. Bind after constructing the
//actual module and before any Human/context activation; unbind after module
//finish while its owners still live. An unbound lookup is never success/null.
template<class Frames>class MissionScriptWorldLink {
public:
    using Frame=typename Frames::Frame;using Context=MissionScriptContext<Frames>;
    explicit MissionScriptWorldLink(Frames& frames):frames_(frames){}
    MissionScriptWorldLink(const MissionScriptWorldLink&)=delete;
    MissionScriptWorldLink& operator=(const MissionScriptWorldLink&)=delete;
    template<class Module>void bind(Module& module){
        if(module_||&module.frames()!=&frames_||&module.runtime().module()!=&module)
            throw std::logic_error("Script world link requires its SAME live module/runtime/frame registry");
        std::function<Context*(Frame)> contexts=[&module](Frame frame){return module.contexts().context_for_frame(frame);};
        std::function<bool()> authority=[&module]{return module.runtime().script_authority();};
        contexts_=std::move(contexts);authority_=std::move(authority);module_=&module;
    }
    template<class Module>void unbind(Module& module)noexcept{
        if(module_!=&module)return;
        module_=nullptr;contexts_={};authority_={};
    }
    Context* context_for_frame(Frame frame){require_bound();return contexts_(frame);}
    bool script_authority()const{require_bound();return authority_();}
    bool bound()const noexcept{return module_!=nullptr;}
private:
    void require_bound()const{if(!module_)throw std::logic_error("Script world link has no actual containing module");}
    Frames& frames_;const void* module_=nullptr;
    std::function<Context*(Frame)> contexts_;std::function<bool()> authority_;
};
} //namespace hd2::game
#endif
