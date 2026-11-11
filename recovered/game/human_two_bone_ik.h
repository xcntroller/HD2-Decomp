#ifndef HD2_GAME_HUMAN_TWO_BONE_IK_H
#define HD2_GAME_HUMAN_TWO_BONE_IK_H

#include "game/human_ai_shot_direction.h"

namespace hd2::game {

//007A3710, geometry6628/662C preferences are supplied by the containing owner.
//The native failure sign-bit test precedes every write to the middle point.
inline bool solve_human_two_bone_positions(const HumanVector& root, float first_length,
        const HumanVector& target, float second_length, HumanVector& middle,
        const HumanVector* preferred, float aligned_weight, float opposed_weight) {
    const float first_square = static_cast<float>(static_cast<long double>(first_length) * first_length);
    const float second_square = static_cast<float>(static_cast<long double>(second_length) * second_length);
    HumanVector direction = middle;
    for (unsigned axis = 0; axis != 3; ++axis) direction[axis] = target[axis] - root[axis];
    const float square = static_cast<float>((static_cast<long double>(direction[2]) * direction[2] +
        static_cast<long double>(direction[1]) * direction[1]) + static_cast<long double>(direction[0]) * direction[0]);
    std::int32_t bits;
    std::memcpy(&bits, &square, sizeof(bits));
    long double distance = 0;
    if (bits >= 0x322bcc77) {
        distance = std::abs(static_cast<long double>(square) - 1) < 9.99999993922529e-09f ?
            1.L : std::sqrt(static_cast<long double>(square));
    }
    const float inverse = static_cast<float>(1.L / distance);
    const auto numerator = (static_cast<long double>(first_square) - second_square) + distance * distance;
    const float height_square = static_cast<float>(first_square -
        (((numerator * numerator) * inverse) * inverse) * .25f);
    if (std::signbit(height_square)) return false;
    const float along = static_cast<float>(numerator * inverse * .5f);
    HumanVector centre = root;
    for (unsigned axis = 0; axis != 3; ++axis) {
        direction[axis] *= inverse;
        const float offset = direction[axis] * along;
        centre[axis] = offset + root[axis];
    }
    std::array<long double, 3> displacement;
    for (unsigned axis = 0; axis != 3; ++axis) {
        displacement[axis] = static_cast<long double>(middle[axis]) - centre[axis];
        middle[axis] = static_cast<float>(displacement[axis]);
    }
    const auto projection = (displacement[2] * direction[2] + displacement[0] * direction[0]) + displacement[1] * direction[1];
    HumanVector bend = middle;
    for (unsigned axis = 0; axis != 3; ++axis) {
        const float projected = static_cast<float>(direction[axis] * projection);
        bend[axis] = middle[axis] - projected;
    }
    const auto squared_length = [](const HumanVector& value) {
        return (static_cast<long double>(value[2]) * value[2] +
            static_cast<long double>(value[1]) * value[1]) + static_cast<long double>(value[0]) * value[0];
    };
    normalize_human_ai_shot(bend, squared_length(bend));
    HumanVector radial = bend;
    const auto radius = std::sqrt(static_cast<long double>(height_square));
    if (preferred) {
        const auto dot = (static_cast<long double>(direction[2]) * (*preferred)[2] +
            static_cast<long double>(direction[1]) * (*preferred)[1]) + static_cast<long double>(direction[0]) * (*preferred)[0];
        auto desired = *preferred;
        for (unsigned axis = 0; axis != 3; ++axis) {
            const float projected = static_cast<float>(direction[axis] * dot);
            desired[axis] -= projected;
        }
        normalize_human_ai_shot(desired, squared_length(desired));
        const auto alignment = (static_cast<long double>(desired[2]) * bend[2] +
            static_cast<long double>(desired[1]) * bend[1]) + static_cast<long double>(desired[0]) * bend[0];
        const auto weight = alignment > 0 ? aligned_weight : opposed_weight;
        for (unsigned axis = 0; axis != 3; ++axis) {
            const float old = static_cast<float>((1.L - weight) * bend[axis]);
            const float next = static_cast<float>(static_cast<long double>(desired[axis]) * weight);
            radial[axis] = old + next;
        }
        normalize_human_ai_shot(radial, squared_length(radial));
        for (unsigned axis = 0; axis != 3; ++axis) radial[axis] = static_cast<float>(radius * radial[axis]);
    } else {
        const float stored_radius = static_cast<float>(radius); //004042B0 takes a stored FLOAT scale.
        for (unsigned axis = 0; axis != 3; ++axis) radial[axis] *= stored_radius;
    }
    for (unsigned axis = 0; axis != 3; ++axis) middle[axis] = centre[axis] + radial[axis];
    //Native vector temporaries leave W unwritten. Retain the known input lane.
    return true;
}

} //namespace hd2::game
#endif
