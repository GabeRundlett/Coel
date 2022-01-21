#include <coel/opengl/context.hpp>
#include <stdexcept>

namespace coel::opengl {
    void Context::attach(WindowHandle w) {
#if COEL_USE_WIN32
        PIXELFORMATDESCRIPTOR pfd{
            .nSize = sizeof(PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 32,
            .cRedBits = {},
            .cRedShift = {},
            .cGreenBits = {},
            .cGreenShift = {},
            .cBlueBits = {},
            .cBlueShift = {},
            .cAlphaBits = 8,
            .cAlphaShift = {},
            .cAccumBits = {},
            .cAccumRedBits = {},
            .cAccumGreenBits = {},
            .cAccumBlueBits = {},
            .cAccumAlphaBits = {},
            .cDepthBits = 24,
            .cStencilBits = 8,
            .cAuxBuffers = 0,
            .iLayerType = PFD_MAIN_PLANE,
            .bReserved = {},
            .dwLayerMask = {},
            .dwVisibleMask = {},
            .dwDamageMask = {},
        };
        if (!hdc)
            hdc = GetDC(w);
        prev_pixel_format = GetPixelFormat(hdc);
        auto chosen_format = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, chosen_format, &pfd);
        if (!hrc)
            hrc = wglCreateContext(hdc);
#elif COEL_USE_X11
#endif
    }
    void Context::make_current() {
#if COEL_USE_WIN32
        wglMakeCurrent(hdc, hrc);
#elif COEL_USE_X11
        glXMakeCurrent(display, xid, glx_context);
#endif
    }
    void Context::swap_buffers() {
#if COEL_USE_WIN32
        SwapBuffers(hdc);
#elif COEL_USE_X11
        glXSwapBuffers(display, xid);
#endif
    }
    void Context::detach(WindowHandle w) {
#if COEL_USE_WIN32
        if (hrc) {
            wglMakeCurrent(hdc, nullptr);
            wglDeleteContext(hrc);
            hrc = nullptr;
        }
        if (hdc) {
            SetPixelFormat(hdc, prev_pixel_format, nullptr);
            ReleaseDC(w, hdc);
            hdc = nullptr;
        }
#elif COEL_USE_X11
        glXMakeCurrent(display, xid, nullptr);
#endif
    }
} // namespace coel::opengl
