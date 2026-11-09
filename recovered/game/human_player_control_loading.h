#ifndef HD2_GAME_HUMAN_PLAYER_CONTROL_LOADING_H
#define HD2_GAME_HUMAN_PLAYER_CONTROL_LOADING_H

#include "game/human_player_roster.h"
#include <array>
#include <cstdint>
#include <type_traits>

namespace hd2::game {

//Module11's actual control at0089AD68+1C. Same roster08, selected/current/
//previous28/2C/30, override58, selection38, per-Human collected frames60 and
//Dog listC0. Presentation supplies required actual FPV/HUD/camera endpoints.
//No transport, populated order channels or containing module activation claim.
template<class Frames,class Human,class Presentation>class HumanPlayerControlLoading {
public:
    using Frame=typename Frames::Frame;
    using Actor=typename std::decay_t<decltype(std::declval<Human&>().perception.lists()[0])>::value_type;
    struct CollectedFrames {std::uint8_t allocator=0;std::vector<Frame> frames;};
    HumanPlayerControlLoading(Frames& frames,Presentation& presentation):frames_(frames),presentation_(presentation){} //0045D1F0 known writes.
    ~HumanPlayerControlLoading(){clear();} //0045D2E0; no actor/frame release from these borrowed lists.
    HumanPlayerControlLoading(const HumanPlayerControlLoading&)=delete;
    HumanPlayerControlLoading& operator=(const HumanPlayerControlLoading&)=delete;
    void clear()noexcept { //0045D3A0, retains containers and selection.changed10.
        roster_.clear();index28_=index2c_=index30_=-1;override58_=nullptr;selection_.clear();
        while(!collected_.empty())collected_.pop_back();
        dogs_.clear();field34_=0;
    }
    Human* current_player()const { //0045D1D0, distinct from selected28 and previous30.
        if(override58_)return override58_;
        return indexed(index2c_);
    }
    Human* selected_player(std::uint32_t mode) { //0045D160, callback may mutate these SAME fields.
        return selected_control_player(roster_,index28_,mode,[&](Human& human){return presentation_.human_selected_suppressed(human);});
    }
    Human* previous_player()const{return previous_control_player(roster_,index30_);} //0045D1B0.
    void select_human(Human& human) { //0045DA60, no change when not in actual roster.
        const auto previous=index2c_,next=roster_.index(&human);if(next==-1)return;
        index2c_=index28_=next;
        if(next!=previous)presentation_.reset_player_camera(0); //00464E90 virtual40.
        index30_=-1;
    }
    bool add_human(Human* human) { //0045E5D0; FPV/HUD still run for rejected duplicates.
        const bool appended=roster_.append(human);const auto count=roster_.size();
        if(count==1) {
            index2c_=0;presentation_.bind_first_person_player(); //004922A0 reads SAME current2C.
            presentation_.refresh_first_person_visibility(); //0045DAC0.
        }
        presentation_.hud_add_human(human,static_cast<std::int32_t>(count)-1); //0049B7E0 even when append refused.
        if(appended)collect_frames(static_cast<std::int32_t>(count)-1);
        return appended;
    }
    bool remove_human(Human* human) { //0045E670, swap-pop BOTH roster and its corresponding collected-frame record.
        const auto old_last=roster_.size()?roster_.at(static_cast<std::int32_t>(roster_.size()-1)):nullptr;
        std::int32_t removed=-1;if(!roster_.remove(human,&removed))return false;
        presentation_.hud_remove_human(human,removed); //0049B870, before current/selected changes.
        if(index2c_==removed) {
            //Native reads the old vector's still-allocated last element when
            //removing the last/current Human. Preserve that borrower, no OOB read.
            auto* replacement=static_cast<std::size_t>(removed)<roster_.size()?roster_.at(removed):old_last;
            if(!replacement)throw std::runtime_error("Removed current Human lost its actual frame borrower");
            frames_.object_visible(replacement->base().frame(),true);index2c_=-1;
        }else if(static_cast<std::uint32_t>(index2c_)>=roster_.size())index2c_=-1;
        if(index2c_==-1&&roster_.size())index2c_=0;
        if(index28_==removed)index28_=-1;
        if(index30_==removed)index30_=-1;
        if(removed<0||static_cast<std::size_t>(removed)>=collected_.size())
            throw std::runtime_error("Human removal requires its actual collected-frame owner");
        auto old=std::move(collected_[static_cast<std::size_t>(removed)]);
        old.reset(); //0045D480 destroys frame vector BEFORE moving last record into its slot.
        if(static_cast<std::size_t>(removed)+1!=collected_.size())collected_[static_cast<std::size_t>(removed)]=std::move(collected_.back());
        collected_.pop_back();return true;
    }
    void collect_frames(std::int32_t index) { //0045D710 /0045D7C0 enumerate ALL kind-mask400 frames, no name filter.
        if(index<0||static_cast<std::size_t>(index)>collected_.size())throw std::runtime_error("Human frame collection index has no native slot");
        if(static_cast<std::size_t>(index)==collected_.size()) {
            auto record=std::make_unique<CollectedFrames>();record->allocator=static_cast<std::uint8_t>(index);
            collected_.push_back(std::move(record));
        }
        auto* human=roster_.at(index);if(!human)throw std::runtime_error("Human frame collection requires actual roster member");
        auto& record=*collected_[static_cast<std::size_t>(index)];record.frames.clear();
        frames_.enumerate_human_control_frames(human->base().frame(),[&](Frame frame){record.frames.push_back(frame);});
    }
    void add_sighted_dog(Actor actor) { //0045D4A0, reverse identity check then append, NULL is not specially rejected.
        for(auto n=dogs_.size();n;)if(dogs_[--n]==actor)return;
        dogs_.push_back(actor);
    }
    void post_loading() { //0045DDE0 after base00461D30 genuine RET4.
        index28_=index30_=-1;if(roster_.size()==1)index2c_=0;override58_=nullptr;
        selection_.clear();dogs_.clear();field34_=0; //Collected60/pending6C remain intact.
    }
    const auto& human_roster()const{return roster_;}auto& human_roster(){return roster_;}
    auto& selection(){return selection_;}const auto& collected_frames()const{return collected_;}const auto& sighted_dogs()const{return dogs_;}
    std::int32_t& selected_index(){return index28_;}std::int32_t& current_index(){return index2c_;}std::int32_t& previous_index(){return index30_;}
    Human*& override_player(){return override58_;} //Typed borrowed native58, actual writers remain containing consumers.
private:
    Human* indexed(std::int32_t index)const {
        if(index==-1)return nullptr;
        auto* human=roster_.at(index);if(!human)throw std::runtime_error("Current Human index exceeds actual control roster");return human;
    }
    Frames& frames_;Presentation& presentation_;HumanRoster<Human> roster_;HumanSelectionRecords<Human> selection_;
    std::int32_t index28_=-1,index2c_=-1,index30_=-1;Human* override58_=nullptr;std::uint32_t field34_=0;
    std::vector<std::unique_ptr<CollectedFrames>> collected_;std::vector<Actor> dogs_;
};
} //namespace hd2::game
#endif
