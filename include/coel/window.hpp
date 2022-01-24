#pragma once

#if COEL_USE_WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
namespace coel {
    using WindowHandle = HWND;
}
#endif

#if COEL_USE_X11
#include <X11/Xlib.h>
namespace coel {
    using WindowHandle = ::Window;
}
#endif

