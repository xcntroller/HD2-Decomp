#ifndef HD2_GAME_HUMAN_CLIP_POSE_H
#define HD2_GAME_HUMAN_CLIP_POSE_H

#include "game/human_skeleton.h"
#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"
#include <limits>

namespace hd2::game {

inline bool human_clip_missing_key(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7fffffffu) == 0x7fc00000u;
}

struct HumanClipKeyPair {
    HumanVector previous{}, next{};
    std::uint32_t previous_time = 0, next_time = 0;
};

//007B02B0 repeats the count queries while advancing each actual bone cursor.
//Only the original positive/negative quiet-NaN bit patterns mark absent keys.
template<class Count, class Read>
HumanClipKeyPair sample_human_clip_keys(std::uint32_t& cursor, std::int32_t time,
        unsigned sentinel_axis, Count&& count, Read&& read) {
    HumanClipKeyPair keys;
    for (;;) {
        keys.previous[sentinel_axis] = std::numeric_limits<float>::quiet_NaN();
        keys.next[sentinel_axis] = std::numeric_limits<float>::quiet_NaN();
        if (human_frame_signed_word(cursor) < count())
            read(human_frame_signed_word(cursor), keys.previous_time, keys.previous);
        if (human_frame_signed_word(cursor + 1u) < count())
            read(human_frame_signed_word(cursor + 1u), keys.next_time, keys.next);
        if (human_clip_missing_key(keys.next[sentinel_axis])) {
            keys.next = keys.previous;
            keys.next_time = keys.previous_time;
            return keys;
        }
        if (time <= human_frame_signed_word(keys.next_time)) return keys;
        ++cursor;
    }
}

inline HumanVector human_clip_position_target(const HumanClipKeyPair& keys,std::int32_t time){
    auto target=keys.previous;
    const auto duration=human_frame_signed_word(keys.next_time-keys.previous_time);
    if(duration>0){
        const auto fraction=static_cast<long double>(human_frame_signed_word(
            static_cast<std::uint32_t>(time)-keys.previous_time))/duration;
        for(unsigned axis=0;axis!=3;++axis){
            const float previous=static_cast<float>(keys.previous[axis]*(1.L-fraction));
            const auto next=keys.next[axis]*fraction;
            target[axis]=static_cast<float>(previous+(axis?static_cast<float>(next):next));
        }
    }
    return target;
}

inline float human_secondary_clip_bone_weight(int bone) { //007AFD10, absent frozen inventory.
    switch (bone) {
    case 7: case 8: case 12: case 13: return .5f;
    case 9: return .2f;
    default: return 1.f;
    }
}

//007B02B0 traverses the retained skeleton in child0, child1, child2 order.
//Key cursors belong to the geometry. Engine frames supply actual local pose,
//quaternion interpolation/cache publication and normalization.
template<class Frames, class Weight>
void apply_human_clip_pose(Frames& frames, const HumanSkeleton<typename Frames::Frame>& skeleton,
        typename Frames::Frame clip, std::array<std::uint32_t, 23>& positions,
        std::array<std::uint32_t, 23>& rotations, std::int32_t time,
        int bone_index, float weight, Weight& bone_weight) {
    const auto& bone = skeleton.bones().at(static_cast<std::size_t>(bone_index));
    if (bone.index < 0) throw std::logic_error("Human clip pose requires its actual skeleton node");
    const auto index = frames.human_clip_bone_index(clip, human_bone_names[bone.index]);
    if (index >= 0) {
        const auto position = sample_human_clip_keys(positions.at(bone.index), time, 0,
            [&] { return frames.human_clip_position_count(clip, index); },
            [&](std::int32_t key, std::uint32_t& key_time, HumanVector& value) {
                frames.human_clip_position_key(clip, index, key, key_time, value);
            });
        const auto rotation = sample_human_clip_keys(rotations.at(bone.index), time, 3,
            [&] { return frames.human_clip_rotation_count(clip, index); },
            [&](std::int32_t key, std::uint32_t& key_time, HumanVector& value) {
                frames.human_clip_rotation_key(clip, index, key, key_time, value);
            });
        const float influence = static_cast<float>(bone_weight(bone.index) * static_cast<long double>(weight));
        if (!human_clip_missing_key(position.previous[0]) && !human_clip_missing_key(position.next[0])) {
            frames.human_clip_blend_position(bone.frame,human_clip_position_target(position,time),influence);
        }
        if (!human_clip_missing_key(rotation.previous[3]) && !human_clip_missing_key(rotation.next[3])) {
            auto target = rotation.previous;
            const auto duration = human_frame_signed_word(rotation.next_time - rotation.previous_time);
            if (duration > 0) {
                const float fraction = static_cast<float>(static_cast<long double>(human_frame_signed_word(
                    static_cast<std::uint32_t>(time) - rotation.previous_time)) / duration);
                target = frames.physics_slerp(rotation.previous, rotation.next, fraction, false);
            }
            frames.human_clip_blend_rotation(bone.frame, target, influence);
        }
    }
    for (auto child : bone.children)
        if (child >= 0) apply_human_clip_pose(frames, skeleton, clip, positions, rotations, time, child, weight, bone_weight);
}

//007B7410 publishes the retained reaction clip directly. Its native factor is
//only forwarded recursively, so no influence blend belongs to this worker.
template<class Frames>
void apply_human_ragdoll_clip_pose(Frames& frames,const HumanSkeleton<typename Frames::Frame>& skeleton,
        typename Frames::Frame clip,std::array<std::uint32_t,23>& positions,
        std::array<std::uint32_t,23>& rotations,std::int32_t time,int bone_index){
    const auto& bone=skeleton.bones().at(static_cast<std::size_t>(bone_index));
    if(bone.index<0)throw std::logic_error("Human ragdoll clip requires its actual skeleton node");
    const auto index=frames.human_clip_bone_index(clip,human_bone_names[bone.index]);
    if(index>=0){
        const auto position=sample_human_clip_keys(positions.at(bone.index),time,0,
            [&]{return frames.human_clip_position_count(clip,index);},
            [&](std::int32_t key,std::uint32_t& key_time,HumanVector& value){
                frames.human_clip_position_key(clip,index,key,key_time,value);
            });
        const auto rotation=sample_human_clip_keys(rotations.at(bone.index),time,3,
            [&]{return frames.human_clip_rotation_count(clip,index);},
            [&](std::int32_t key,std::uint32_t& key_time,HumanVector& value){
                frames.human_clip_rotation_key(clip,index,key,key_time,value);
            });
        const bool publish_rotation=human_joint_frame_basis_square(frames,bone.frame)>9.99999993922529e-09f;
        if(!human_clip_missing_key(position.previous[0]))
            frames.car_suspension_position(bone.frame,human_clip_position_target(position,time));
        if(!human_clip_missing_key(rotation.previous[3])){
            auto target=rotation.previous;
            const auto duration=human_frame_signed_word(rotation.next_time-rotation.previous_time);
            if(duration>0){
                const float fraction=static_cast<float>(static_cast<long double>(human_frame_signed_word(
                    static_cast<std::uint32_t>(time)-rotation.previous_time))/duration);
                target=frames.physics_slerp(rotation.previous,rotation.next,fraction,false);
            }
            if(publish_rotation)frames.car_steering_rotation(bone.frame,target);
        }
    }
    for(auto child:bone.children)if(child>=0)
        apply_human_ragdoll_clip_pose(frames,skeleton,clip,positions,rotations,time,child);
}

} //namespace hd2::game
#endif
