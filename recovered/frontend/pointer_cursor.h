#ifndef HD2_FRONTEND_POINTER_CURSOR_H
#define HD2_FRONTEND_POINTER_CURSOR_H

namespace hd2::frontend {

// Cursor globals 00840660/64 and input fields +10/+14/+18/+1c/+24/+28.
// Portable storage replaces the original shared globals and object ABI.
class PointerCursor {
public:
    void initialize(float x, float y, float width, float height) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }
    void sample(float horizontal, float vertical, float wheel) {
        horizontal_ = horizontal;
        vertical_ = vertical;
        wheel_delta_ = wheel;
    }
    // Client 0066a220: integrate only after suppression/unchanged-input gates.
    void integrate() {
        x_ = static_cast<float>(static_cast<long double>(x_) + horizontal_);
        y_ = static_cast<float>(static_cast<long double>(y_) + vertical_);
        wheel_ = static_cast<float>(static_cast<long double>(wheel_) + wheel_delta_);
        clip_axis(x_, horizontal_, width_);
        clip_axis(y_, vertical_, height_);
    }
    void clear() { *this = {}; }
    float x() const { return x_; }
    float y() const { return y_; }
    float horizontal() const { return horizontal_; }
    float vertical() const { return vertical_; }
    float wheel_delta() const { return wheel_delta_; }
    float wheel() const { return wheel_; }

private:
    static void clip_axis(float& position, float& delta, float limit) {
        // Preserve original store boundaries and subtraction order: the upper
        // overflow stays in x87 precision until subtracted from the delta.
        // Do not replace with delta = clamped_position - previous_position.
        if (position < 0.0f) {
            delta = static_cast<float>(static_cast<long double>(delta) - position);
            position = 0.0f;
        } else if (position > limit) {
            delta = static_cast<float>(static_cast<long double>(delta) -
                (static_cast<long double>(position) - limit));
            position = limit;
        }
    }

    float x_ = 0, y_ = 0;
    float width_ = 0, height_ = 0;
    float horizontal_ = 0, vertical_ = 0, wheel_delta_ = 0;
    float wheel_ = 0;
};

}

#endif
