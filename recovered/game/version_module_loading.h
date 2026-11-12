#ifndef HD2_GAME_VERSION_MODULE_LOADING_H
#define HD2_GAME_VERSION_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/actor_frame_data.h"
#include "game/rigid_body.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <functional>
#include "game/session_packet_stream.h"

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
    void refresh(){ //0043C710 recomputes its snapshot AFTER part/script retirement.
        const auto model=descriptor_.frame;if(!model)return;
        const auto bounds=frames_.physics_object_bounds(model,false);typename Frames::Vector centre{};
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        if(bounds.minimum[0]<bounds.maximum[0]){
            typename Frames::Vector half{};
            for(unsigned axis=0;axis!=3;++axis){
                const float difference=static_cast<float>(static_cast<long double>(bounds.maximum[axis])-bounds.minimum[axis]);
                half[axis]=static_cast<float>(static_cast<long double>(difference)*.5f);
                centre[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])+half[axis]);
            }
            descriptor_.radius=static_cast<float>(std::sqrt((static_cast<long double>(half[2])*half[2]+static_cast<long double>(half[1])*half[1])+static_cast<long double>(half[0])*half[0]));
        }else descriptor_.radius=0; //Native centre stack lanes unwritten: source guard retains the prior centre.
        world_=frames_.actor_world_matrix(model);
        if(bounds.minimum[0]<bounds.maximum[0])descriptor_.position=frames_.car_transform(centre,world_);
        inverse_=frames_.car_inverse_matrix(world_);collisions_.move(descriptor_);
    }
private:
    Frames& frames_;Collisions& collisions_;Matrix world_{},inverse_{};Descriptor descriptor_;
};

//Registered Versions7 loading and lazy damage-stage transitions. Model/part
//metadata, collision snapshots and reload queue belong to this SAME owner.
template<class Frames,class Collisions>class VersionModuleLoading {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    using Collision=VersionModelCollision<Frames,Collisions>;
    struct TransitionServices {
        std::function<void(Frame,bool)> script; //false:0041D920 death; true:0041DBB0 hit.
        std::function<void(Frame,std::int32_t)> particle;
        std::function<void(Frame,std::int32_t)> sound;
        std::function<void(Frame,Frame,std::uint32_t,Frame,bool)> debris;
        std::function<void(std::uint32_t,std::uint32_t,std::uint32_t)> publish;
    };
    void bind_transition_services(TransitionServices services){
        if(services_)throw std::logic_error("Versions transition services already bound");
        services_=std::move(services);
    }
    struct Master;
    struct Part {
        VersionModuleLoading& module;Owner frame;Master* master;VersionLoadingParameters parameters;std::size_t index;
        std::int32_t health=0,stage=0,count=0;bool resources_bound=false;
        std::vector<std::int32_t> health20,sound50;std::vector<Frame> visual30,debris40;

        Part(VersionModuleLoading& owner,Frame value,Master& model,VersionLoadingParameters settings,std::size_t identity)
            :module(owner),frame(owner.frames_.retain_actor_frame(value)),master(&model),parameters(settings),index(identity),health(settings.version){}
        ~Part(){ //0043AEF0: metadata clear, frame release, THEN50/40/30/20 storage.
            module.clear_part_data(*this);frame.reset();
            std::vector<std::int32_t>().swap(sound50);std::vector<Frame>().swap(debris40);
            std::vector<Frame>().swap(visual30);std::vector<std::int32_t>().swap(health20);
        }
    };
    struct Master {
        VersionModuleLoading& module;Owner frame;std::size_t index;
        std::unique_ptr<Collision> collision;std::vector<std::unique_ptr<Part>> parts;
        std::optional<Owner> resource;bool changed=false,retired=false;

        Master(VersionModuleLoading& owner,Frame value,std::size_t identity):module(owner),frame(owner.frames_.retain_actor_frame(value)),index(identity){}
        ~Master(){module.clear_master_data(*this);while(!parts.empty())parts.pop_back();resource.reset();} //0043C1A0 reverse parts, then frame8 release.
    };
    VersionModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,
            Frames& frames,Collisions& collisions,const std::uint32_t& mode)
        :registration_(registry,registration,7,"Versions"),frames_(frames),collisions_(collisions),mode_(mode){}
    ~VersionModuleLoading(){close();}
    VersionModuleLoading(const VersionModuleLoading&)=delete;VersionModuleLoading& operator=(const VersionModuleLoading&)=delete;
    auto& registration(){return registration_.get();}const auto& masters()const{return masters_;}
    Frames& frames(){return frames_;}
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
            for(auto& candidate:masters_)if(candidate&&reinterpret_cast<std::uintptr_t>(candidate.get())==data->words[5]){master=candidate.get();break;}
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
    bool damage_master(std::uintptr_t borrowed,std::int32_t damage,std::uint32_t presentation){ //0043C6A0.
        require_open();auto* master=checked_master(borrowed);const auto index=master->index;bool result=true;
        for(auto n=master->parts.size();n;){--n;
            if(auto* part=master->parts[n].get()){
                if(!damage_part(*part,damage,nullptr,presentation))result=false;
                //The part callback may delete its own Master. Do not touch its
                //borrowed pointer after that point; preserve global slot recheck.
                if(index>=masters_.size()||!masters_[index])return false;
                master=masters_[index].get();
            }
        }
        return result;
    }
    bool receive_packet(SessionPacketStream* packet){ //0043F1E0, signed fields and late authority gate.
        require_open();if(!(mode_&2u)||!packet)return false;
        const auto master_word=packet->read(16),part_word=packet->read(8),stage_word=packet->read(8);
        const auto master_id=master_word&0x8000u?static_cast<std::int32_t>(master_word)-0x10000:static_cast<std::int32_t>(master_word);
        const auto part_id=part_word&0x80u?static_cast<std::int32_t>(part_word)-0x100:static_cast<std::int32_t>(part_word);
        const auto stage=stage_word&0x80u?static_cast<std::int32_t>(stage_word)-0x100:static_cast<std::int32_t>(stage_word);
        if(mode_&1u)return true;
        const auto index=static_cast<std::uint32_t>(master_id),slot=static_cast<std::uint32_t>(part_id);
        if(index>=masters_.size()||!masters_[index]||slot>=masters_[index]->parts.size()||!masters_[index]->parts[slot])return true;
        apply_part_stage(*masters_[index]->parts[slot],stage,true);
        //Raw reloads the global slots after0043C040. Reject the original NULL
        //dereference if missing resources retired this Master during application.
        if(!masters_[index])throw std::logic_error("Received Versions stage retired its native Master before global slot reread");
        (void)changed_part(*masters_[index],masters_[index]->parts[slot].get());return true;
    }
    bool damage_part(Part& part,std::int32_t damage,Frame* output,std::uint32_t presentation){ //0043B000.
        if(output)*output=part.frame.get();
        if(damage<part.health){part.health=word_subtract(part.health,damage);return true;}
        if(!part.resources_bound&&!bind_part_resources(part)){
            part.count=0;part.stage=0;(void)changed_part(*part.master,&part);return false;
        }
        auto next=part.stage;if(next>=part.count)return false;
        do{
            damage=word_subtract(damage,part.health);++next;if(next==part.count)break;
            part.health=part.health20.at(static_cast<std::size_t>(next));
        }while(part.health<=damage);
        if(next!=part.count)part.health=word_subtract(part.health,damage);
        frames_.replace_version_part(part,part.visual30.at(static_cast<std::size_t>(next-1))); //0043BA60 before publication.
        if(output)*output=part.frame.get();
        required_services().publish(static_cast<std::uint32_t>(part.master->index),static_cast<std::uint32_t>(part.index),static_cast<std::uint32_t>(next));
        if(mode_&2u){
            const bool effects=presentation==0?true:presentation==2?false:part.master->index%3u==0;
            apply_part_stage(part,next,effects);
        }
        part.stage=next;return changed_part(*part.master,&part);
    }
    bool bind_part_resources(Part& part){ //0043CCA0, actual resource frame chain; no eager loading at construction.
        auto& master=*part.master;
        if(!master.resource){
            auto resource=frames_.load_version_resource(master.frame.get());
            if(resource){master.resource.emplace(std::move(resource));required_services().script(master.frame.get(),false);}
        }
        if(!master.resource)return false;
        auto frame=frames_.actor_find(master.resource->get(),frames_.actor_short_name(part.frame.get()).c_str(),0xffff);
        if(!frame)return false;
        std::vector<Frame> seen;
        while(frame){
            if(std::find(seen.begin(),seen.end(),frame)!=seen.end())throw std::logic_error("Cyclic native Versions resource frame chain");
            seen.push_back(frame);const auto text=frames_.actor_user_data(frame);
            const auto health=version_loading_integer(text,"version",100),sound=version_loading_integer(text,"sound",-1);
            Frame debris{},next{};
            for(auto n=frames_.actor_child_count(frame);n;){const auto child=frames_.actor_child(frame,--n);
                if(child){const auto kind=frames_.frame_kind(child);if(kind==6)debris=child;else if(kind==1)next=child;} //0043CE10.
            }
            //Native four separate appends, in this order; partial failures keep
            //their preceding publications and do not fabricate a bound stage.
            part.debris40.push_back(debris);part.visual30.push_back(frame);part.health20.push_back(health);part.sound50.push_back(sound);
            frame=next;
        }
        if(part.health20.size()>std::size_t(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Versions resource count exceeds native DWORD");
        part.count=static_cast<std::int32_t>(part.health20.size());part.resources_bound=true;return true;
    }
    void apply_part_stage(Part& part,std::int32_t stage,bool effects){ //0043C040.
        if(!part.resources_bound&&part.master&&!bind_part_resources(part)){
            part.count=0;part.stage=0;(void)changed_part(*part.master,&part);return;
        }
        if(stage<0)stage=static_cast<std::int32_t>(part.visual30.size());
        if(stage<=part.stage)return;
        const auto index=static_cast<std::size_t>(stage-1);
        if(!(mode_&1u)||((mode_&7u)==7u&&!effects)){
            if(static_cast<std::size_t>(stage)<=part.visual30.size())frames_.replace_version_part(part,part.visual30.at(index));
        }
        if(effects&&part.parameters.particle>=0)required_services().particle(part.frame.get(),part.parameters.particle); //0043C000.
        if(static_cast<std::size_t>(stage)<=part.debris40.size())required_services().debris(part.frame.get(),part.debris40.at(index),
            static_cast<std::uint32_t>(part.parameters.material),part.master->frame.get(),effects);
        if(effects&&static_cast<std::size_t>(stage)<=part.sound50.size())required_services().sound(part.frame.get(),part.sound50.at(index));
        part.stage=stage;
    }
    bool changed_part(Master& master,Part* part){ //0043C710; may delete BOTH input owners.
        if(!part)return false;
        master.changed=true;required_services().script(master.frame.get(),true);bool result=true;
        if(part->stage==part->count){
            for(auto n=master.parts.size();n;){--n;if(master.parts[n].get()==part){master.parts[n].reset();result=false;break;}}
        }
        if(master.collision)master.collision->refresh();
        if(!master.parts.empty()&&std::none_of(master.parts.begin(),master.parts.end(),[](const auto& owner){return bool(owner);})){ //0043E7C0 leaves the global slot NULL.
            const auto index=master.index;master.retired=true;retire_master(index);
        }
        return result;
    }
    void clear(){ //0043E2C0: reverse masters, reverse deferred collision owners, then reverse borrowed reloads.
        std::exception_ptr failure;
        while(!masters_.empty()){
            auto master=std::move(masters_.back());masters_.pop_back();
            if(!master)continue;
            try{if(master->collision){
                if(frames_.version_collision_retires_immediately(master->frame.get()))master->collision.reset();
                else deferred_.push_back(std::move(master->collision)); //0043E830 appends, it does not remove.
            }}catch(...){if(!failure)failure=std::current_exception();master->collision.reset();}
            clear_master_data(*master);if(master->changed)reloads_.push_back(master->frame.get());
            master.reset();
        }
        while(!deferred_.empty())deferred_.pop_back();
        for(auto n=reloads_.size();n;){auto frame=reloads_[--n];if(frame)(void)frames_.reload_version_model(frame);}
        reloads_.clear();
        if(failure)std::rethrow_exception(failure);
    }
    void close()noexcept{if(closed_)return;closed_=true;try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
private:
    static std::int32_t word_subtract(std::int32_t left,std::int32_t right){
        const auto bits=static_cast<std::uint32_t>(left)-static_cast<std::uint32_t>(right);std::int32_t value;std::memcpy(&value,&bits,4);return value;
    }
    TransitionServices& required_services(){
        if(!services_||!services_->script||!services_->particle||!services_->sound||!services_->debris||!services_->publish)
            throw std::logic_error("Versions mutation requires actual containing transition services");
        return *services_;
    }
    Master* checked_master(std::uintptr_t borrowed){
        for(const auto& candidate:masters_)if(candidate&&reinterpret_cast<std::uintptr_t>(candidate.get())==borrowed)return candidate.get();
        throw std::logic_error("Versions contact has no actual live containing Master");
    }
    void retire_master(std::size_t index){
        auto owner=std::move(masters_.at(index));if(!owner)return;
        if(owner->collision){if(frames_.version_collision_retires_immediately(owner->frame.get()))owner->collision.reset();else deferred_.push_back(std::move(owner->collision));}
        clear_master_data(*owner);if(owner->changed)reloads_.push_back(owner->frame.get());owner.reset();
    }
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
    std::optional<TransitionServices> services_;std::vector<Frame> reloads_;
    std::vector<std::unique_ptr<Master>> masters_;std::vector<std::unique_ptr<Collision>> deferred_;bool closed_=false;
};
} //namespace hd2::game
#endif
