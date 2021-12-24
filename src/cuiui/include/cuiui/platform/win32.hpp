#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace cuiui::native {
    struct WindowState {
        HWND handle       = nullptr;
        bool should_close = false, is_open = false;

        // OpenGL
        HDC   device_ctx;
        HGLRC render_ctx;

        bool mouse_down = false;
        POINT mouse_down_point;
    };

    void windowmanager_init();
} // namespace cuiui::native
