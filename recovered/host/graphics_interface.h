#ifndef HD2_HOST_GRAPHICS_INTERFACE_H
#define HD2_HOST_GRAPHICS_INTERFACE_H

#include "host/graphics_settings.h"
#include "host/menu_cursor.h"

namespace hd2::host {

struct Graphics;
using Method = void (WINAPI *)(Graphics*);
using Settings = hd2::host::GraphicsSettings32;

struct GraphicsVtable {
    std::int32_t (WINAPI *initialize)(Graphics*, const Settings*);
    const Settings* (WINAPI *settings)(Graphics*);
    std::int32_t (WINAPI *set_flags)(Graphics*, std::uint32_t);
    std::uint8_t (WINAPI *initialized)(Graphics*);
    Method close;
    Method pump;
    std::int32_t (WINAPI *width)(Graphics*);
    std::int32_t (WINAPI *height)(Graphics*);
    Method reserved_20[7];
    std::int32_t (WINAPI *load_texture)(Graphics*, const char*, const char*, std::uint32_t,
                                       hd2::host::EngineFrame**);
    Method reserved_40;
    std::uint8_t (WINAPI *bind_texture)(Graphics*, hd2::host::EngineFrame*, std::uint32_t);
    HWND (WINAPI *window)(Graphics*);
    HWND (WINAPI *child_window)(Graphics*);
    Method reserved_50[5];
    void (WINAPI *clear)(Graphics*, std::uint32_t, float, std::uint32_t);
    Method begin_scene;
    Method end_scene;
    Method present;
    Method reserved_74[2];
    std::int32_t (WINAPI *set_viewport)(Graphics*, const hd2::host::CursorViewport*);
    const hd2::host::CursorViewport* (WINAPI *viewport)(Graphics*);
    Method reserved_84;
    std::int32_t (WINAPI *draw_primitive)(Graphics*, std::uint32_t, std::uint32_t,
                                         const hd2::host::CursorVertex*, std::uint32_t);
};

struct Graphics {
    const GraphicsVtable* methods;
};

static_assert(sizeof(void*) == 4, "The original engine requires an i386 executable");
static_assert(offsetof(GraphicsVtable, close) == 0x10);
static_assert(offsetof(GraphicsVtable, window) == 0x48);
static_assert(offsetof(GraphicsVtable, clear) == 0x64);
static_assert(offsetof(GraphicsVtable, present) == 0x70);
static_assert(offsetof(GraphicsVtable, load_texture) == 0x3c);
static_assert(offsetof(GraphicsVtable, bind_texture) == 0x44);
static_assert(offsetof(GraphicsVtable, set_viewport) == 0x7c);
static_assert(offsetof(GraphicsVtable, viewport) == 0x80);
static_assert(offsetof(GraphicsVtable, draw_primitive) == 0x88);


} // namespace hd2::host
#endif
