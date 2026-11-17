#ifndef HD2_GAME_DOG_CLIP_MIXER_H
#define HD2_GAME_DOG_CLIP_MIXER_H
#include "game/dog_body_actions.h"
#include "game/human_animation_requests.h"
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
template<class Frame>struct DogClipRequest {Frame clip{};float stride=0,weight=0,blend=0;bool separate=false;};
template<class Frame>class DogClipLifetime {
public:
    using Requests=std::array<DogClipRequest<Frame>,2>;using Vector=std::array<float,4>;
    virtual ~DogClipLifetime()=default;
    virtual void initialize(Frame)=0;
    virtual void bind(const Requests&)=0;
    virtual std::int32_t time(unsigned)=0;
    virtual std::int32_t duration(unsigned)const=0;
    virtual void rate(unsigned,float)=0;
    virtual void tick(std::int32_t,std::int32_t,bool,const Requests*)=0;
    virtual void event(const char*)=0;
    virtual void advance_motion(std::int32_t,const Vector&)=0;
    virtual void decelerate(const Vector&,std::int32_t)=0;
    virtual const Vector& movement()const=0;
};
//005C6D10/6E10/7430/6EE0. Clip base is Actor+A0=Body+10. Model10 is
//the SAME ActorB0 slot. Two group records alternate engine slots0/1 and2/3;
//the four blend records are independent of the Body action queue.
template<class Frames,class Callbacks>
class DogClipMixer final : public DogClipLifetime<typename Frames::Frame> {
public:
    using Frame=typename Frames::Frame;using Request=DogClipRequest<Frame>;using Requests=std::array<Request,2>;using Vector=std::array<float,4>;
    struct Group {
        std::int32_t slot=-1,time=-1,lock=0;
        std::array<std::optional<std::uint32_t>,3> events;
        std::int32_t event_count=0,duration=0;bool paused=false;
    };
    struct Blend {float weight=0,duration=0,elapsed=-10000;bool separate=true;std::uint32_t flags=0;float rate=1;bool flag18=false;std::int32_t key=-1;};
    DogClipMixer(Frames& frames,Callbacks& callbacks,DogClipForcedMotion& forced):frames_(frames),callbacks_(callbacks),forced_(forced){}
    ~DogClipMixer()override{clear();}
    void initialize(Frame model)override{ //005C6E10 stores10 before005C6D10.
        if(!model)throw std::runtime_error("Dog clip initialization requires actual model");
        if(model_&&model_!=model)throw std::runtime_error("Dog clip model replacement requires actual retirement");
        model_=model;
        for(auto& group:groups_){group.slot=group.time=-1;group.lock=group.event_count=group.duration=0;group.paused=false;}
        for(auto& blend:blends_)blend=Blend{};
        const auto marker=frames_.actor_find(model_,"a",0xffffu);
        if(marker){callbacks_.install_animation_events(marker,*this);marker_=marker;}
        fraction_=old_stride_=stride_=0;for(unsigned n=0;n!=3;++n)movement_[n]=0;changed_=false;
        forced_.timer114=static_cast<std::uint32_t>(-10000);deceleration_timer_=-10000;
        initialized_=true;
    }
    void clear()noexcept{
        if(marker_)try{callbacks_.remove_animation_events(marker_,*this);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        marker_=Frame{};model_=Frame{};initialized_=false;
    }
    const auto& groups()const{return groups_;}const auto& blends()const{return blends_;}
    auto& group(unsigned index){require();return groups_.at(index);}auto& blend(unsigned index){require();return blends_.at(index);}
    bool changed()const{return changed_;}
    void bind(const Requests& requests)override{ //005C7430, caller's two five-word descriptors.
        require();
        for(unsigned n=0;n!=2;++n){const auto& request=requests[n];if(!request.clip)continue;
            auto& g=groups_[n];g.lock=g.time=0;
            if(g.slot==-1||g.slot==0){
                g.slot=1;g.duration=frames_.human_clip_duration(request.clip);g.paused=false;
                frames_.human_model_animation(model_,request.clip,n*2+1,0.f,0u);
                set_blend(blends_[n*2+1],request);
            }else{
                g.slot=0;g.duration=frames_.human_clip_duration(request.clip);g.paused=false;
                frames_.human_model_animation(model_,request.clip,n*2,request.separate?0.f:request.weight,0u);
                set_blend(blends_[n*2],request);
            }
            old_stride_=stride_;stride_=request.stride;
        }
    }
    std::int32_t time(unsigned index)override{ //005C7850: actual modelAC before cached18.
        require();const auto& g=groups_.at(index);
        if(g.slot!=-1&&frames_.human_model_animation_enabled(model_,slot(index,g.slot)))return g.time;
        return -1;
    }
    std::int32_t duration(unsigned index)const override{require();const auto& g=groups_.at(index);return g.slot==-1?0:g.duration;} //005C7890.
    void rate(unsigned index,float value)override{require();const auto& g=groups_.at(index);if(g.slot!=-1)blends_.at(slot(index,g.slot)).rate=value;} //005C7930.
    void pause(unsigned index,std::int32_t lock){require();auto& g=groups_.at(index);if(!g.paused){blends_.at(slot(index,g.slot)).rate=0;g.lock=lock;}} //005C7A70, absent frozen.
    void resume(unsigned index){require();auto& g=groups_.at(index);if(g.lock){g.lock=-1;blends_.at(slot(index,g.slot)).rate=1;}} //005C7AB0, absent frozen.
    std::int32_t take_event_count(unsigned index){require();auto& g=groups_.at(index);const auto result=g.event_count;g.event_count=0;return result;} //005C7AE0.
    long double fraction(unsigned index){ //005C7B00, unlike tick its inactive-negative path returns1.
        require();fraction_=0;const auto& g=groups_.at(index);if(g.slot==-1)return fraction_;
        const auto& b=blends_.at(slot(index,g.slot));if(b.elapsed<0)return 1.L;
        if(b.elapsed>=b.duration){fraction_=1;return fraction_;}
        const auto ratio=blend_ratio(b.elapsed,0,b.duration);if(index==0)fraction_=static_cast<float>(ratio);
        return fraction_;
    }
    //The native first elapsed argument is unused. The SECOND signed argument
    //drives both the engine eight-slot advance and the two blend groups.
    void tick(std::int32_t,std::int32_t elapsed,bool weights_only,const Requests* mask)override{ //005C6EE0.
        require();fraction_=0;if(elapsed==-1&&!weights_only)return;
        if(!weights_only){
            std::array<std::int32_t,8> times{};const auto unsigned_elapsed=static_cast<std::uint32_t>(elapsed);
            for(unsigned n=0;n!=8;++n)times[n]=n<4?human_animation_time_integer(static_cast<long double>(unsigned_elapsed)*blends_[n].rate):elapsed;
            frames_.human_model_advance(model_,times);
        }
        for(unsigned n=0;n!=2;++n){if(weights_only&&mask&&!(*mask)[n].clip)continue;
            auto& g=groups_[n];if(g.slot==-1)continue;
            const auto current=slot(n,g.slot),other=n*2+static_cast<unsigned>(g.slot==0);auto& b=blends_[current];
            if(g.lock<1){g.time=wrap(g.time,elapsed);if(g.time>0x7fffd8ef)g.time=100000;}
            if(b.elapsed<0)continue; //Native unordered elapsed proceeds, not an ordered>=0 test.
            const float stored_step=static_cast<float>(elapsed);
            const auto sum=static_cast<long double>(elapsed)+b.elapsed;
            b.elapsed=static_cast<float>(sum);
            if(!(sum<b.duration||std::isunordered(sum,static_cast<long double>(b.duration)))){
                if(b.flags&1u)g.slot=-1;else frames_.human_model_weight(model_,current,b.weight);
                frames_.human_model_remove_animation(model_,other);
                frames_.human_model_animation(model_,Frame{},other,1.f,0u);
                changed_=true;auto& previous=blends_[other];previous.elapsed=-10000;previous.flags=0;previous.rate=1;previous.flag18=true;
                b.elapsed=-10000;
                if(b.flags&1u){frames_.human_model_remove_animation(model_,current);b.flags=0;b.rate=1;b.flag18=true;}
                continue;
            }
            const float ratio=static_cast<float>(blend_ratio(b.elapsed,stored_step,b.duration));
            if(b.flags&1u){
                const float value=static_cast<float>((1.L-ratio)*b.weight);frames_.human_model_weight(model_,current,value);
                if(frames_.dog_model_weight(model_,other)>=value)frames_.human_model_weight(model_,other,value);
            }else if(b.duration>0){
                if(b.separate){frames_.human_model_weight(model_,other,static_cast<float>((1.L-ratio)*blends_[other].weight));frames_.human_model_weight(model_,current,static_cast<float>(static_cast<long double>(ratio)*b.weight));}
                else if(current<other)frames_.human_model_weight(model_,other,static_cast<float>((1.L-ratio)*blends_[other].weight));
                else frames_.human_model_weight(model_,current,static_cast<float>(static_cast<long double>(ratio)*b.weight));
            }
        }
    }
    const Vector& movement()const override{require();return movement_;} //005C75B0, XYZ meaningful; native W is unwritten.
    const auto& deceleration_velocity()const{return deceleration_velocity_;}
    const auto& deceleration_reduction()const{return deceleration_reduction_;}
    std::int32_t deceleration_timer()const{return deceleration_timer_;}
    const auto& deceleration_accumulator()const{return deceleration_accumulator_;}
    const auto& deceleration_elapsed()const{return deceleration_elapsed_;}
    void decelerate(const Vector& input,std::int32_t duration)override{ //005C7600.
        require();deceleration_accumulator_=0;deceleration_velocity_=input;
        deceleration_elapsed_=0;deceleration_timer_=duration;deceleration_reduction_=input;
        auto& reduction=*deceleration_reduction_;
        normalize_human_ai_shot(reduction,stored_square_xyz(reduction)); //Native normalization compares the FLOAT square.
        const float square=stored_square_xyz(input);std::uint32_t bits;std::memcpy(&bits,&square,4);
        const auto length=dog_order_signed(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        const auto factor=length/(static_cast<long double>(duration)/80.f);
        for(unsigned n=0;n!=3;++n)reduction[n]=static_cast<float>(factor*reduction[n]);
    }
    void advance_motion(std::int32_t elapsed,const Vector& direction)override{ //005C71E0, no engine call or synthetic motion.
        require();const float seconds=static_cast<float>(static_cast<long double>(elapsed)*.001f);
        const auto blend=fraction(0);long double current=stride_;const auto lane=groups_[0].slot;
        if(lane!=-1)current*=blends_.at(slot(0,lane)).rate;
        const auto speed=current*blend+(1.L-blend)*old_stride_;
        movement_[0]=static_cast<float>(speed*direction[0]*seconds);
        for(unsigned n=1;n!=3;++n){const float component=static_cast<float>(speed*direction[n]);movement_[n]=static_cast<float>(static_cast<long double>(component)*seconds);}
        const auto duration=dog_order_signed(required(forced_.timer114,"Dog motion requires actual clip114 duration producer"));
        if(duration!=-10000){
            const auto old=dog_order_signed(required(forced_.field110,"Dog forced motion requires actual clip110 counter producer"));
            const auto next=wrap(old,elapsed);forced_.field110=static_cast<std::uint32_t>(next);auto active=elapsed;
            if(duration<next){forced_.timer114=static_cast<std::uint32_t>(-10000);active=wrap(wrap(duration,-static_cast<std::uint32_t>(next)),elapsed);}
            const auto ratio=static_cast<long double>(active)/duration;
            const auto& vector=required_reference(forced_.direction100,"Dog forced motion requires actual clip100 vector producer");
            add_scaled_motion(vector,ratio); //X remains extended through add; Y/Z products spill first.
        }
        if(deceleration_timer_!=-10000){
            const auto old=required(deceleration_elapsed_,"Dog deceleration requires actual clip144 elapsed producer");auto active=elapsed;
            if(wrap(old,elapsed)>=deceleration_timer_){active=wrap(deceleration_timer_,-static_cast<std::uint32_t>(old));deceleration_timer_=-10000;}
            deceleration_elapsed_=wrap(old,active);auto remaining=active;
            while(remaining>0){
                const auto accumulated=required(deceleration_accumulator_,"Dog deceleration requires actual clip140 accumulator producer");
                if(wrap(accumulated,remaining)<=80){deceleration_accumulator_=wrap(accumulated,remaining);break;}
                auto& velocity=required_reference(deceleration_velocity_,"Dog deceleration requires actual clip120 velocity producer");
                const auto& reduction=required_reference(deceleration_reduction_,"Dog deceleration requires actual clip130 reduction producer");
                for(unsigned n=0;n!=3;++n)velocity[n]=static_cast<float>(static_cast<long double>(velocity[n])-reduction[n]);
                remaining=human_animation_time_integer(static_cast<long double>(remaining)-(80.f-static_cast<long double>(accumulated)));
                deceleration_accumulator_=0;
            }
            const auto& velocity=required_reference(deceleration_velocity_,"Dog deceleration requires actual clip120 velocity producer");
            add_scaled_motion(velocity,static_cast<long double>(active)*.001f); //No FLOAT seconds spill on this native branch.
        }
    }
    //005C7960 bridge resolves actual parentEC/Actor before this005C79B0
    //receiver. Only the text argument is read. Three event slots per group;
    //native counter-overwriting/out-of-array states remain explicit failures.
    void event(const char* text)override{
        if(!text)return;
        if(!model_)throw std::runtime_error("Dog animation callback requires actual clip model producer");
        if(text[0]=='s'){
            auto& g=groups_[0];if(g.event_count>0){bool zero=false;for(std::int32_t n=0;n<g.event_count;++n){const auto& value=g.events.at(static_cast<unsigned>(n));if(!value)throw std::runtime_error("Dog event reads unproduced native event word");zero=zero||*value==0;}if(zero)return;}
            append_event(g,0);
        }else if(text[0]=='m')append_event(groups_[1],3);
        else if(text[0]=='a'&&text[1]=='1')append_event(groups_[1],1);
        else if(text[0]=='a'&&text[1]=='2')append_event(groups_[1],2);
    }
private:
    void require()const{if(!initialized_||!model_)throw std::runtime_error("Dog clips require actual005C6E10 model/reset producer");}
    static unsigned slot(unsigned group,std::int32_t lane){if(group>=2||lane<0||lane>1)throw std::runtime_error("Dog clip reaches native invalid engine slot");return group*2+static_cast<unsigned>(lane);}
    static std::int32_t wrap(std::int32_t a,std::uint32_t b){return dog_order_signed(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));}
    static long double blend_ratio(float elapsed,float step,float duration){const auto denominator=static_cast<long double>(duration)*.01f;const auto previous=(static_cast<long double>(elapsed)-step)/denominator;return ((static_cast<long double>(elapsed)/denominator-previous)*.5f+previous)*.01f;} //007724A0, existing source helper.
    static void set_blend(Blend& blend,const Request& r){blend.weight=r.weight;blend.duration=r.blend;blend.separate=r.separate;blend.elapsed=0;blend.flags=0;blend.rate=1;}
    static void append_event(Group& group,std::uint32_t value){if(group.event_count<0||group.event_count>=3)throw std::runtime_error("Dog animation event reaches native counter/array overlap");group.events[static_cast<unsigned>(group.event_count)]=value;++group.event_count;}
    Frames& frames_;Callbacks& callbacks_;DogClipForcedMotion& forced_;
    Frame model_{},marker_{};bool initialized_=false,changed_=false;
    std::array<Group,2> groups_;std::array<Blend,4> blends_;
    float fraction_=0,old_stride_=0,stride_=0;
    static float stored_square_xyz(const Vector& v){return static_cast<float>((static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2]);}
    template<class T>static T required(const std::optional<T>& v,const char* message){if(!v)throw std::runtime_error(message);return *v;}
    template<class T>static T& required_reference(std::optional<T>& v,const char* message){if(!v)throw std::runtime_error(message);return *v;}
    void add_scaled_motion(const Vector& vector,long double factor){movement_[0]=static_cast<float>(factor*vector[0]+movement_[0]);for(unsigned n=1;n!=3;++n){const float component=static_cast<float>(factor*vector[n]);movement_[n]=static_cast<float>(static_cast<long double>(component)+movement_[n]);}}
    Vector movement_{};std::int32_t deceleration_timer_=-10000;
    std::optional<Vector> deceleration_velocity_,deceleration_reduction_;
    std::optional<std::int32_t> deceleration_accumulator_,deceleration_elapsed_;
};
}
#endif
