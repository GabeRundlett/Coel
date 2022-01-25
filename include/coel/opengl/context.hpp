#pragma once

#if COEL_USE_WIN32
#include <Windows.h>
namespace coel::opengl {
    using WindowHandle = HWND;
    struct NativeContextState {
        HDC hdc = nullptr;
        HGLRC hrc = nullptr;
        int prev_pixel_format = -1;
    };
} // namespace coel::opengl
#elif COEL_USE_X11
#define GLFW_INCLUDE_NONE
#include <X11/Xlib.h>
#include <GL/glx.h>
namespace coel::opengl {
    using WindowHandle = ::Window;
    struct NativeContextState {
        Display *display;
        GLXContext glx_context;
        ::Window xid;
    };
} // namespace coel::opengl
#elif COEL_USE_NULL
namespace coel::opengl {
    using WindowHandle = void *;
    struct NativeContextState {
        WindowHandle w = nullptr;
    };
} // namespace coel::opengl
#else
#error "Unsupported platform!"
#endif

namespace coel::opengl {
    struct Context : NativeContextState {
        void COEL_EXPORT attach(WindowHandle w);
        void COEL_EXPORT make_current();
        void COEL_EXPORT swap_buffers();
        void COEL_EXPORT detach(WindowHandle w);
    };
} // namespace coel::opengl
