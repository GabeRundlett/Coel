#pragma once

#if COEL_USE_WIN32
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

