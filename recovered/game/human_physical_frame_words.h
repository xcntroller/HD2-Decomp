#ifndef HD2_GAME_HUMAN_PHYSICAL_FRAME_WORDS_H
#define HD2_GAME_HUMAN_PHYSICAL_FRAME_WORDS_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//DD0+260..603 is the actual animation-track owner embedded in the original
//physical layout. Frame bodies using DD0 offsets must share that owner's words.
//Model/parent/Human pointers at260/264/268 retain their typed representations.
class HumanPhysicalFrameWords {
public:
    using Physical=HumanJointWords<0xaa0>;
    using Tracks=HumanJointWords<0x3a4>;
    explicit HumanPhysicalFrameWords(Physical& physical):physical_(physical){}
    void bind(Tracks& tracks){
        if(tracks_&&tracks_!=&tracks)throw std::logic_error("Human frame words already bind different animation tracks");
        tracks_=&tracks;
    }
    void unbind()noexcept{tracks_=nullptr;}
    template<class Value>Value get(std::size_t offset)const{
        if(in_tracks(offset,sizeof(Value)))return tracks_->template get<Value>(offset-0x260);
        return physical_.template get<Value>(offset);
    }
    template<class Value>void set(std::size_t offset,const Value& value){
        if(in_tracks(offset,sizeof(Value)))tracks_->set(offset-0x260,value);
        else physical_.set(offset,value);
    }
private:
    bool in_tracks(std::size_t offset,std::size_t size)const{
        if(offset>=0x260&&offset<0x604){
            if(offset<0x26c)throw std::logic_error("Human track pointers require their actual typed owners");
            if(size>0x604-offset)throw std::out_of_range("Human frame scalar crosses animation-track ownership");
            if(!tracks_)throw std::logic_error("Human frame scalar requires the actual bound animation tracks");
            return true;
        }
        if(offset<0x260&&size>0x260-offset)throw std::out_of_range("Human frame scalar crosses animation-track ownership");
        return false;
    }
    Physical& physical_;Tracks* tracks_=nullptr;
};

} //namespace hd2::game
#endif
