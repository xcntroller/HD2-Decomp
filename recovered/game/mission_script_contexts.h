#ifndef HD2_GAME_MISSION_SCRIPT_CONTEXTS_H
#define HD2_GAME_MISSION_SCRIPT_CONTEXTS_H

#include "game/mission_script_context.h"
#include "game/mission_script_compiler.h"
#include "game/mission_script_source.h"

namespace hd2::game {

//00427730 module18 owns the contexts, independently of the pending B810
//bindings. Runtime must create the ACTUAL base/Human/Dog family and perform
//005A3DF0/005907C0/Dog534+40 notifications. The owner never substitutes a base
//context when a derived receiver is required. Files is the live resolver.
template<class Frames,class Files,class Runtime>class MissionScriptContexts {
public:
    using Frame=typename Frames::Frame;
    using Context=MissionScriptContext<Frames>;
    MissionScriptContexts(Frames& frames,Files& files,Runtime& runtime,std::string resource):frames_(frames),source_(files,resource),runtime_(runtime),resource_(std::move(resource)){}
    const auto& values()const{return contexts_;}
    const std::string& error()const{return error_;}
    Context* context_for_frame(Frame frame)const { //0041CFA0, against this actual module's owners.
        if(!frame)return nullptr;
        const auto* data=frames_.find_actor_frame_data(frame);if(!data||!data->words[1])return nullptr;
        //00427730's just-constructed local context already owns EC word1
        //while005A3DF0 binds Human state5, before compilation/vector append.
        for(auto n=pending_.size();n;)if(pending_[--n]==reinterpret_cast<Context*>(data->words[1])){
            if(pending_[n]->frame()!=frame)throw std::runtime_error("Pending script EC binding disagrees with its actual frame");
            return pending_[n];
        }
        for(std::size_t n=contexts_.size();n;)if(contexts_[--n].get()==reinterpret_cast<Context*>(data->words[1])) {
            if(contexts_[n]->frame()!=frame)throw std::runtime_error("Script EC binding disagrees with its actual owner");
            return contexts_[n].get();
        }
        throw std::runtime_error("Script EC binding has no owner in the actual script module");
    }
    //00428490 context traversal only. Containing module must first drain its
    //actual deferred frame queue/timer and test loaded10; this entry claims
    //neither of those producers nor a successfully activated world.
    void tick_contexts(std::uint32_t elapsed,bool only_cutscenes) {
        for(std::size_t n=contexts_.size();n;) {
            auto* context=contexts_[--n].get();
            if(context&&(!only_cutscenes||context->during_cutscene()))context->step(elapsed,runtime_);
        }
    }
    void finish_contexts() { //00427B80's reverse exit→destroy context pass, after frame restoration.
        std::exception_ptr failure;
        while(!contexts_.empty()) {
            auto* context=contexts_.back().get();
            try{context->exit(runtime_);}catch(...){if(!failure)failure=std::current_exception();}
            contexts_.pop_back();
        }
        if(failure)std::rethrow_exception(failure);
    }
    bool bind(Frame frame,const char* script) {
        if(!frame)throw std::logic_error("Script binding requires an actual frame");
        const auto* data=frames_.find_actor_frame_data(frame);
        const bool existed=data&&data->words[1];
        const bool named=script&&*script;
        if(!named||existed) {
            for(std::size_t n=contexts_.size();n;)if(contexts_[--n]->frame()==frame) {
                contexts_[n]->exit(runtime_); //Original exit block precedes destruction and swap-pop.
                contexts_[n].reset();contexts_[n]=std::move(contexts_.back());contexts_.pop_back();
                return !existed||!named; //Existing nonempty binding REMOVES then reports false; no replacement.
            }
            return false;
        }
        error_.clear();
        auto context=runtime_.make_script_context(frames_,frame);
        if(!context)throw std::runtime_error("Actual script context factory returned no owner");
        if(context->frame()!=frame)throw std::runtime_error("Script context factory changed its bound frame");
        struct Pending {
            std::vector<Context*>& stack;Context* context;
            Pending(std::vector<Context*>& values,Context* value):stack(values),context(value){stack.push_back(context);}
            ~Pending(){close();}
            void close()noexcept{if(context){stack.pop_back();context=nullptr;}}
        } pending(pending_,context.get());
        const auto target=runtime_.prepare_script_binding(*context);
        std::string path="Scripts\\"+resource_+"\\"+script;
        //Native uses strstr twice, not a case-insensitive extension test.
        if(path.find(".scr")==std::string::npos&&path.find(".SCR")==std::string::npos)path+=".scr";
        if(path.size()>=0x400)throw std::length_error("Script path exceeds native compiler path buffer");
        auto text=source_.load(path);
        if(text) {
            MissionScriptCompiler compiler(context->program());
            if(compiler.compile(std::move(*text))) {
                auto* published=context.get();contexts_.push_back(std::move(context));pending.close();
                published->begin(runtime_.script_authority());
                runtime_.script_binding_completed(target,published);return true;
            }
            error_=compiler.error();
        }else error_="Cannot open script: "+path;
        pending.close();context.reset(); //EC/ref gone BEFORE the actual captured failure notification.
        runtime_.script_binding_completed(target,static_cast<Context*>(nullptr));return false;
    }
private:
    Frames& frames_;MissionScriptSource<Files> source_;Runtime& runtime_;std::string resource_,error_;
    std::vector<std::unique_ptr<Context>> contexts_;
    std::vector<Context*> pending_; //Borrow only each active bind()'s actual local owner, including nested frames.
};
} //namespace hd2::game
#endif
