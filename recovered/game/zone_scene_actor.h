#ifndef HD2_GAME_ZONE_SCENE_ACTOR_H
#define HD2_GAME_ZONE_SCENE_ACTOR_H
#include "game/scene_actor.h"
#include "game/mission_vertex_instances.h"
#include "game/human_spawn_position.h"
#include "frontend/cleanup.h"
#include <functional>
#include <optional>
namespace hd2::game {
//005EBED0/005EBFD0/005EDA60 actual type37 zone. The retained named frames and
//vertex instance belong to this actor; only registration borrows module9's list.
template<class Frames>class ZoneSceneActor final:public SceneActor<Frames>{
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().retain_actor_frame(Frame{}));
    using Vertex=MissionVertexInstance<Frame>;
    explicit ZoneSceneActor(Frames& frames):SceneActor<Frames>(frames,37){}
    ~ZoneSceneActor()override{try{retire();}catch(...){this->backend_.record_actor_cleanup_failure(std::current_exception());}}
    bool initialize(Frame frame)override{return frame&&this->base_.initialize_frame(frame);}
    void load(SerializedBytes& bytes)override{
        const auto version=bytes.word();const auto signed_version=signed_word(version);
        field90=signed_version<0x8001?version:bytes.word();field94=bytes.word();field98=bytes.word();fielda0=bytes.word();
        const auto raw=bytes.take(namesae.size());std::memcpy(namesae.data(),raw,namesae.size());
        fielda4=bytes.word();fielda8=bytes.word();if(signed_version>0x8000)flagac=bytes.byte();
        fielde4=field98;field9c=field98;
    }
    template<class Services,class Vertices>void activate(bool authority,Services& s,Vertices& vertices){ //005ED110.
        this->base_.activate_frame(authority,s);
        if(!flag118){
            flag118=true;
            for(unsigned n=0;n!=3;++n){
                const auto begin=namesae.data()+n*16;const auto end=static_cast<const char*>(std::memchr(begin,0,16));
                if(!end)throw std::runtime_error("Zone flag name exceeds native sixteen-byte field");
                flags104_[n]=this->backend_.game_scene_find(std::string(begin,static_cast<std::size_t>(end-begin)),0x80);
                if(flags104_[n])flag_owners_[n].emplace(this->backend_.retain_actor_frame(flags104_[n]));
            }
            vertex_=std::make_unique<Vertex>();
            unregister_=[this,&vertices]{if(vertex_)vertices.unregister_borrowed(*vertex_);};
            const auto name=this->backend_.car_short_name(this->base_.frame());
            if(name.size()+6>28)throw std::length_error("Zone spawn name exceeds native stack buffer");
            spawn110_=this->backend_.game_scene_find(name+"spawn",0x20);
            if(spawn110_)spawn_owner_.emplace(this->backend_.retain_actor_frame(spawn110_));
            flagf1=0;flagf0=0;
        }
        if(!authority){field98=0xffffffffu;flagad=0;refresh_owner(s,vertices);}
    }
    void deactivate(bool){retire();} //005ED260 does NOT call common00610510.
    template<class Services,class Vertices>void refresh_owner(Services& s,Vertices& vertices){ //005EC8B0.
        if(field98==fielde4)return;
        if((s.mode_flags()&2u)&&fielda4==0xffffffffu&&(!spawn_group()||s.zone_style()!=2)&&!s.zone_remote_ai())s.zone_hud_objective(field90,signed_word(fielde4));
        for(unsigned n=0;n!=3;++n)if(const auto frame=flag_frame(n)){
            this->backend_.object_visible(frame,fielde4==n);
            const auto current=flag_frame(n);if(!current)throw std::logic_error("Zone flag disappeared during visibility receiver");
            this->backend_.update_scene_object(current);
            if(fielde4==n){
                const auto* definition=vertices.definition("vlajka");
                const auto visual=this->backend_.actor_find(flag_frame(n),"vlajka",0x10001);
                if(visual&&this->backend_.visual_type(visual)==0x5f4a424fu&&definition){
                    if(!vertex_)throw std::logic_error("Zone flag animation requires its actual vertex instance");
                    vertex_->finished=true;vertices.reset_geometry(*vertex_);vertices.unregister_borrowed(*vertex_);
                    vertices.bind(*vertex_,visual,definition);vertices.register_borrowed(*vertex_);vertex_->ping_pong=false;
                    if(definition->sizes.empty()||definition->sizes.size()>0x80000000u)throw std::runtime_error("Zone vertex LOD count leaves native maximum undefined");
                    vertex_->maximum_lod=static_cast<std::int32_t>(definition->sizes.size()-1);vertex_->duration=210;vertex_->start=0;vertex_->repeat=true;vertex_->finished=false;
                }
            }
        }
        field98=fielde4;
    }
    template<class Runtime>bool next_spawn_pose(HumanVector& position,HumanVector& direction,bool cycle,Runtime& r){ //005ED700.
        unsigned attempts=0;
        do{
            if(!cycle&&cursor100==child_count())return false;
            spawn_pose(cursor100,position,direction);++cursor100;
            if(cycle){const auto count=child_count();if(!count)throw std::logic_error("Zone cycling spawn has zero native children");cursor100%=count;}
            if(field98!=2&&cursor100==child_count()){field98=2;r.publish_zone_owner(*this,2);}
            ++attempts;
            if(!r.spawn_position_occupied(position))break;
        }while(attempts<32&&r.zone_style()!=2);
        return attempts<32;
    }
    void spawn_pose(std::uint32_t index,HumanVector& position,HumanVector& direction){ //005ED550.
        if(!spawn_group())throw std::logic_error("Zone spawn requires its actual retained frame group");
        const auto frame=this->backend_.actor_child(spawn_group(),index);if(!frame)throw std::logic_error("Zone spawn child is missing");
        const auto p=this->backend_.actor_world_matrix(frame);position={{p[12],p[13],p[14],p[15]}};
        const auto d=this->backend_.actor_world_matrix(frame);direction=normalize_zone_spawn_direction({{d[8],d[9],d[10],d[11]}});
    }
    Frame spawn_group()const{return spawn110_;}
    Frame flag_frame(unsigned n)const{return flags104_.at(n);}
    std::uint32_t child_count(){if(!spawn_group())throw std::logic_error("Zone child count requires its actual retained group");return this->backend_.actor_child_count(spawn_group());}
    const Vertex* vertex()const{return vertex_.get();}
    std::uint32_t field90=0,field94=0,field98=2,field9c=2,fielda0=0xffffffffu,fielda4=0xffffffffu,fielda8=0;
    std::uint8_t flagac=1,flagad=0,flagf0=0,flagf1=0;std::array<char,48> namesae{}; //Cold constructor defines only each first byte; suffixes inaccessible until load.
    std::uint32_t fielde4=2,fielde8=0,fieldec=0,field114=0,cursor100=0;
    std::array<std::uint32_t,3> fieldsf4{};bool flag118=false;
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t out;std::memcpy(&out,&value,4);return out;}
    void retire(){
        if(!flag118)return;
        flag118=false;
        for(unsigned n=0;n!=3;++n){flag_owners_[n].reset();flags104_[n]=Frame{};} //Native pointer stays published through release callback.
        if(vertex_){if(unregister_)unregister_();vertex_.reset();}
        unregister_={};spawn_owner_.reset();spawn110_=Frame{};
    }
    std::array<Frame,3> flags104_{};Frame spawn110_{};std::array<std::optional<Owner>,3> flag_owners_;std::optional<Owner> spawn_owner_;std::unique_ptr<Vertex> vertex_;std::function<void()> unregister_;
};
} //namespace hd2::game
#endif
