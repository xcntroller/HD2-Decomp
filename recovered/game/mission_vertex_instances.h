#ifndef HD2_GAME_MISSION_VERTEX_INSTANCES_H
#define HD2_GAME_MISSION_VERTEX_INSTANCES_H

#include "game/serialized_bytes.h"
#include "frontend/vertex_animations.h"
#include <algorithm>
#include <cmath>
#include <string_view>

namespace hd2::game {

//00474290/00473790's instance values borrow the SAME startup definitions and
//scene frames. Clear instances before either owner. Native addresses/padding do
//not participate in serialization. The32-byte vertex rows remain engine ABI.
template<class Frame>struct MissionVertexInstance {
    std::int32_t maximum_lod=0,start=0,current=0,next=1,elapsed=0,duration=250;
    bool ping_pong=false,backward=false,finished=false,repeat=true;
    std::uint32_t deferred=0,visibility_required=1;
    Frame frame{};
    const frontend::VertexAnimation* definition=nullptr;
};

template<class Services>
class MissionVertexInstances {
public:
    using Frame=typename Services::Frame;
    using Instance=MissionVertexInstance<Frame>;
    struct Registration {
        std::unique_ptr<Instance> owned;Instance* value;
        explicit Registration(std::unique_ptr<Instance> item):owned(std::move(item)),value(owned.get()){}
        explicit Registration(Instance& item):value(&item){}
        Instance* get()const{return value;}Instance* operator->()const{return value;}Instance& operator*()const{return *value;}
    };
    MissionVertexInstances(Services& services,frontend::VertexAnimations& definitions,
                          const std::uint32_t& mode,std::int32_t& clock)
        : services_(services),definitions_(definitions),mode_(mode),clock_(clock) {}
    const auto& values() const {return instances_;}
    const frontend::VertexAnimation* definition(const std::string& name)const{return find_definition(name);}
    void register_borrowed(Instance& instance) { //Same0089B38C publication as actor-owned SupplyBox214.
        if(std::any_of(instances_.begin(),instances_.end(),[&](const auto& value){return value.get()==&instance;}))throw std::logic_error("Vertex instance already has its actual registration");
        instances_.emplace_back(instance);
    }
    void unregister_borrowed(Instance& instance) { //004746B0(instance,false): ascending first match, swap/pop.
        for(std::size_t n=0;n!=instances_.size();++n)if(instances_[n].get()==&instance) {
            if(instances_[n].owned)throw std::logic_error("Actor cannot retire a file-owned vertex instance as a borrower");
            if(n+1!=instances_.size())instances_[n]=std::move(instances_.back());
            instances_.pop_back();return;
        }
    }
    void bind(Instance& instance,Frame frame,const frontend::VertexAnimation* definition) { //00473790.
        instance.frame=frame;instance.definition=definition;instance.backward=false;
        instance.current=0;instance.next=1;instance.elapsed=0;instance.deferred=0;
    }
    void reset_geometry(Instance& instance) { //004737C0, independent of visibility epoch/finished/duration.
        if(!instance.frame||!instance.definition)return;
        const auto lod=services_.vertex_animation_lod(instance.frame);if(lod>instance.maximum_lod)return;
        if(lod<0||static_cast<std::size_t>(lod)>=instance.definition->sizes.size()||instance.definition->frames.empty())throw std::runtime_error("Invalid initial vertex geometry LOD");
        const auto& base=instance.definition->frames.front().at(static_cast<std::size_t>(lod));
        const auto count=instance.definition->sizes[static_cast<std::size_t>(lod)];if(base.bytes/32<count)throw std::runtime_error("Short initial vertex definition block");
        services_.write_vertex_animation(instance.frame,static_cast<std::uint32_t>(lod),count,[&](std::size_t index,unsigned char* output){
            for(unsigned component:{0u,1u,2u,4u,5u,6u}) {
                float value;std::memcpy(&value,base.data.get()+index*32+component*4,4);
                const auto zero=static_cast<long double>(value)*0.f;
                const float result=component==0||component==4?static_cast<float>(zero+value):static_cast<float>(static_cast<long double>(value)+static_cast<float>(zero));
                //Definitions pad position to16 bytes; engine32-byte vertices pack normals at12..20.
                const auto destination=component<3?component:component-1;
                std::memcpy(output+destination*4,&result,4);
            }
        });
        instance.next=1;instance.current=0;instance.elapsed=0;instance.backward=false;
    }
    void clear() noexcept {while(!instances_.empty())instances_.pop_back();}
    void read(SerializedBytes bytes) {
        clear();const auto version=bytes.word();
        while(bytes.remaining()) {
            auto item=std::make_unique<Instance>();
            item->maximum_lod=signed_word(bytes.word());
            const auto name=bytes.section(128).text();
            item->frame=services_.find_scene_frame(name,0xcffff);
            //Original immediately starts a new record here, without consuming
            //the rest of a rejected visual's record. Do not guess a tail skip.
            if(!item->frame||services_.visual_type(item->frame)!=0x5f4a424fu)continue;
            item->start=signed_word(bytes.word());
            bytes.word(); //Serialized elapsed is explicitly reset by00473790.
            item->duration=signed_word(bytes.word());item->ping_pong=bytes.byte()!=0;
            const auto definition=bytes.section(20).text();
            if(version)item->visibility_required=bytes.word();
            item->definition=find_definition(definition);
            if(item->definition)instances_.emplace_back(std::move(item));
        }
    }
    template<class Files>bool load(Files& files,const std::string& resource) {
        const auto path=std::string("Missions\\")+resource+"\\Vertanim.bin";
        const auto handle=files.open(path.c_str(),0);
        if(handle==0xffffffffu) {files.close(handle);return false;} //Missing preserves instances.
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        clear();const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid vertex-instance size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind vertex instances");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read vertex instances");
        read(SerializedBytes(bytes));return true;
    }
    template<class Files>void finish_loading(bool authority,Files& files,const std::string& resource) {
        if((mode_&2)&&!authority)load(files,resource); //00474AA0.
    }
    void release_loading(bool authority) {if((mode_&2)&&!authority)clear();} //00474AC0: flag is retained.
    void activate() { //00474B40: authority argument is ignored by the original.
        if(!(mode_&2)||definitions_.word0c)return;
        definitions_.word0c=1;
        for(auto& value:instances_) {
            auto& item=*value;item.backward=false;
            if(item.start<0) {
                const auto last=item.definition->last_frame;
                if(last==0||last>0x7fffffffu)throw std::runtime_error("Invalid random vertex frame range");
                if(item.ping_pong&&last==1)item.current=0;
                else {
                    const auto random=services_.vertex_random();
                    if(random<0)throw std::runtime_error("Negative vertex random value");
                    item.current=random%static_cast<std::int32_t>(item.ping_pong?last-1:last);
                }
                item.next=item.current+1;
                if(static_cast<std::uint32_t>(item.next)>=last)item.next=0;
            } else {item.current=0;item.next=1;}
        }
    }
    void deactivate() { //00474C10 ignores authority; clears only if actually activated.
        if((mode_&2)&&definitions_.word0c) {clear();definitions_.word0c=0;}
    }
    void restart() {if(mode_&2){definitions_.word0c=0;activate();}} //00474C90.
    void update(std::int32_t delta) { //00474740's deferred pass.
        if(!(mode_&2)||!services_.vertex_animations_enabled())return;
        clock_=add(clock_,delta);
        if(static_cast<std::uint32_t>(clock_)>=300u) { //0047477F CMP300/JB: UNSIGNED process clock, including wrapped negative deltas.
            clock_=add(clock_,-300); //One subtraction, not modulo.
            const auto camera=services_.active_game_camera();
            if(!camera)throw std::runtime_error("Vertex animation requires the actual camera");
            const auto position=services_.actor_world_matrix(camera);
            const auto projection=services_.vertex_camera_projection(camera);
            const auto scale=static_cast<float>(std::tan(static_cast<long double>(projection[0])*.5L)/projection[1]);
            if(!std::isfinite(scale))throw std::runtime_error("Invalid vertex camera projection");
            for(auto& value:instances_) {
                const auto world=services_.actor_world_matrix(value->frame);
                const auto x=static_cast<long double>(world[12])-position[12];
                const auto y=static_cast<long double>(world[13])-position[13];
                const auto z=static_cast<long double>(world[14])-position[14];
                const float squared=static_cast<float>(z*z+y*y+x*x);
                if(!std::isfinite(squared))throw std::runtime_error("Invalid vertex animation distance");
                const auto distance=squared<1.e-8f?0.L:std::abs(static_cast<long double>(squared)-1)<1.e-8f?
                    1.L:std::sqrt(static_cast<long double>(squared));
                const auto projected=1.7000000476837158L/(2*scale*distance);
                value->deferred=(projected<.019999999552965164L&&value->visibility_required!=0)?1u:0u;
            }
        }
        for(auto& item:instances_)if(item->deferred==1)advance(*item,delta);
    }
    void render_update(std::int32_t delta) { //004749A0's ordinary pass.
        if(!(mode_&2)||!services_.vertex_animations_enabled())return;
        for(auto& item:instances_)if(item->deferred==0)advance(*item,delta);
    }
private:
    static std::int32_t signed_word(std::uint32_t word) {std::int32_t value;std::memcpy(&value,&word,4);return value;}
    static std::int32_t add(std::int32_t a,std::int32_t b) {return signed_word(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));}
    const frontend::VertexAnimation* find_definition(const std::string& name) const {
        for(const auto& value:definitions_.entries()) {
            const auto end=std::find(value->name.begin(),value->name.end(),std::uint8_t{0});
            if(end==value->name.end())throw std::runtime_error("Unterminated vertex definition name");
            const std::string_view candidate(reinterpret_cast<const char*>(value->name.data()),static_cast<std::size_t>(end-value->name.begin()));
            if(candidate==name)return value.get(); //00473E30: ascending first exact match.
        }
        return nullptr;
    }
    void advance(Instance& item,std::int32_t delta) { //00473960, one frame transition per call.
        if(!item.frame||!item.definition||item.finished)return;
        if(item.duration<=0)throw std::runtime_error("Invalid vertex frame duration");
        if(item.definition->last_frame>0x7fffffffu)throw std::runtime_error("Invalid vertex frame range");
        const auto last=static_cast<std::int32_t>(item.definition->last_frame);
        item.elapsed=add(item.elapsed,delta);
        if(item.elapsed>item.duration) {
            item.elapsed=add(item.elapsed,-item.duration);
            if(item.elapsed>item.duration)item.elapsed=0;
            if(!item.ping_pong) {
                item.current=add(item.current,1);
                if(item.current>last) {
                    if(!item.repeat){item.finished=true;item.current=add(item.current,-1);item.elapsed=item.duration;return;}
                    item.current=0;
                }
                item.next=add(item.next,1);if(item.next>last)item.next=0;
            } else if(!item.backward) {
                item.current=add(item.current,1);item.next=add(item.next,1);
                if(item.next>last) {
                    if(!item.repeat){item.current=add(item.current,-1);item.next=add(item.next,-1);item.finished=true;item.elapsed=item.duration;return;}
                    item.current=last;item.next=last-1;item.backward=true;
                }
            } else {
                item.current=add(item.current,-1);item.next=add(item.next,-1);
                if(item.next<0){item.current=0;item.next=1;item.backward=false;}
            }
        }
        const auto epoch=services_.vertex_frame_epoch(item.frame);
        if(epoch!=services_.vertex_scene_epoch()&&item.visibility_required)return;
        const auto lod=services_.vertex_animation_lod(item.frame);
        if(lod>item.maximum_lod)return;
        if(lod<0||static_cast<std::size_t>(lod)>=item.definition->sizes.size()||
           item.current<0||item.next<0||static_cast<std::size_t>(item.current)>=item.definition->frames.size()||
           static_cast<std::size_t>(item.next)>=item.definition->frames.size()||item.duration==0)
            throw std::runtime_error("Invalid vertex interpolation state");
        const auto& current=item.definition->frames[item.current].at(static_cast<std::size_t>(lod));
        const auto& next=item.definition->frames[item.next].at(static_cast<std::size_t>(lod));
        const auto count=item.definition->sizes[static_cast<std::size_t>(lod)];
        if(current.bytes/32<count||next.bytes/32<count)throw std::runtime_error("Short vertex definition block");
        const float fraction=static_cast<float>(static_cast<long double>(item.elapsed)/item.duration);
        const float complement=static_cast<float>(1.L-fraction);
        services_.write_vertex_animation(item.frame,static_cast<std::uint32_t>(lod),count,
            [&](std::size_t index,unsigned char* output) {
                for(unsigned component:{0u,1u,2u,4u,5u,6u}) {
                    float a,b;std::memcpy(&a,current.data.get()+index*32+component*4,4);
                    std::memcpy(&b,next.data.get()+index*32+component*4,4);
                    float result;
                    if(component==0||component==4)
                        result=static_cast<float>(static_cast<long double>(complement)*a+static_cast<long double>(fraction)*b);
                    else {
                        const float first=static_cast<float>(static_cast<long double>(complement)*a);
                        const float second=static_cast<float>(static_cast<long double>(fraction)*b);
                        result=static_cast<float>(static_cast<long double>(first)+second);
                    }
                    //Definitions pad position to16 bytes; engine32-byte vertices pack normals at12..20.
                    const auto destination=component<3?component:component-1;
                    std::memcpy(output+destination*4,&result,4);
                }
            });
    }
    Services& services_;
    frontend::VertexAnimations& definitions_;
    const std::uint32_t& mode_;
    std::int32_t& clock_; //0089B398 persists independently of mission instance cleanup.
    std::vector<Registration> instances_; //File-owned entries and actual actor-owned borrowers share one ordered registry.
};

} //namespace hd2::game
#endif
