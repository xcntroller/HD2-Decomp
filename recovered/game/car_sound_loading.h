#ifndef HD2_GAME_CAR_SOUND_LOADING_H
#define HD2_GAME_CAR_SOUND_LOADING_H
#include "game/car_physics.h"
#include <exception>
#include <iterator>
#include <map>
#include <optional>
#include <utility>
namespace hd2::game {
//0051BD80/00517370 actual per-Car sound frames and ordered loading attempts.
//The shared receiver list must be the process list0089C0B4, not a per-Car copy.
//This does not supply the later playback update or Car activation/settling. Frames/model/
//scene outlive these sound owners; native offsets identify slots, not an ABI.
template<class Frames>class CarSoundLoading {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Owner=decltype(std::declval<Frames&>().create_human_sound());
    struct Slot {std::optional<Owner> owner;Frame get()const{return owner?owner->get():Frame{};}};
    CarSoundLoading(Frames& frames,Frame model,const Vector& head):frames_(frames),model_(model),head_(head){if(!model_)throw std::runtime_error("Car sounds require actual model");}
    ~CarSoundLoading(){try{clear();}catch(...){}}
    CarSoundLoading(const CarSoundLoading&)=delete;CarSoundLoading& operator=(const CarSoundLoading&)=delete;
    Frame sound(unsigned offset)const{const auto at=slots_.find(offset);return at==slots_.end()?Frame{}:at->second.get();}
    const auto& slots()const{return slots_;}
    void clear(){ //0051C180: detach, release and null in the original order. No disable call.
        std::exception_ptr error;
        const auto retire=[&](Slot& slot){
            if(slot.get())try{frames_.human_sound_detach(slot.get());}catch(...){if(!error)error=std::current_exception();}
            slot.owner.reset();
        };
        for(auto offset:slot_order())if(auto at=slots_.find(offset);at!=slots_.end())retire(at->second);
        //Also reclaim a partially acquired standalone helper slot on failure.
        for(auto& entry:slots_)if(entry.second.get())retire(entry.second);
        slots_.clear();if(error)std::rethrow_exception(error);
    }
    void register_receivers(const std::vector<Frame>& receivers){ //00517CF0; original borrows these receivers.
        for(auto receiver=receivers.rbegin();receiver!=receivers.rend();++receiver)
            for(auto offset:slot_order())if(auto frame=sound(offset))frames_.car_sound_register(*receiver,frame);
    }
    void unregister_receivers(const std::vector<Frame>& receivers){ //005181C0, independent of sound retirement.
        for(auto receiver=receivers.rbegin();receiver!=receivers.rend();++receiver)
            for(auto offset:slot_order())if(auto frame=sound(offset))frames_.car_sound_unregister(*receiver,frame);
    }
    void load_slot(unsigned offset,const std::string& filename,std::uint32_t flags,const std::array<float,4>& ranges,bool loop,bool use_head) { //0051BD80, RET36.
        auto& slot=slots_[offset];const auto name=filename.substr(0,filename.find('\0'));
        if(name.empty()) {
            if(slot.get()){frames_.human_sound_detach(slot.get());frames_.human_sound_enabled(slot.get(),false);slot.owner.reset();}
            return;
        }
        if(slot.get()) {
            const auto current=frames_.car_sound_filename(slot.get());
            if(current.size()>=80)throw std::runtime_error("Car sound filename exceeds native reload buffer");
            if(current==name)return; //Native exact case-sensitive comparison also skips changed flags/ranges.
            frames_.human_sound_enabled(slot.get(),false);
        }else{
            slot.owner.emplace(frames_.create_human_sound());if(!slot.get()){slot.owner.reset();return;}
        }
        if(frames_.load_car_sound(slot.get(),name,flags&8u)!=0){slot.owner.reset();return;}
        frames_.human_sound_enabled(slot.get(),false);frames_.human_sound_volume(slot.get(),1.f);frames_.car_sound_pitch(slot.get(),0.f);
        if(loop)frames_.human_sound_parameter90(slot.get(),1); //False does not manufacture a loop-reset call on reload.
        if(flags&3u){
            frames_.human_sound_parameter7c(slot.get(),3);frames_.car_sound_cone(slot.get(),0.f,0.f);
            const auto root=frames_.camera_root_sector();if(!root)throw std::runtime_error("Car sound requires the actual scene root");
            frames_.link_scene_object(slot.get(),root);
        }else{
            frames_.human_sound_parameter7c(slot.get(),1);const auto angle=real(0x40c90fdau);frames_.car_sound_cone(slot.get(),angle,angle);
            frames_.human_sound_range(slot.get(),ranges[0],ranges[1],ranges[2],ranges[3]);frames_.link_scene_object(slot.get(),model_);
        }
        if(use_head)frames_.car_sound_head(slot.get(),head_);
        frames_.update_scene_object(slot.get());
    }
    void load_slots(const CarPhysics& physics) { //00517370, inclusive original call order; attempts optional sound resources.
        if(!physics.applied_parameters())throw std::runtime_error("Car sound loading requires actual applied parameters");
        const auto& p=*physics.applied_parameters();const auto& fields=physics.scalars();
        const std::array<float,4> fixed{{4,50,real(0x3e99999a),real(0x3e4ccccd)}};
        const auto group=[&](unsigned offset){std::array<float,4> out{};for(unsigned n=0;n!=4;++n)out[n]=fields.get(offset+n*4);return out;};
        const auto external=group(0xee8),detail=group(0xf10),general=group(0x95c);
        const auto call=[&](unsigned slot,unsigned name,std::uint32_t flags,const std::array<float,4>& ranges,bool loop,bool head){load_slot(slot,p.name(name/4),flags,ranges,loop,head);};
        for(unsigned n=0;n!=2;++n)call(0x97c+n*4,0x894+n*16,3,fixed,false,true);
        call(0x984,0x8b4,3,fixed,false,true);
        for(unsigned n=0;n!=5;++n)call(0x988+n*4,0x8c4+n*16,11,fixed,true,true);
        for(unsigned n=0;n!=5;++n)call(0x99c+n*4,0x914+n*16,11,fixed,true,true);
        for(unsigned n=0;n!=5;++n)call(0x9b0+n*4,0x964+n*16,11,fixed,true,true);
        call(0x9c4,0x9b4,3,fixed,true,true);
        if(fields.get(0x8d8)>real(0x3f000000))call(0x9c8,0x9c4,11,fixed,true,false);
        call(0x9cc,0x9d4,3,fixed,true,false);call(0x9d0,0x9e4,3,fixed,true,false);call(0x9d4,0x9f4,3,fixed,false,false);
        for(unsigned n=0;n!=2;++n)call(0x9d8+n*4,0xa04+n*16,3,fixed,false,false);
        call(0x9e0,0xa24,11,fixed,true,false);call(0x9e4,0xa34,3,fixed,true,false);call(0x9e8,0xa44,3,fixed,false,false);call(0x9ec,0xa54,3,fixed,false,false);
        for(unsigned n=0;n!=4;++n){call(0x9fc-n*4,0xaa4-n*16,3,fixed,false,false);call(0xa0c-n*4,0xae4-n*16,3,fixed,false,false);}
        call(0xa10,0xaf4,0,fixed,false,false);call(0xa14,0xb04,0,fixed,false,false);
        load_slot(0xa18,"PNEU_PUNC.WAV",11,{{6,30,real(0x3e99999a),real(0x3e4ccccd)}},true,false);
        for(unsigned n=0;n!=2;++n)call(0xc48+n*4,0xd20+n*16,0,external,false,true);
        call(0xc50,0xd40,0,external,false,true);call(0xc54,0xd50,8,external,true,true);
        for(unsigned n=0;n!=5;++n)call(0xc58+n*4,0xd60+n*16,8,external,true,true);
        for(unsigned n=0;n!=5;++n)call(0xc6c+n*4,0xdb0+n*16,8,external,true,true);
        for(unsigned n=0;n!=5;++n)call(0xc80+n*4,0xe00+n*16,8,external,true,true);
        call(0xc94,0xe50,0,external,true,true);
        if(fields.get(0x8d8)>real(0x3f000000))call(0xc98,0xe60,8,detail,true,false);
        call(0xc9c,0xe70,0,general,true,false);call(0xca0,0xe80,0,{{4,110,real(0x3ee66666),real(0x3ebd70a4)}},true,false);call(0xca4,0xe90,0,general,false,false);
        for(unsigned n=0;n!=2;++n)call(0xca8+n*4,0xea0+n*16,0,general,false,false);
        call(0xcb0,0xec0,8,general,true,false);call(0xcb4,0xed0,0,general,true,false);call(0xcb8,0xee0,0,general,false,false);call(0xcbc,0xef0,0,general,false,false);
        for(unsigned n=0;n!=4;++n){call(0xccc-n*4,0xf40-n*16,0,general,false,false);call(0xcdc-n*4,0xf80-n*16,0,general,false,false);}
        call(0xce0,0xf90,0,general,false,false);call(0xce4,0xfa0,0,general,false,false);load_slot(0xce8,"PNEU_PUNC.WAV",8,general,true,false);
    }
private:
    static constexpr std::array<unsigned,81> slot_order(){
        std::array<unsigned,81> out{};unsigned at=0;
        for(unsigned offset=0x97c;offset<=0x9ec;offset+=4)out[at++]=offset;
        for(unsigned n=0;n!=4;++n){out[at++]=0x9fc-n*4;out[at++]=0xa0c-n*4;}
        for(unsigned offset=0xa10;offset<=0xa18;offset+=4)out[at++]=offset;
        for(unsigned offset=0xc48;offset<=0xcbc;offset+=4)out[at++]=offset;
        for(unsigned n=0;n!=4;++n){out[at++]=0xccc-n*4;out[at++]=0xcdc-n*4;}
        for(unsigned offset=0xce0;offset<=0xce8;offset+=4)out[at++]=offset;
        return out;
    }
    static float real(std::uint32_t word){float value;std::memcpy(&value,&word,4);return value;}
    Frames& frames_;Frame model_;Vector head_;std::map<unsigned,Slot> slots_;
};
}
#endif
