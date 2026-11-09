#ifndef HD2_GAME_ACTOR_FRAME_DATA_H
#define HD2_GAME_ACTOR_FRAME_DATA_H

#include <array>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//006E9AD0's eight native words. On the actual i386 host this is32 bytes.
//Word4's sentinel is the native equivalent of00832CDC→00867988 (empty text),
//not an address into the original executable. Other producers must share this
//owner; words1/2/5/6 remain real borrowed native handles, never successful stubs.
struct ActorFrameData {
    inline static constexpr char empty_text[1]={0};
    static std::uintptr_t empty_text_token() noexcept {return reinterpret_cast<std::uintptr_t>(empty_text);}
    std::array<std::uintptr_t,8> words{{0,0,0,0,empty_text_token(),0,0,0}};
    bool unused() const noexcept { //00475C50, deliberately ignores words3/7.
        return !words[0]&&!words[1]&&!words[2]&&words[4]==empty_text_token()&&!words[5]&&!words[6];
    }
};

//Matching native allocator for frame opaque data. Retain each published frame
//until its block is removed; clear opaque pointers before releasing those refs.
//This native lifetime guard adds no original function credit. Existing blocks
//from an unknown allocator reject; they cannot be silently adopted/deleted.
template<class Frames>
class ActorFrameDataStore {
public:
    using Frame=typename Frames::Frame;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    explicit ActorFrameDataStore(Frames& frames) : frames_(frames) {
        static_assert(noexcept(std::declval<Frames&>().actor_opaque_data(std::declval<Frame>(),static_cast<void*>(nullptr))),
                      "Opaque frame-data writes must not throw during cleanup");
        static_assert(noexcept(std::declval<Frames&>().actor_opaque_data(std::declval<Frame>())),
                      "Opaque frame-data reads must not throw during cleanup");
    }
    ~ActorFrameDataStore() {clear();}
    ActorFrameDataStore(const ActorFrameDataStore&)=delete;
    ActorFrameDataStore& operator=(const ActorFrameDataStore&)=delete;
    ActorFrameData& ensure(Frame frame) {
        if(!frame)throw std::logic_error("Actor opaque data requires a frame");
        const auto current=frames_.actor_opaque_data(frame);
        auto found=owners_.find(frame);
        if(found!=owners_.end()) {
            if(current!=&found->second.data)throw std::runtime_error("Actor frame-data ownership changed unexpectedly");
            return found->second.data;
        }
        if(current)throw std::runtime_error("Actor frame has opaque data from an unknown owner");
        auto owner=Owner{frames_.retain_actor_frame(frame),{}};
        auto result=owners_.emplace(frame,std::move(owner));
        auto& data=result.first->second.data;
        frames_.actor_opaque_data(frame,&data);return data;
    }
    void assign(Frame frame,const ActorFrameData& data) {ensure(frame)=data;} //006E9AD0 copy.
    const ActorFrameData* find(Frame frame) const {
        if(!frame)throw std::logic_error("Actor opaque-data lookup requires a frame");
        const auto current=frames_.actor_opaque_data(frame);const auto found=owners_.find(frame);
        if(found==owners_.end()) {
            if(current)throw std::runtime_error("Actor frame has opaque data from an unknown owner");
            return nullptr;
        }
        if(current!=&found->second.data)throw std::runtime_error("Actor frame-data ownership changed unexpectedly");
        return &found->second.data;
    }
    void bind(Frame frame,std::uintptr_t actor) {ensure(frame).words[0]=actor;}
    void unbind(Frame frame) noexcept {
        auto found=owners_.find(frame);
        if(found==owners_.end())return;
        found->second.data.words[0]=0;
        if(found->second.data.unused())erase(found);
    }
    void remove_if_unused(Frame frame) noexcept {
        auto found=owners_.find(frame);
        if(found!=owners_.end()&&found->second.data.unused())erase(found);
    }
    void erase(Frame frame) noexcept { //006E9B20, only matching native allocations.
        auto found=owners_.find(frame);if(found!=owners_.end())erase(found);
    }
    void clear() noexcept {while(!owners_.empty())erase(owners_.begin());}
    std::size_t size() const {return owners_.size();}
private:
    struct Owner {FrameOwner frame;ActorFrameData data;};
    using Owners=std::map<Frame,Owner>;
    void erase(typename Owners::iterator found) noexcept {
        const auto frame=found->first;
        if(frames_.actor_opaque_data(frame)==&found->second.data)frames_.actor_opaque_data(frame,nullptr);
        owners_.erase(found);
    }
    Frames& frames_;
    Owners owners_;
};

} //namespace hd2::game
#endif
