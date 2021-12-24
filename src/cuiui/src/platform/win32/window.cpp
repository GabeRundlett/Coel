#include <cuiui/cuiui.hpp>
#include <iostream>

#include <glad/glad.h>
#include <dwmapi.h>

constexpr DWORD window_style = WS_OVERLAPPEDWINDOW;

namespace cuiui {
    static void native::windowmanager_init() {
        auto        app_instance_handle = GetModuleHandleA(nullptr);
        WNDCLASSEXA win32_window_class{
            .cbSize      = sizeof(WNDCLASSEXA),
            .style       = 0,
            .lpfnWndProc = [](HWND win32_window_handle, UINT message_id, WPARAM wp,
                              LPARAM lp) -> LRESULT {
                auto window_state_ptr = (cuiui::WindowState *)GetWindowLongPtrA(
                    win32_window_handle, GWLP_USERDATA);
                if (window_state_ptr) {
                    switch (message_id) {
                    case WM_CLOSE: window_state_ptr->native.should_close = true; break;
                    case WM_SETTEXT: break;
                    case WM_GETICON: break;
                    case WM_SETCURSOR: break;
                    case WM_NCHITTEST: break;

                    case WM_MOUSEMOVE: {
                        if (window_state_ptr->native.mouse_down) {
                            POINT currentpos;
                            GetCursorPos(&currentpos);
                            int x = currentpos.x -
                                    window_state_ptr->native.mouse_down_point.x;
                            int y = currentpos.y -
                                    window_state_ptr->native.mouse_down_point.y;
                            RECT window_rect;
                            GetWindowRect(win32_window_handle, &window_rect);
                            // AdjustWindowRect(&window_rect, window_style, false);
                            MoveWindow(win32_window_handle, x, y,
                                       window_rect.right - window_rect.left,
                                       window_rect.bottom - window_rect.top, false);
                        }
                    } break;

                    case WM_LBUTTONDOWN: {
                        window_state_ptr->native.mouse_down = true;
                        GetCursorPos(&window_state_ptr->native.mouse_down_point);
                        RECT window_rect;
                        GetWindowRect(win32_window_handle, &window_rect);
                        // AdjustWindowRect(&window_rect, window_style, false);
                        window_state_ptr->native.mouse_down_point.x -= window_rect.left;
                        window_state_ptr->native.mouse_down_point.y -= window_rect.top;
                        SetCapture(win32_window_handle);
                    } break;

                    case WM_LBUTTONUP: {
                        ReleaseCapture();
                        window_state_ptr->native.mouse_down = false;
                    } break;

                    case WM_SIZE: {
                        RECT window_rect;
                        GetWindowRect(win32_window_handle, &window_rect);
                        window_state_ptr->dim[0] = window_rect.right - window_rect.left;
                        window_state_ptr->dim[1] = window_rect.bottom - window_rect.top;
                    };

                    default: {
                        // std::cout << "message_id: " << message_id << "\n";
                    } break;
                    }
                }
                return DefWindowProcA(win32_window_handle, message_id, wp, lp);
            },
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = app_instance_handle,
            .hIcon         = nullptr,
            .hCursor       = nullptr,
            .hbrBackground = nullptr,
            .lpszMenuName  = nullptr,
            .lpszClassName = "cuiui_wc",
            .hIconSm       = nullptr,
        };
        RegisterClassExA(&win32_window_class);
    }

    Window::Window() {
        state = std::make_shared<WindowState>();

        state->native.is_open      = false;
        state->native.should_close = false;
    }

    void Window::update() {
        if (state->native.should_close) close();
        if (state->native.is_open) {
            MSG msg;
            while (PeekMessageA(&msg, state->native.handle, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }
    void Window::open(const WindowConfig & config) {
        if (!state->native.is_open) {
            // if (!config.title.empty()) {
            //     state->title = config.title;
            // } else {
            //     state->title = "Default Title";
            // }

            // Win32
            auto app_instance_handle = GetModuleHandleA(nullptr);
            state->dim               = config.dim;

            state->native.handle = CreateWindowExA(
                0, "cuiui_wc", "cuiui_wc title", window_style, CW_USEDEFAULT,
                CW_USEDEFAULT, state->dim[0], state->dim[1], nullptr, nullptr,
                app_instance_handle, nullptr);
            state->native.should_close = false;
            state->native.is_open      = true;
            SetWindowLongPtrA(state->native.handle, GWLP_USERDATA, (LONG_PTR)state.get());
            ShowWindow(state->native.handle, SW_SHOW);

            // OpenGL Context/Loader
            PIXELFORMATDESCRIPTOR pfd{
                .nSize       = sizeof(PIXELFORMATDESCRIPTOR),
                .nVersion    = 1,
                .dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                .iPixelType  = PFD_TYPE_RGBA,
                .cColorBits  = 32,
                .cAlphaBits  = 8,
                .cDepthBits  = 24,
                .cAuxBuffers = 0,
                .iLayerType  = PFD_MAIN_PLANE,
            };
            state->native.device_ctx = GetDC(state->native.handle);
            auto choose_format       = ChoosePixelFormat(state->native.device_ctx, &pfd);
            SetPixelFormat(state->native.device_ctx, choose_format, &pfd);
            state->native.render_ctx = wglCreateContext(state->native.device_ctx);
            wglMakeCurrent(state->native.device_ctx, state->native.render_ctx);
            // ((BOOL(WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT"))(0);
            UpdateWindow(state->native.handle);
            gladLoadGL();
        }
    }
    void Window::close() {
        if (state->native.is_open) {
            // OpenGL Context
            wglDeleteContext(state->native.render_ctx);
            ReleaseDC(state->native.handle, state->native.device_ctx);

            // Win32
            DestroyWindow(state->native.handle);
            state->native.should_close = false;
            state->native.is_open      = false;
        }
    }
    bool Window::is_open() {
        return state->native.is_open;
    }
    void Window::bind_render_ctx() {
        // OpenGL
        wglMakeCurrent(state->native.device_ctx, state->native.render_ctx);
    }
    void Window::swap_buffers() {
        // OpenGL
        SwapBuffers(state->native.device_ctx);
    }

    WindowDim Window::dim() {
        return state->dim;
    }

    WindowDim::value_type Window::dim(size_t idx) {
        return state->dim[idx];
    }
    // void Window::update_title() {
    //     SetWindowTextA(state->native.handle, state->title.c_str());
    // }
} // namespace cuiui
