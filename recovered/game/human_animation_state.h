#ifndef HD2_GAME_HUMAN_ANIMATION_STATE_H
#define HD2_GAME_HUMAN_ANIMATION_STATE_H

#include "game/human_animation_tracks.h"
#include "game/human_opacity_control.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//DD0+CD0:00757C20/00757DC0/00757DF0/00757E10/00757F50,
//00757FB0 and00758290. The containing owner supplies its actual DD0 scalar
//view, E0 properties, shared animation tables, equipment and module receivers.
//Typed model/sector borrowers stay outside the native scalar pointer slots.
template<class Frames,class Tracks,class Shadow,class Control,class Random>
class HumanAnimationState {
public:
    using Frame=typename Frames::Frame;
    using Vector=HumanVector;
    HumanAnimationState(Frames& frames,Tracks& tracks,Shadow& shadow,Control& control,
            HumanOpacityControl& opacity,Random& random)
        :frames_(frames),tracks_(tracks),shadow_(shadow),control_(control),opacity_(opacity),random_(random) {}
    ~HumanAnimationState(){deactivate();}
    HumanAnimationState(const HumanAnimationState&)=delete;
    HumanAnimationState& operator=(const HumanAnimationState&)=delete;
    void initialize(Frame model) { //00757DC0; native argument to tracks is Human, whose+20 is model.
        if(started_)throw std::logic_error("Human CD0 initialization repeated while live");
        if(!model)throw std::runtime_error("Human CD0 requires its actual model");
        model_=model;started_=true;
        try {
            tracks_.initialize(model_);control_.animation_state_word(0xa9c,0);
            reset();
        }catch(...){deactivate();throw;}
    }
    void activate(Frame model) { //00757E10, including its second track initialization.
        typename Tracks::Primary primary{{45,0,1}};
        typename Tracks::Secondary secondary{{31+(control_.animation_look_word5c()!=0),0}},third{{75,0}};
        initialize(model);
        try {
            tracks_.initialize(model_);
            const auto kind=control_.animation_property_a0();
            if(kind!=1&&kind!=2)throw std::runtime_error("Human activation has no actual animation table");
            //Native compact65C is row135's reference in a12-byte row, not a serialized offset.
            const auto clip=control_.human_animation_resource(static_cast<unsigned>(kind-1),135);
            typename Tracks::Requests requests{};
            requests[0].animation=clip;requests[0].weight=1;requests[0].flags=2;requests[0].blend=100;
            if(clip)frames_.human_clip_word8(clip,2);
            tracks_.select(requests,primary,secondary,third,false);
            tracks_.update(1000,true,&requests);tracks_.update(1000,false);
            words_.set(0x10,std::uint8_t{1});
        }catch(...){deactivate();throw;}
    }
    void reset() { //00757C20.
        require_model();
        words_.set(0x60,std::uint8_t{0});words_.set(0x20,std::int32_t{0});words_.set(0x24,std::int32_t{-1});
        words_.set(0xc,control_.animation_human_flags()&4u);control_.animation_state_word(0x750,0xffffffffu);
        words_.set(0xc4,std::uint32_t{0});words_.set(0xc8,std::uint8_t{0});words_.set(0xd0,std::uint8_t{0});
        for(auto offset:{0x70u,0x74u,0x78u})words_.set(offset,0.f); //7C is unwritten SIMD padding.
        words_.set(0xc0,world_position()[2]);words_.set(0x1c,std::int32_t{-1});words_.set(0xcc,std::uint32_t{0});
        control_.refresh_equipment_visibility();
        words_.set(0x18,static_cast<std::int32_t>(random_()%1000));
        sector_=frames_.human_scene_sector(world_position());
        opacity_.bind_sector(frames_,sector_);opacity_.update(model_,10000,0,control_);
        const auto position=world_position();
        for(auto offset:{0x50u,0x30u,0x80u,0xa0u})words_.set(offset,position);
        set_position(position,true);
    }
    void clear() noexcept { //00757DF0. Shadow closes after tracks, before model/cache.
        if(!started_)return;
        tracks_.clear();shadow_.clear();sector_=model_=Frame{};started_=false;
    }
    void deactivate() noexcept {clear();words_.set(0x10,std::uint8_t{0});} //00757F50.
    void enable_control(bool enabled) { //00757F60;005AA790 is authority mode1 AND stance19C!=1.
        const auto mode=control_.mode_flags();
        words_.set(0xd0,static_cast<std::uint8_t>(((mode&7)!=7&&!control_.animation_authority_actor())||enabled));
    }
    void refresh_motion_gate() { //00757FB0; preserves every C4 bit except bit0.
        bool eligible=true;
        const auto id=state(0x1d8),detail=state(0x1dc),variant=state(0x1e0);
        if(id>=190)eligible=false;
        else {
            switch(id) {
            case 18:if(!detail)eligible=table_value(false,variant)==0;break;
            case 23:if(detail)eligible=table_value(true,static_cast<std::int64_t>(detail)*6+variant)==0;break;
            case 24:eligible=table_value(false,static_cast<std::int64_t>(detail)*6+variant)==0;break;
            case 89:case 90:eligible=false;break; //00816878=.63f.
            case 106:case 107:case 108:
                //00816C24=.504f (stance2/5 multiplies1.25f),00816C28=3.f: all nonzero.
                //The native stance read on106 remains a required actual E0 query.
                if(state(0x274)<1) {if(id==106)(void)control_.animation_look_stance();eligible=false;}
                break;
            default:break;
            }
        }
        eligible=eligible&&state(0x1ac)==1;
        for(auto offset:{0x554u,0x4f4u,0x514u,0x588u,0x534u})eligible=eligible&&state(offset)==-10000;
        words_.set(0xc4,(words_.template get<std::uint32_t>(0xc4)&~1u)|static_cast<unsigned>(eligible));
    }
    void set_position(const Vector& target,bool initial) { //00758290, both branches.
        require_model();refresh_motion_gate();
        if(initial) {
            words_.set(0x24,std::int32_t{100});words_.set(0x20,std::int32_t{0});words_.set(0x60,std::uint8_t{1});
            words_.set(0x30,words_.template get<Vector>(0x50));words_.set(0x50,target);
            words_.set(0x80,world_position());words_.set(0xa0,target);set_delta();return;
        }
        const auto destination=words_.template get<Vector>(0x50),previous=words_.template get<Vector>(0x30);
        const auto remaining=distance(destination,world_position()),span=distance(destination,previous);
        words_.set(0x80,world_position());set_delta();words_.set(0x24,std::int32_t{100});
        for(unsigned axis=0;axis!=3;++axis)words_.set(0xa0+axis*4,
            static_cast<float>(static_cast<long double>(words_.template get<float>(0xb0+axis*4))+destination[axis]));
        //AC/BC are unwritten stack/SIMD padding in the native temporary vectors.
        if(remaining<span)words_.set(0x20,ftol(static_cast<long double>(remaining)/span*100.f));
        else {
            words_.set(0x20,std::int32_t{0});
            if(span>0)words_.set(0x24,ftol(static_cast<long double>(remaining)/span*100.f));
        }
    }
    bool synchronize_authority_position(const Vector& target){ //007599F0, CD0.
        if((control_.mode_flags()&7u)==7u||control_.animation_authority_actor()){
            words_.set(0xc4,words_.template get<std::uint32_t>(0xc4)&~2u);
            set_position(target,true);
        }
        if(!words_.template get<std::uint8_t>(0xd0))return false;
        const auto attachment=state(0xa34);
        if(attachment!=0&&attachment!=1)return false;
        const auto current=world_position();
        const float x=std::abs(current[0]-target[0]),y=std::abs(current[1]-target[1]),z=std::abs(current[2]-target[2]);
        return static_cast<long double>(z)+y+x>=.0005f; //0080FA00.
    }
    void present_position(std::uint32_t elapsed){ //007587F0→007585C0.
        require_model();
        const auto gate=words_.template get<std::uint32_t>(0xc4);
        if(words_.template get<std::uint8_t>(0x60))words_.set(0xc8,std::uint8_t{0});
        else if(gate)words_.set(0xc8,std::uint8_t{1});
        words_.set(0x20,words_.template get<std::uint32_t>(0x20)+elapsed);
        const auto attachment=state(0xa34);
        if(words_.template get<std::uint8_t>(0xd0)||(attachment!=0&&attachment!=1)||!(state(0x18c)&0xff)){
            words_.set(0x20,0u);
            return;
        }
        const auto motion=state(0x1ac);
        if((motion!=1&&(motion<6||motion>9))||words_.template get<std::uint8_t>(0xc8))return;
        words_.set(0x60,std::uint8_t{0});
        const auto timer=words_.template get<std::int32_t>(0x20);
        const auto duration=words_.template get<std::int32_t>(0x24);
        float fraction=1.f;
        if(timer<duration)fraction=static_cast<float>(static_cast<long double>(timer)/duration);
        else if(!gate){
            auto continuation=words_.template get<Vector>(0xa0);
            for(unsigned axis=0;axis!=3;++axis)continuation[axis]+=words_.template get<float>(0xb0+axis*4);
            set_position(continuation,false);
            words_.set(0x20,elapsed);
            fraction=static_cast<float>(static_cast<long double>(human_frame_signed_word(elapsed))/words_.template get<std::int32_t>(0x24));
        }else{
            words_.set(0xc8,std::uint8_t{1});
            words_.set(0x20,0u);
        }
        const auto current=world_position();
        auto target=current;
        const float complement=1.f-fraction;
        for(unsigned axis=0;axis!=3;++axis){
            const float end=fraction*words_.template get<float>(0xa0+axis*4);
            target[axis]=static_cast<float>(static_cast<long double>(complement)*words_.template get<float>(0x80+axis*4)+end);
        }
        const long double x=static_cast<long double>(target[0])-current[0];
        const long double y=static_cast<long double>(target[1])-current[1];
        const long double z=static_cast<long double>(target[2])-current[2];
        if((x*x+y*y)+z*z>1.e-7f)frames_.human_control_set_position(model_,target);
    }
    const auto& words() const{return words_;}
    Frame sector() const{return sector_;}
    bool active() const{return words_.template get<std::uint8_t>(0x10)!=0;}
private:
    void require_model() const {if(!started_||!model_)throw std::logic_error("Human CD0 has no model");}
    std::int32_t state(unsigned offset) const {
        const auto bits=control_.animation_state_word(offset);std::int32_t value;std::memcpy(&value,&bits,4);return value;
    }
    Vector world_position() {const auto matrix=frames_.actor_world_matrix(model_);return {{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    void set_delta() {
        for(unsigned axis=0;axis!=3;++axis)words_.set(0xb0+axis*4,
            static_cast<float>(static_cast<long double>(words_.template get<float>(0x50+axis*4))-words_.template get<float>(0x30+axis*4)));
    }
    static float distance(const Vector& first,const Vector& second) {
        const auto x=static_cast<long double>(first[0])-second[0],y=static_cast<long double>(first[1])-second[1],z=static_cast<long double>(first[2])-second[2];
        const float square=static_cast<float>((z*z+y*y)+x*x);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return 0;
        return std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
    }
    static std::int32_t ftol(long double value) {
        if(!std::isfinite(value)||value<std::numeric_limits<std::int64_t>::min()||value>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))
            throw std::runtime_error("Human CD0 __ftol exceeds original integer range");
        const auto bits=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
        std::int32_t result;std::memcpy(&result,&bits,4);return result;
    }
    static float table_value(bool second,std::int64_t index) {
        //Read-only original DWORD slices00816AB8/00816B48, fingerprinted in
        //human-animation-motion-data.json. Reject reads outside recovered slices.
        static constexpr std::array<float,36> first{{0,1.8f,3.5f,0,5,6,0,0,0,0,2,0}};
        static constexpr std::array<float,36> other{{0,0,0,0,0,0,0,1.5f,2.5f,0,1.5f,2.5f,0,0,0,0,0,0,
            .7f,.7f,1.4f,.7f,.7f,1.4f,.7f,.7f,1.4f,.7f,.7f,1.4f,.7f,.7f,1.4f,.7f,.7f,1.4f}};
        if(index<0||index>=36)throw std::runtime_error("Human animation motion table index is outside recovered data");
        return (second?other:first)[static_cast<unsigned>(index)];
    }
    Frames& frames_;Tracks& tracks_;Shadow& shadow_;Control& control_;HumanOpacityControl& opacity_;Random& random_;
    HumanJointWords<0xd4> words_;Frame model_{},sector_{};bool started_=false;
};
} //namespace hd2::game
#endif
