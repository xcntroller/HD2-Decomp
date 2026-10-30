#ifndef HD2_HOST_FRAME_CLOCK_H
#define HD2_HOST_FRAME_CLOCK_H

#include <windows.h>
#include <mmsystem.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::host {

// Win32 operations used by 006dc950 and the event lifetime in 006dcae0.
class FrameClockPlatform {
public:
    FrameClockPlatform() : event_(CreateEventW(nullptr, TRUE, FALSE, nullptr)) {
        if (!event_) throw std::runtime_error("Cannot create original frame-timer event");
    }
    ~FrameClockPlatform() { CloseHandle(event_); }
    FrameClockPlatform(const FrameClockPlatform&) = delete;
    FrameClockPlatform& operator=(const FrameClockPlatform&) = delete;

    std::uint32_t microseconds() const {
        LARGE_INTEGER frequency{}, counter{};
        if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0 ||
                !QueryPerformanceCounter(&counter))
            throw std::runtime_error("Cannot read original frame-timer performance clock");
        // Original __allmul/__aulldiv preserve the low 64 product bits, then
        // retain the low 32 quotient bits. Do not widen away that wrapping.
        const auto micros = static_cast<std::uint64_t>(counter.QuadPart) * 1000000u /
                            static_cast<std::uint64_t>(frequency.QuadPart);
        return static_cast<std::uint32_t>(micros);
    }
    void wait_for_frame(std::uint32_t milliseconds) const {
        static_assert(TIME_CALLBACK_EVENT_PULSE == 0x20);
        LPTIMECALLBACK callback{};
        static_assert(sizeof(callback) == sizeof(event_));
        std::memcpy(&callback, &event_, sizeof(callback));
        timeSetEvent(milliseconds, 0, callback, 0, TIME_CALLBACK_EVENT_PULSE);
        WaitForSingleObject(event_, 100);
    }

private:
    HANDLE event_;
};

} // namespace hd2::host
#endif
