#ifndef HD2_GAME_VERSION_MODULE_LOADING_H
#define HD2_GAME_VERSION_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/actor_frame_data.h"
#include "game/rigid_body.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace hd2::game {

struct VersionLoadingParameters {std::int32_t version=100,particle=-1,material=0;bool transparent=false;};

//0043E240 uses strstr followed by sscanf("%d"). Embedded keywords count.
//Reject the original overflowing integer/buffer cases instead of invoking UB.
inline std::int32_t version_loading_integer(const std::string& text,const char* keyword,std::int32_t fallback){
    const auto where=text.find(keyword);if(where==std::string::npos)return fallback;
    const auto first=text.c_str()+where+std::char_traits<char>::length(keyword);char* end=nullptr;
    errno=0;const auto value=std::strtoll(first,&end,10);if(first==end)return fallback;
    if(errno==ERANGE||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
        throw std::runtime_error("Version metadata integer exceeds native range");
    return static_cast<std::int32_t>(value);
}
inline std::optional<VersionLoadingParameters> version_loading_parameters(std::uint32_t kind,std::string text){
    const auto nul=text.find('\0');if(nul!=std::string::npos)text.resize(nul);
    if(text.empty())return std::nullopt;
    if(text.size()>=1024)throw std::runtime_error("Version metadata exceeds original buffer");
    for(auto& c:text)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
    if(kind==9||text.find("version")==std::string::npos)return std::nullopt;
    return VersionLoadingParameters{version_loading_integer(text,"version",100),version_loading_integer(text,"particle",-1),
        version_loading_integer(text,"matid",0),text.find("trans")!=std::string::npos};
}

//0043DDB0's reached model-kind9 collision snapshot. Native kind6 is not reached
//through0043E560's model-owner gate. Share the SAME descriptor type/grid as all
//actors; no RigidBody or independent collision registry is constructed here.
template<class Frames,class Collisions>class VersionModelCollision {
public:
    using Frame=typename Frames::Frame;using Matrix=typename Frames::Matrix;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    VersionModelCollision(Frames& frames,Collisions& collisions):frames_(frames),collisions_(collisions){}
    ~VersionModelCollision(){collisions_.remove_for_cleanup(descriptor_);} //0043E1F0→006EF5E0.
    VersionModelCollision(const VersionModelCollision&)=delete;VersionModelCollision& operator=(const VersionModelCollision&)=delete;
    void initialize(Frame model,bool transparent){
        if(!model||frames_.frame_kind(model)!=9)throw std::logic_error("Version collision requires its actual model owner");
        const auto bounds=frames_.physics_object_bounds(model,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Version aggregate bounds leave the native centre undefined");
        const auto scale=frames_.actor_local_scale(model);
        descriptor_.flags=transparent?0x100040u:0x40u;descriptor_.mask=0x20000;descriptor_.frame=model;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        for(unsigned axis=0;axis!=3;++axis){
            if(!std::isfinite(scale[axis])||scale[axis]==0)throw std::runtime_error("Version collision has a singular local scale");
            descriptor_.minimum[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])*scale[axis]);
            descriptor_.maximum[axis]=static_cast<float>(static_cast<long double>(bounds.maximum[axis])*scale[axis]);
        }
        descriptor_.position=frames_.car_transform(bounds.centre,frames_.actor_world_matrix(model));
        world_=frames_.actor_world_matrix(model);
        for(unsigned axis=0;axis!=3;++axis){
            const auto reciprocal=1.0L/scale[axis];
            for(unsigned component=0;component!=3;++component)
                world_[axis*4+component]=static_cast<float>(reciprocal*world_[axis*4+component]);
        }
        inverse_=frames_.car_inverse_matrix(world_);descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;
        const auto world_scale=frames_.car_scale(model);const auto yz=world_scale[1]>world_scale[2]?world_scale[1]:world_scale[2];
        const auto maximum=world_scale[0]>yz?world_scale[0]:yz;
        descriptor_.radius=static_cast<float>(static_cast<long double>(maximum)*bounds.radius);
        //Original recomputes transformed centre after normalization/radius work.
        descriptor_.position=frames_.car_transform(bounds.centre,frames_.actor_world_matrix(model));
        collisions_.insert(descriptor_);collisions_.move(descriptor_);
    }
    auto& descriptor(){return descriptor_;}
private:
    Frames& frames_;Collisions& collisions_;Matrix world_{},inverse_{};Descriptor descriptor_;
};

//Registered Versions7 loading/retirement, including actual positive model/part
//owners. No version-changing/mesh extraction producer is supplied: cold master
//C and flag28, and part resource arrays20/30/40/50, remain genuinely empty.
template<class Frames,class Collisions>class VersionModuleLoading {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    using Collision=VersionModelCollision<Frames,Collisions>;
    struct Master;
    struct Part {
        VersionModuleLoading& module;Owner frame;Master* master;VersionLoadingParameters parameters;std::size_t index;
        Part(VersionModuleLoading& owner,Frame value,Master& model,VersionLoadingParameters settings,std::size_t identity)
            :module(owner),frame(owner.frames_.retain_actor_frame(value)),master(&model),parameters(settings),index(identity){}
        ~Part(){module.clear_part_data(*this);} //0043AEF0: metadata clear before retained-frame release.
    };
    struct Master {
        VersionModuleLoading& module;Owner frame;std::size_t index;
        std::unique_ptr<Collision> collision;std::vector<std::unique_ptr<Part>> parts;
        Master(VersionModuleLoading& owner,Frame value,std::size_t identity):module(owner),frame(owner.frames_.retain_actor_frame(value)),index(identity){}
        ~Master(){module.clear_master_data(*this);while(!parts.empty())parts.pop_back();} //0043C1A0 reverse parts, then frame8 release.
    };
    VersionModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,
            Frames& frames,Collisions& collisions,const std::uint32_t& mode)
        :registration_(registry,registration,7,"Versions"),frames_(frames),collisions_(collisions),mode_(mode){}
    ~VersionModuleLoading(){close();}
    VersionModuleLoading(const VersionModuleLoading&)=delete;VersionModuleLoading& operator=(const VersionModuleLoading&)=delete;
    auto& registration(){return registration_.get();}const auto& masters()const{return masters_;}
    void finish_loading(bool){require_open();} //0043DC90 genuine RET4.
    void activate(bool authority){ //0043DD40→0043E2C0/0043E3E0.
        require_open();if(!authority&&(mode_&1u))return;
        clear();frames_.enumerate_activation_scene(false,0xffff,[&](Frame frame){visit(frame);});
    }
    void finish(bool authority){require_open();if(authority||!(mode_&1u))clear();} //0043DD90.
    void visit(Frame frame){ //0043E400 callback (absent frozen); ignores null/empty metadata.
        require_open();const auto parameters=version_loading_parameters(frames_.frame_kind(frame),frames_.actor_user_data(frame));
        if(!parameters)return;
        if(!parameters->material)std::fprintf(stderr,"###### C_VersionMaster::BuildVersionKeepersFromScene() : frame with default material ['%s']######",frames_.frame_name(frame).c_str());
        add(frame,*parameters);
    }
    void add(Frame frame,const VersionLoadingParameters& parameters){ //0043E560, actual same modelEC word5.
        require_open();if(!frame)return;
        const auto model=frames_.frame_model_owner(frame);if(!model||frames_.frame_kind(model)!=9)return;
        const auto* data=frames_.find_actor_frame_data(model);Master* master=nullptr;
        if(data&&data->words[5]){
            for(auto& candidate:masters_)if(reinterpret_cast<std::uintptr_t>(candidate.get())==data->words[5]){master=candidate.get();break;}
            if(!master)throw std::runtime_error("Version model metadata has no actual containing owner");
        }else{
            auto next=std::make_unique<Master>(*this,model,masters_.size());
            frames_.actor_frame_data(model).words[5]=reinterpret_cast<std::uintptr_t>(next.get());
            next->collision=std::make_unique<Collision>(frames_,collisions_);next->collision->initialize(model,parameters.transparent);
            master=next.get();masters_.push_back(std::move(next));
        }
        const auto* part_data=frames_.find_actor_frame_data(frame);if(part_data&&part_data->words[5])return; //0043C440 native existing gate.
        auto part=std::make_unique<Part>(*this,frame,*master,parameters,master->parts.size());
        auto& writable=frames_.actor_frame_data(frame);writable.words[5]=reinterpret_cast<std::uintptr_t>(master);writable.words[6]=reinterpret_cast<std::uintptr_t>(part.get());
        master->parts.push_back(std::move(part));
    }
    void clear(){ //0043E2C0: reverse masters, reverse deferred collision owners, then borrowed reloads(empty).
        std::exception_ptr failure;
        while(!masters_.empty()){
            auto master=std::move(masters_.back());masters_.pop_back();
            try{if(master->collision){
                if(frames_.version_collision_retires_immediately(master->frame.get()))master->collision.reset();
                else deferred_.push_back(std::move(master->collision)); //0043E830 appends, it does not remove.
            }}catch(...){if(!failure)failure=std::current_exception();master->collision.reset();}
            master.reset();
        }
        while(!deferred_.empty())deferred_.pop_back();
        if(failure)std::rethrow_exception(failure);
    }
    void close()noexcept{if(closed_)return;closed_=true;try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
private:
    void require_open()const{if(closed_)throw std::logic_error("Versions world has been retired");}
    void clear_master_data(Master& master)noexcept{
        try{auto& data=frames_.actor_frame_data(master.frame.get());if(data.words[5]==reinterpret_cast<std::uintptr_t>(&master))data.words[5]=0;}
        catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void clear_part_data(Part& part)noexcept{
        try{auto& data=frames_.actor_frame_data(part.frame.get());
            if(data.words[5]==reinterpret_cast<std::uintptr_t>(part.master))data.words[5]=0;
            if(data.words[6]==reinterpret_cast<std::uintptr_t>(&part))data.words[6]=0;
        }catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    frontend::ClientModuleBinding registration_;Frames& frames_;Collisions& collisions_;const std::uint32_t& mode_;
    std::vector<std::unique_ptr<Master>> masters_;std::vector<std::unique_ptr<Collision>> deferred_;bool closed_=false;
};
} //namespace hd2::game
#endif
