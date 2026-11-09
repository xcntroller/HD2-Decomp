#ifndef HD2_GAME_SPEECH_DEFINITIONS_H
#define HD2_GAME_SPEECH_DEFINITIONS_H

#include "game/serialized_bytes.h"
#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <utility>

namespace hd2::game {

//005B0340/005AF880's F24-byte native record, represented without pointer/ABI
//padding. Categories start at08, have28-byte native stride, and flag18 is set
//only by an actual phrase producer. Their untouched padding has no value contract.
struct SpeechDefinition {
    struct Category {
        std::vector<std::uint32_t> phrases;
        std::size_t cursor=0;
        std::uint32_t field14=0;
        bool enabled=false;
        template<class Random>void insert(std::uint32_t phrase,Random&& random) { //005B0570.
            if(cursor>phrases.size()||phrases.size()>std::size_t(std::numeric_limits<std::int32_t>::max()-2))
                throw std::runtime_error("Speech phrase window exceeds native bounds");
            const auto value=random();
            if(value>0x7fffu)throw std::runtime_error("Speech insertion requires the shared client CRT random stream");
            const auto index=cursor+value%((phrases.size()-cursor)+2);
            //Two of the random choices append; preserve the current index after
            //vector movement/reallocation, rather than retaining an old pointer.
            if(index<phrases.size())phrases.insert(phrases.begin()+static_cast<std::ptrdiff_t>(index),phrase);
            else phrases.push_back(phrase);
        }
    };
    std::int32_t index=0;
    std::uint32_t field0=0,usage=0;
    std::array<Category,0x8a> categories;
};

//005AFE00 reads only the first root1000 and finds the requested POSITION,
//ignoring each set/phrase chunk's ID. Unknown members/categories skip. The name
//is read but never published by the original; retain no invented name service.
template<class Random>
std::unique_ptr<SpeechDefinition> read_speech_definition(SerializedBytes input,std::int32_t requested,Random&& random) {
    auto root=next_mission_chunk(input);
    if(!root||root->id!=1000||requested<0)return {};
    std::int64_t index=0;
    while(auto entry=next_mission_chunk(root->bytes)) {
        if(index++!=requested)continue;
        std::unique_ptr<SpeechDefinition> result;
        std::uint32_t name_bytes=0;
        while(auto member=next_mission_chunk(entry->bytes)) {
            switch(member->id) {
            case 0:name_bytes=member->bytes.word();break;
            case 1:
                if(name_bytes>128)throw std::runtime_error("Speech name exceeds original stack buffer");
                member->bytes.take(name_bytes);break;
            case 2: {
                auto next=std::make_unique<SpeechDefinition>();next->index=requested;
                while(auto category=next_mission_chunk(member->bytes)) {
                    if(category->id>=next->categories.size())continue;
                    auto& target=next->categories[category->id];
                    while(auto phrase=next_mission_chunk(category->bytes)) {
                        target.enabled=true;
                        const auto value=phrase->bytes.word();
                        target.insert(value,random);
                    }
                }
                result=std::move(next); //Last data member wins; native guard frees a replaced allocation.
                break;
            }
            default:break;
            }
        }
        return result;
    }
    return {};
}

//Same lazy005AFE00/005B0390 owner. Source supplies actual Tables\\speech.set
//bytes and the SAME shared CRT random stream. Destroy actor references before
//this cache, just as the original global speech owner outlives its Human actors.
template<class Source>
class SpeechDefinitionCache {
public:
    explicit SpeechDefinitionCache(Source& source):source_(source) {}
    SpeechDefinitionCache(const SpeechDefinitionCache&)=delete;
    SpeechDefinitionCache& operator=(const SpeechDefinitionCache&)=delete;
    SpeechDefinition* acquire(std::int32_t index) {
        for(const auto& owner:definitions_)if(owner->index==index) {
            if(owner->usage==std::uint32_t(std::numeric_limits<std::int32_t>::max()))
                throw std::runtime_error("Speech reference count overflow");
            ++owner->usage;return owner.get();
        }
        const auto bytes=source_.read_speech_table();if(!bytes)return nullptr;
        auto owner=read_speech_definition(SerializedBytes(*bytes),index,[&]{return source_.speech_random();});
        if(!owner)return nullptr;
        definitions_.reserve(definitions_.size()+1);
        ++owner->usage;auto* definition=owner.get();definitions_.push_back(std::move(owner));return definition;
    }
    void release(SpeechDefinition* definition) noexcept {
        for(std::size_t index=0;index!=definitions_.size();++index)if(definitions_[index].get()==definition) {
            if(definition->usage>1){--definition->usage;return;}
            auto owner=std::move(definitions_[index]);
            if(index+1!=definitions_.size())definitions_[index]=std::move(definitions_.back());
            definitions_.pop_back();owner.reset();return;
        }
        //Native guard: never dereference/free a foreign or already released pointer.
    }
    std::size_t size() const {return definitions_.size();}
private:
    Source& source_;
    std::vector<std::unique_ptr<SpeechDefinition>> definitions_;
};

} //namespace hd2::game
#endif
