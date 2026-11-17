#ifndef HD2_GAME_MISSION_SCRIPT_PAUSE_SOUNDS_H
#define HD2_GAME_MISSION_SCRIPT_PAUSE_SOUNDS_H
#include <string>
#include <vector>
namespace hd2::game {
//Script858/85C/860 borrows existing Scene frames. Finish resumes and forgets
//these references before context/frame retirement; neither add nor clear owns them.
template<class Frames>class MissionScriptPauseSounds {
public:
    explicit MissionScriptPauseSounds(Frames& frames):frames_(frames){}
    void add(const std::string& name){ //00428D10 / AddPauseSound103.
        const auto frame=frames_.find_scene_frame(name,0xffff);if(!frame)return;
        for(auto n=frames_list_.size();n;)if(frames_list_[--n]==frame)return;
        frames_list_.push_back(frame);
    }
    void pause(bool enabled){ //00428F00, requested byte, reverse existing vector.
        for(auto n=frames_list_.size();n;){const auto frame=frames_list_[--n];frames_.object_visible(frame,!enabled);frames_.update_scene_object(frame);}
    }
    void clear()noexcept{frames_list_.clear();} //00427B80 AFTER pause(false).
private:
    Frames& frames_;std::vector<typename Frames::Frame> frames_list_;
};
} //namespace hd2::game
#endif
