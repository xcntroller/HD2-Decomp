#ifndef HD2_GAME_HUMAN_BONE_DIRECTION_H
#define HD2_GAME_HUMAN_BONE_DIRECTION_H

#include "game/human_skeleton.h"
#include "game/human_joint_math.h"
#include <optional>

namespace hd2::game {

//Native composition shared by007BF980/007BFE80 and007A6C70. Each vector
//product and sum retains its original FLOAT spill before the scalar dot.
inline HumanVector multiply_human_bone_rotations(const HumanVector& left,const HumanVector& right){
    HumanVector cross{};
    cross[0] = static_cast<float>(static_cast<long double>(left[1]) * right[2] - static_cast<long double>(left[2]) * right[1]);
    cross[1] = static_cast<float>(static_cast<long double>(left[2]) * right[0] - static_cast<long double>(left[0]) * right[2]);
    cross[2] = static_cast<float>(static_cast<long double>(left[0]) * right[1] - static_cast<long double>(left[1]) * right[0]);
    HumanVector combined{};
    for (unsigned component = 0; component != 3; ++component) {
        const float first = static_cast<float>(static_cast<long double>(left[component]) * right[3]);
        const float second = static_cast<float>(static_cast<long double>(right[component]) * left[3]);
        const float sum = first + second;
        combined[component] = sum + cross[component];
    }
    const float scalar = static_cast<float>(static_cast<long double>(left[3]) * right[3]);
    const auto dot = (static_cast<long double>(right[2]) * left[2] +
        static_cast<long double>(right[1]) * left[1]) + static_cast<long double>(right[0]) * left[0];
    combined[3] = static_cast<float>(scalar - dot);
    return combined;
}

//Shared quaternion worker in007BF980 and007BFE80. Callers retain their
//different projection and cross-product FLOAT spill boundaries.
inline std::optional<HumanVector> human_bone_rotation_correction(HumanVector axis,
        const HumanVector& rotation) {
    float length = static_cast<float>((static_cast<long double>(axis[2]) * axis[2] +
        static_cast<long double>(axis[1]) * axis[1]) + static_cast<long double>(axis[0]) * axis[0]);
    std::int32_t bits;
    std::memcpy(&bits, &length, sizeof(bits));
    if (bits < 0x322bcc77) return std::nullopt;
    constexpr float epsilon = 9.99999993922529e-09f;
    if (std::abs(static_cast<long double>(length) - 1) < epsilon) length = 1;
    else {
        const auto root = std::sqrt(static_cast<long double>(length));
        length = static_cast<float>(root);
        if (!(root > epsilon)) return std::nullopt;
    }
    const auto inverse_length = 1.L / length;
    for (unsigned component = 0; component != 3; ++component)
        axis[component] = static_cast<float>(axis[component] * inverse_length);
    const auto half_angle = std::asin(static_cast<long double>(length)) * .5;
    const auto sine = std::sin(half_angle);
    HumanVector correction{};
    for (unsigned component = 0; component != 3; ++component)
        correction[component] = static_cast<float>(axis[component] * sine);
    correction[3] = static_cast<float>(std::cos(static_cast<long double>(static_cast<float>(half_angle))));

    return multiply_human_bone_rotations(rotation,correction);
}

//007BF980 transforms both directions through repeated actual model queries,
//then applies the weighted cross-product rotation to the retained bone.
template<class Frames>
void correct_human_bone_direction(Frames& frames,
        const HumanSkeleton<typename Frames::Frame>& skeleton, typename Frames::Frame model,
        unsigned bone_index, const HumanVector& desired, const HumanVector& reference,
        const HumanVector& weights) {
    const auto& bone = skeleton.bones().at(bone_index);
    const auto project = [&](const HumanVector& value) {
        const auto z = frames.actor_world_matrix(model);
        const auto y = frames.actor_world_matrix(model);
        const auto x = frames.actor_world_matrix(model);
        HumanVector result = value;
        for (unsigned axis = 0; axis != 3; ++axis) {
            const auto& matrix = axis == 0 ? x : axis == 1 ? y : z;
            result[axis] = static_cast<float>((static_cast<long double>(matrix[axis * 4 + 1]) * value[1] +
                static_cast<long double>(matrix[axis * 4 + 2]) * value[2]) +
                static_cast<long double>(matrix[axis * 4]) * value[0]);
        }
        return result;
    };
    const auto target = project(desired), current = project(reference);
    auto inverse = human_transpose(frames.actor_world_matrix(model));
    inverse[3] = inverse[7] = inverse[11] = 0;
    auto local = frames.rigid_matrix_product(frames.actor_world_matrix(bone.frame), inverse);
    const auto rotation = frames.human_matrix_rotation(local);
    HumanVector axis = desired; //Native temporary W is unwritten. Retain known input padding.
    axis[0] = static_cast<float>((static_cast<long double>(current[1]) * target[2] -
        static_cast<long double>(current[2]) * target[1]) * weights[0]);
    axis[1] = static_cast<float>((static_cast<long double>(current[2]) * target[0] -
        static_cast<long double>(current[0]) * target[2]) * weights[1]);
    axis[2] = static_cast<float>((static_cast<long double>(current[0]) * target[1] -
        static_cast<long double>(current[1]) * target[0]) * weights[2]);
    const auto combined = human_bone_rotation_correction(axis, rotation);
    if (!combined) return;
    frames.human_set_matrix_rotation(local, *combined);
    local = frames.car_matrix_product(local, frames.actor_world_matrix(model));
    const auto parent = bone.parent < 0 ? model : skeleton.bones().at(static_cast<std::size_t>(bone.parent)).frame;
    inverse = human_transpose(frames.actor_world_matrix(parent));
    inverse[3] = inverse[7] = inverse[11] = 0;
    local = frames.car_matrix_product(local, inverse);
    frames.human_publish_bone_rotation(bone.frame, local);
}

//007BFE80 uses the supplied actual reference matrix, with a stored cross
//product before applying the weights.
template<class Frames>
void correct_human_bone_reference(Frames& frames,
        const HumanSkeleton<typename Frames::Frame>& skeleton, typename Frames::Frame model,
        unsigned bone_index, const HumanVector& desired, const HumanVector& reference,
        const HumanMatrix& basis, const HumanVector& weights) {
    const auto project = [&](const HumanVector& value, bool target) {
        HumanVector result = value;
        const auto x = static_cast<long double>(basis[0]) * value[0];
        const auto y = static_cast<long double>(basis[1]) * value[1];
        const auto z = static_cast<long double>(basis[2]) * value[2];
        result[0] = static_cast<float>(target ? (x + z) + y : (z + x) + y);
        for (unsigned axis = 1; axis != 3; ++axis)
            result[axis] = static_cast<float>((static_cast<long double>(basis[axis * 4 + 2]) * value[2] +
                static_cast<long double>(basis[axis * 4 + 1]) * value[1]) +
                static_cast<long double>(basis[axis * 4]) * value[0]);
        return result;
    };
    const auto target = project(desired, true), current = project(reference, false);
    const auto& bone = skeleton.bones().at(bone_index);
    auto inverse = human_transpose(basis);
    inverse[3] = inverse[7] = inverse[11] = 0;
    auto local = frames.rigid_matrix_product(frames.actor_world_matrix(bone.frame), inverse);
    const auto rotation = frames.human_matrix_rotation(local);
    HumanVector axis = desired;
    axis[0] = static_cast<float>(static_cast<long double>(current[1]) * target[2] - static_cast<long double>(current[2]) * target[1]);
    axis[1] = static_cast<float>(static_cast<long double>(current[2]) * target[0] - static_cast<long double>(current[0]) * target[2]);
    axis[2] = static_cast<float>(static_cast<long double>(current[0]) * target[1] - static_cast<long double>(current[1]) * target[0]);
    for (unsigned component = 0; component != 3; ++component)
        axis[component] = static_cast<float>(static_cast<long double>(axis[component]) * weights[component]);
    const auto combined = human_bone_rotation_correction(axis, rotation);
    if (!combined) return;
    frames.human_set_matrix_rotation(local, *combined);
    local = frames.car_matrix_product(local, basis);
    const auto parent = bone.parent < 0 ? model : skeleton.bones().at(static_cast<std::size_t>(bone.parent)).frame;
    inverse = human_transpose(frames.actor_world_matrix(parent));
    inverse[3] = inverse[7] = inverse[11] = 0;
    local = frames.car_matrix_product(local, inverse);
    frames.human_publish_bone_rotation(bone.frame, local);
}

//007A2F10 rotates an existing matrix so its Y row follows the supplied vector.
template<class Frames>
void align_human_matrix_up(Frames& frames, HumanMatrix& matrix, const HumanVector& direction) {
    HumanMatrix rotation{};
    human_align_vectors(HumanVector{{matrix[4], matrix[5], matrix[6], matrix[7]}}, direction, rotation);
    rotation[15] = 1;
    matrix = frames.car_matrix_product(matrix, rotation);
}

//007A3260 extracts signed roll relative to007A2E10's vertical basis.
template<class Frames>float human_matrix_roll(Frames& frames,const HumanMatrix& matrix){
    const HumanVector forward{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const auto relative=frames.rigid_matrix_product(matrix,human_transpose(human_vertical_basis(forward)));
    return static_cast<float>(std::atan2(static_cast<long double>(relative[1]),static_cast<long double>(relative[0])));
}

//007A43C0 uses the original first-child basis stored by skeleton loading.
template<class Frames>
void orient_human_bone(Frames& frames,const HumanSkeleton<typename Frames::Frame>& skeleton,
        typename Frames::Frame model,unsigned index,const HumanVector& direction,float influence){
    if(index>=23)return;
    const auto& bone=skeleton.bones()[index];
    auto world=frames.actor_world_matrix(bone.frame);
    world[12]=world[13]=world[14]=0;world[15]=1;
    const auto parent=bone.parent<0?model:skeleton.bones()[static_cast<std::size_t>(bone.parent)].frame;
    const auto inverse=human_transpose(frames.actor_world_matrix(parent));
    const auto& basis=bone.bases[0];
    auto target=human_bone_direction_basis(frames.rigid_matrix_product(basis,world),direction);
    target=frames.rigid_matrix_product(human_transpose(basis),target);
    target=frames.rigid_matrix_product(target,inverse);
    if(std::signbit(influence))orthonormalize_human_bone_matrix(target);
    else{
        const auto desired=frames.human_matrix_rotation(target);
        const auto current=frames.human_matrix_rotation(frames.car_local_matrix(bone.frame));
        const float complement=1.f-influence;
        frames.human_set_matrix_quaternion(target,frames.physics_slerp(desired,current,complement,false));
    }
    frames.human_publish_bone_rotation(bone.frame,target);
}

} //namespace hd2::game
#endif
