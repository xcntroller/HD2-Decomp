#ifndef HD2_GAME_BLAST_WAVE_PROCESS_H
#define HD2_GAME_BLAST_WAVE_PROCESS_H
#include "game/actor_activation_math.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
namespace hd2::game {
//Actual process008AED48, DISTINCT from proximity008AED54. Retained frame
//owners must be releasable independently of the temporary World bridge.
template<class FrameOwner,class Actor>class BlastWaveProcess {
public:
    using Owner=FrameOwner;using ActorType=Actor;using Frame=decltype(std::declval<Owner&>().get());
    using Vector=std::array<float,4>;
    struct Node {
        explicit Node(Owner frame):frame(std::move(frame)){}
        ~Node(){frame.reset();next.reset();} //006DDF60: resource BEFORE recursive next, not reverse member order.
        Owner frame;Vector offset{};float threshold=0;std::unique_ptr<Node> next;
    };
    struct Record {
        Vector centre{};float progress10=0,power14=0,radius18=0;Actor* source1c=nullptr; //Borrowed Human passed to virtualD4, never a serialized identity/DWORD.
        bool same_sector24=false;Frame sector28{};std::unique_ptr<Node> head;
        std::vector<Actor*> hit_actors;
        ~Record(){head.reset();hit_actors.clear();} //006DDEE0: nodes before raw vector storage.
    };
    struct Request {Vector centre{};float radius=0,power=0;bool same_sector=false;Actor* source=nullptr;};
    ~BlastWaveProcess(){clear();}
    BlastWaveProcess()=default;BlastWaveProcess(const BlastWaveProcess&)=delete;BlastWaveProcess& operator=(const BlastWaveProcess&)=delete;
    void clear()noexcept{while(!records_.empty())records_.pop_back();} //006DF390(0,true), retain capacity.
    std::size_t size()const{return records_.size();}
    const auto& records()const{return records_;}
    template<class Services>bool create(const Request& request,Services& services){ //006DF170.
        auto record=construct(request,services);
        if(advance(*record,100,services))records_.push_back(std::move(record));
        return true; //Native consumed initial wave still returns1.
    }
    template<class Services>void update(std::uint32_t elapsed,Services& services){ //006DF390(elapsed,false).
        for(std::size_t n=records_.size();n;){--n;
            if(!advance(*records_.at(n),elapsed,services)){
                records_[n].reset();records_[n]=std::move(records_.back());records_.pop_back(); //Swap-LAST, no shift.
            }
        }
    }
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    static std::int32_t truncate(long double value){
        value=std::trunc(value);
        if(!std::isfinite(value)||value<-2147483648.L||value>2147483647.L)throw std::runtime_error("Blast wave value exceeds native integer conversion");
        return static_cast<std::int32_t>(value);
    }
    //Native insertion checks every node EXCEPT the tail; a one-node list
    //always appends the new node, even when the new threshold is smaller.
    static void insert(Record& record,std::unique_ptr<Node> node){
        if(!record.head){record.head=std::move(node);return;}
        auto* link=&record.head;
        while((*link)->next){
            if(node->threshold<(*link)->threshold){node->next=std::move(*link);*link=std::move(node);return;}
            link=&(*link)->next;
        }
        (*link)->next=std::move(node);
    }
    template<class Services>static std::unique_ptr<Record> construct(const Request& request,Services& services){ //006DDFC0.
        if(!std::isfinite(request.radius)||request.radius<=0||!std::isfinite(request.power))throw std::runtime_error("Invalid blast wave range/power");
        for(unsigned a=0;a!=3;++a)if(!std::isfinite(request.centre[a]))throw std::runtime_error("Non-finite blast wave centre");
        auto record=std::make_unique<Record>();record->centre=request.centre;record->radius18=request.radius;record->power14=request.power;
        record->source1c=request.source;record->same_sector24=request.same_sector;record->sector28=services.blast_scene_sector(request.centre);
        constexpr std::uint32_t mask=0xd8000u;
        const auto candidates=services.blast_radius_candidates(request.centre,record->radius18,mask);
        std::vector<Frame> frames;
        if(const auto* containing=services.blast_containing_object(request.centre,mask))frames.push_back(containing->frame);
        for(const auto* descriptor:candidates){
            if(!descriptor)throw std::runtime_error("Missing blast candidate descriptor");
            const auto frame=descriptor->frame;if(!frame)break; //Native stops, does not skip later descriptors.
            auto target=[&]{const auto current=descriptor->frame;
                if(current)if(const auto* actor=services.blast_direct_actor(current))if(services.blast_actor_type(*actor)==3)return services.blast_find(current,"a",0xffffu);
                return current;
            };
            bool found=false;for(const auto previous:frames)if(previous==target()){found=true;break;}
            if(!found)frames.push_back(target()); //Native repeats the actual Human lookup when appending.
        }
        services.blast_camera_impulse(record->centre,static_cast<std::uint32_t>(truncate(record->radius18)));
        for(std::size_t n=frames.size();n;){--n;const auto frame=frames[n];
            if(!frame)throw std::runtime_error("Blast candidate has no actual retained target frame");
            auto node=std::make_unique<Node>(services.retain_blast_frame(frame));
            const auto matrix=services.blast_world_matrix(frame);
            const auto local=services.blast_transform(record->centre,services.blast_inverse_matrix(matrix));
            const auto bounds=services.blast_frame_bounds(frame);
            long double square=0;
            for(unsigned a=0;a!=3;++a){const long double delta=local[a]<bounds.minimum[a]?static_cast<long double>(local[a])-bounds.minimum[a]:
                    bounds.maximum[a]<local[a]?static_cast<long double>(local[a])-bounds.maximum[a]:0.L;
                square+=delta*delta;
            }
            node->threshold=static_cast<float>(square); //Squared distance, NOT sqrt.
            const auto refreshed=services.blast_world_matrix(frame);
            for(unsigned a=0;a!=3;++a)node->offset[a]=static_cast<float>(static_cast<long double>(refreshed[12+a])-record->centre[a]);
            if(node->threshold<0)node->threshold=0;
            if(record->radius18<node->threshold)record->radius18=node->threshold;
            insert(*record,std::move(node));
        }
        return record;
    }
    template<class Services>static void apply_hit(Record& record,Frame frame,float power,const Vector& direction,bool heavy,Services& services){ //006DDD40.
        if(!(power>0))return;
        if(auto* actor=services.blast_actor_for_frame(frame)){
            if(std::find(record.hit_actors.rbegin(),record.hit_actors.rend(),actor)==record.hit_actors.rend()){
                bool damage=services.blast_actor_type(*actor)==4;
                if(!damage){
                    const auto material=services.blast_material_index(*actor,frame);
                    if(material!=-1){const auto absorption=static_cast<long double>(services.blast_material_absorption(material));
                        if(absorption<power){power=static_cast<float>(static_cast<long double>(power)-absorption);damage=true;}
                    }
                }
                if(damage&&(services.mode_flags()&1u))services.blast_actor_damage(*actor,record.source1c?2u:4u,power,record.centre,direction,record.source1c);
                record.hit_actors.push_back(actor); //Append even if damage/material/authority gate did not fire.
            }
        }
        if(services.mode_flags()&1u)services.blast_physics_impulse(frame,truncate(power),heavy?2u:1u);
    }
    template<class Services>static bool advance(Record& record,std::uint32_t elapsed,Services& services){ //006DE780.
        if(!record.head)return false;
        record.progress10=static_cast<float>(static_cast<long double>(signed_word(elapsed))*.20000000298023224f+record.progress10);
        unsigned count=0;
        do{
            const auto& node=*record.head;if(record.progress10<node.threshold)return true;
            const auto frame=node.frame.get();auto* actor=services.blast_actor_for_frame(frame);
            bool clear=false;
            if(actor&&services.blast_actor_type(*actor)==3){
                clear=record.same_sector24&&services.blast_human_sector(*actor)==record.sector28;
                if(!clear)clear=services.blast_human_clear(*actor,record.centre);
            }else{
                Vector start{},displacement{};
                for(unsigned a=0;a!=3;++a){start[a]=static_cast<float>(static_cast<long double>(node.offset[a])+record.centre[a]);displacement[a]=-node.offset[a];}
                clear=!services.blast_obstructed(start,displacement); //Static ray, layersFFFFFFFF/flagsB10000.
            }
            if(clear){
                const auto direction=actor_activation_direction(node.offset);
                const auto power=static_cast<float>(std::abs((1.L-static_cast<long double>(node.threshold)/record.radius18)*record.power14));
                apply_hit(record,frame,power,direction,count>10,services);++count;
            }
            auto old=std::move(record.head);record.head=std::move(old->next);old.reset(); //Release only consumed node.
            if(!record.head)return true; //One extra tick before record retirement, native behavior.
        }while(true);
    }
    std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
