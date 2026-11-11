#ifndef HD2_GAME_HUMAN_COLLISION_MARKERS_H
#define HD2_GAME_HUMAN_COLLISION_MARKERS_H

#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

inline constexpr std::array<const char*,9> human_collision_marker_names{{
    "stand1","stand2","stand3","kneel1","kneel2","lie1","lie2","lie3","lie4"
}}; //0084E768.

//Marker lookup portion of0075B280, complete0075C070 and0075BFF0. Borrowed
//kind6 volumes supply real world-space centres and radii. Missing markers stay
//missing; a posture with holes must not be published as a complete collider set.
template<class Frames>
class HumanCollisionMarkers {
public:
    using Frame=typename Frames::Frame;
    using Vector=std::array<float,4>;
    explicit HumanCollisionMarkers(Frames& frames):frames_(frames) {}
    void bind(Frame model) {
        model_=model;
        for(unsigned index=0;index!=markers_.size();++index) {
            const auto frame=frames_.actor_find(model,human_collision_marker_names[index],0xffff);
            markers_[index]=frame&&frames_.frame_kind(frame)==6?frame:Frame{};
        }
    }
    void snapshot() { //0075C070.
        if(!model_)throw std::runtime_error("Human collision markers have no actual model");
        for(unsigned index=0;index!=markers_.size();++index)if(markers_[index]) {
            const auto world=frames_.actor_world_matrix(markers_[index]);
            for(unsigned axis=0;axis!=4;++axis)positions_[index][axis]=world[12+axis];
            const auto model=frames_.actor_world_matrix(model_);
            for(unsigned axis=0;axis!=3;++axis)relative_[index][axis]=positions_[index][axis]-model[12+axis];
            radii_[index]=frames_.human_volume_radius(markers_[index]);
        }
    }
    template<std::size_t Count>unsigned copy_posture(unsigned posture,std::array<Vector,Count>& destination,std::size_t offset=0) const { //0075BFF0.
        const unsigned start=posture==1?3:posture==2?5:0;
        const unsigned count=posture==1?2:posture==2?4:3;
        if(offset>Count||count>Count-offset)throw std::runtime_error("Human posture collision storage exhausted");
        auto present=count;
        for(unsigned index=0;index!=count;++index) {
            if(!markers_[start+index]){--present;continue;}
            auto& sphere=destination[offset+index];
            for(unsigned axis=0;axis!=3;++axis)sphere[axis]=positions_[start+index][axis];
            sphere[3]=radii_[start+index];
        }
        return present;
    }
    void translate(const Vector& position){ //0075C250, relative centres from the actual last snapshot.
        for(unsigned index=0;index!=markers_.size();++index)if(markers_[index])
            for(unsigned axis=0;axis!=3;++axis)positions_[index][axis]=relative_[index][axis]+position[axis];
    }
    const auto& markers() const {return markers_;}
    const auto& positions() const {return positions_;}
    const auto& relative() const {return relative_;}
    const auto& radii() const {return radii_;}
private:
    Frames& frames_;Frame model_{};
    std::array<Frame,9> markers_{};
    alignas(16)std::array<Vector,9> positions_{},relative_{};
    std::array<float,9> radii_{};
};
} //namespace hd2::game
#endif
