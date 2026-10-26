#include "host/graphics_settings.h"

#include <cassert>
#include <iostream>

int main() {
    const auto settings = hd2::host::windowed_settings(0x12345678);
    assert(settings.instance == 0x12345678);
    assert(settings.adapter == 0 && settings.x == 50 && settings.y == 50);
    assert(settings.width == 800 && settings.height == 600);
    assert(settings.child_x == 0 && settings.child_y == 0);
    assert(settings.child_width == 800 && settings.child_height == 600);
    assert(settings.color_bits == 32 && settings.refresh_rate == 0);
    assert(settings.reserved == 0 && settings.flags == 0x80);
    assert(settings.menu == 0 && settings.opaque_callback == 0);
    for (const auto* text : {L"0", L"1", L"180", L"4294967295"}) {
        const auto count = hd2::host::parse_frame_limit(text);
        assert(count == (text == std::wstring_view(L"4294967295") ? 0xffffffffU :
                         text == std::wstring_view(L"180") ? 180U :
                         text == std::wstring_view(L"1") ? 1U : 0U));
    }
    for (const auto* text : {L"", L"-1", L"+1", L" 1", L"1 ", L"1x",
                             L"4294967296", L"999999999999999999999"}) {
        bool rejected = false;
        try {
            hd2::host::parse_frame_limit(text);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
    }
    std::cout << "Graphics settings layout/defaults and frame-limit checks passed\n";
}
