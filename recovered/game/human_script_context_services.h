#ifndef HD2_GAME_HUMAN_SCRIPT_CONTEXT_SERVICES_H
#define HD2_GAME_HUMAN_SCRIPT_CONTEXT_SERVICES_H

#include "game/human_sound_state_callbacks.h"
#include "game/mission_script_contexts.h"

namespace hd2::game {

//005518F0: replay actual Human2AC records reverse, then preserve a script
//state's last interrupt as a processed AI event. The two command queues and
//the process pool are the SAME owners used by normal hearing/AI producers.
template<class Frames,class Human,class Guard,class Factory>bool replay_human_guard_ai_handover(
        Frames& frames,Human& human,Guard& guard,const typename Factory::Base* previous,Factory& factory){
    if(!previous)return false;
    HumanSoundStateCallbacks<Frames,Human,Factory> callbacks(frames,factory);
    for(auto n=human.list2ac.size();n;)callbacks.b4(guard,*human.list2ac.at(--n));
    if(previous->words().template get<std::uint32_t>(0x48)!=5)return true;
    const auto* script=dynamic_cast<const typename Factory::Script*>(previous);
    if(!script)throw std::logic_error("Guard handover requires the actual previous script state");
    const auto point=script->state().template get<HumanVector>(0x140);
    if(point[1]>100000.f||point[1]<-100000.f)return false; //00810910/008109B0.
    std::uint32_t kind=0;
    switch(script->state().template get<std::uint32_t>(0x1b4)){
    case 1:kind=0x40000010u;break;case 2:kind=0x3000000cu;break;
    case 4:kind=0x3000000bu;break;case 8:kind=0x3000000du;break;
    case 0x10:kind=0x3000000eu;break;case 0x20:kind=0x30000001u;break;
    case 0x40:kind=0x40000015u;break;case 0x100:kind=0x20000013u;break;
    case 0x200:kind=0x20000004u;break;default:return true;
    }
    auto* command=guard.command();if(!command)throw std::logic_error("Guard handover requires its actual command owner");
    auto& queue=command->second.events;auto event=queue.pool().acquire();
    event->words[0]=kind;event->words[1]=10000;event->words[2]=0;
    event->words[3]=event->words[4]=0xffffffffu;event->words[12]=0;
    std::memcpy(event->words.data()+8,point.data(),16);queue.merge(std::move(event),10000,true);return true;
}

//Actual VM owner/clock/rand bridge required by the Human state factory. The
//model EC lookup includes00427730's pending local context before publication.
//Native context+EC is the alarm-done event PC, NOT the executing PC at+D8 or
//the pending stream operation at+24 (0041DA80/0041D8D0).
template<class Frames,class Human,class Contexts,class Runtime,class Random,class Lookup>
class HumanScriptContextServices {
public:
    using Context=typename Contexts::Context;
    HumanScriptContextServices(Frames& frames,Contexts& contexts,Runtime& runtime,Random& random,Lookup& lookup)
        :frames_(frames),contexts_(contexts),runtime_(runtime),random_(random),lookup_(lookup){}
    Context* human_script_context(Human& human){return contexts_.context_for_frame(human.base().frame());}
    void human_script_death(Human& human){if(auto* context=human_script_context(human))context->death(runtime_);} //0041D920, SAME module/runtime/context.
    std::int32_t human_script_status(const Context& context)const{return context.program().events.at(4);}
    void interrupt_human_script_context(Context& context){context.alarm(runtime_.script_authority());} //0041DA20.
    void finish_human_script_alarm(Context& context){(void)context.alarm_done(runtime_.script_authority());} //0041DA80.
    void finish_human_script_navigation(Context& context,std::int32_t pc){runtime_.complete_human_navigation(context,pc);} //00416D50.
    std::uint32_t human_script_random(){return random_();}
    template<class Guard,class Previous>void replay_guard_ai_handover(Guard& guard,const Previous& previous){
        auto* human=guard.human();if(!human||human->loading_state!=&guard)
            throw std::logic_error("Guard handover requires its actual published Human");
        auto& factory=lookup_.human_state_factory(*human);
        if(factory.control().human()!=human)throw std::logic_error("Guard handover factory belongs to another Human");
        (void)replay_human_guard_ai_handover<Frames,Human,Guard,std::decay_t<decltype(factory)>>(frames_,*human,guard,&previous,factory);
    }
private:
    Frames& frames_;Contexts& contexts_;Runtime& runtime_;Random& random_;Lookup& lookup_;
};

} //namespace hd2::game
#endif
