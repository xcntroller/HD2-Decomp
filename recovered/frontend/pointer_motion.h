#ifndef HD2_FRONTEND_POINTER_MOTION_H
#define HD2_FRONTEND_POINTER_MOTION_H

namespace hd2::frontend {

// Client 00669ff0.
inline bool pointer_moved(float horizontal, float vertical) {
    return horizontal != 0.0f || vertical != 0.0f;
}

}

#endif
