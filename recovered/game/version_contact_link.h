#ifndef HD2_GAME_VERSION_CONTACT_LINK_H
#define HD2_GAME_VERSION_CONTACT_LINK_H
#include <cstdint>
#include <functional>
#include <stdexcept>
namespace hd2::game {
//Borrow the containing Versions7 owner after its construction and unbind before
//its retirement. Contacts do not create a second Master registry or frame store.
template<class Frames>class VersionContactLink {
public:
    explicit VersionContactLink(Frames& frames):frames_(frames){}
    template<class Versions>void bind(Versions& versions){
        if(owner_||&versions.frames()!=&frames_)throw std::logic_error("Versions contact requires SAME live frame/module owner");
        std::function<bool(std::uintptr_t,std::int32_t,std::uint32_t)> receiver=[&versions](auto handle,auto damage,auto effects){return versions.damage_master(handle,damage,effects);};
        std::function<bool(std::uintptr_t,typename Frames::Frame,std::int32_t)> frame=[&versions](auto handle,auto picked,auto damage){return versions.damage_frame(handle,picked,damage);};
        receiver_=std::move(receiver);frame_=std::move(frame);owner_=&versions;
    }
    template<class Versions>void unbind(Versions& versions)noexcept{if(owner_==&versions){receiver_={};frame_={};owner_=nullptr;}}
    bool contact(std::uintptr_t handle,std::int32_t damage,std::uint32_t effects){
        if(!owner_)throw std::logic_error("Versions contact has no actual containing module");
        return receiver_(handle,damage,effects);
    }
    bool frame(std::uintptr_t handle,typename Frames::Frame picked,std::int32_t damage){
        if(!owner_)throw std::logic_error("Versions picked contact has no actual containing module");
        return frame_(handle,picked,damage);
    }
private:
    std::function<bool(std::uintptr_t,typename Frames::Frame,std::int32_t)> frame_;
    Frames& frames_;const void* owner_=nullptr;std::function<bool(std::uintptr_t,std::int32_t,std::uint32_t)> receiver_;
};
}
#endif
