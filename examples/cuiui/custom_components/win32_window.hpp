#include <iostream>
#include <glad/glad.h>
#include <dwmapi.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <memory>
#include <array>
#include <unordered_map>
#include <stack>

using WindowDim = std::array<uint32_t, 2>;

struct WindowState {
    WindowDim dim;

    WindowState * parent_state = nullptr;

    struct Win32 {
        HWND window_handle = nullptr;
        struct OpenGL {
            HDC   device_ctx;
            HGLRC render_ctx;
        } opengl;
    } win32;

    struct Temp {
        bool  should_close = false, is_open = false;
        bool  mouse_down = false;
        POINT mouse_down_point;
    } temp;
};
struct WindowOpenConfig {
    WindowDim dim;
};
class Window {
  private:
    std::shared_ptr<WindowState> state;

  public:
    Window();
    ~Window() = default;

    Window(const Window &) = default;
    Window & operator=(const Window &) = default;

    Window(Window &&) = default;
    Window & operator=(Window &&) = default;

    void update();
    bool is_open();
    void open(const WindowOpenConfig & config = {});
    void close();

    WindowDim             dim();
    WindowDim::value_type dim(size_t idx);

    friend class UiContext;

    // TODO:
    void bind_render_ctx();
    void swap_buffers();
};

static void windowmanager_init() {
    auto        app_instance_handle = GetModuleHandleA(nullptr);
    WNDCLASSEXA win32_window_class{
        .cbSize      = sizeof(WNDCLASSEXA),
        .style       = 0,
        .lpfnWndProc = [](HWND win32_window_handle, UINT message_id, WPARAM wp,
                          LPARAM lp) -> LRESULT {
            auto window_state_ptr =
                (WindowState *)GetWindowLongPtrA(win32_window_handle, GWLP_USERDATA);
            if (window_state_ptr) {
                switch (message_id) {
                case WM_CLOSE: {
                    window_state_ptr->temp.should_close = true;
                } break;

                case WM_MOUSEMOVE: {
                    if (window_state_ptr->temp.mouse_down) {
                        POINT currentpos;
                        GetCursorPos(&currentpos);
                        int  x = currentpos.x - window_state_ptr->temp.mouse_down_point.x;
                        int  y = currentpos.y - window_state_ptr->temp.mouse_down_point.y;
                        RECT window_rect;
                        GetWindowRect(win32_window_handle, &window_rect);
                        MoveWindow(win32_window_handle, x, y,
                                   window_rect.right - window_rect.left,
                                   window_rect.bottom - window_rect.top, false);
                    }
                } break;

                case WM_LBUTTONDOWN: {
                    window_state_ptr->temp.mouse_down = true;
                    GetCursorPos(&window_state_ptr->temp.mouse_down_point);
                    RECT window_rect;
                    GetWindowRect(win32_window_handle, &window_rect);
                    window_state_ptr->temp.mouse_down_point.x -= window_rect.left;
                    window_state_ptr->temp.mouse_down_point.y -= window_rect.top;
                    SetCapture(win32_window_handle);
                } break;

                case WM_LBUTTONUP: {
                    ReleaseCapture();
                    window_state_ptr->temp.mouse_down = false;
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
            } else {
                switch (message_id) {
                case WM_NCCREATE: {
                    auto & cs = *(CREATESTRUCTA *)lp;
                    SetWindowLongPtrA(win32_window_handle, GWLP_USERDATA,
                                      (LONG_PTR)cs.lpCreateParams);
                } break;
                default: {
                    std::cout << "message_id: " << message_id << "\n";
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

    state->temp.is_open      = false;
    state->temp.should_close = false;
}

void Window::update() {
    if (state->temp.should_close) close();
    if (state->temp.is_open) {
        MSG msg;
        while (PeekMessageA(&msg, state->win32.window_handle, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
}
void Window::open(const WindowOpenConfig & config) {
    if (!state->temp.is_open) {
        auto app_instance_handle = GetModuleHandleA(nullptr);
        state->dim               = config.dim;
        state->win32.window_handle =
            CreateWindowExA(0, "cuiui_wc", "cuiui_wc title", WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, state->dim[0], state->dim[1],
                            nullptr, nullptr, app_instance_handle, (LPVOID)state.get());
        state->temp.should_close = false;
        state->temp.is_open      = true;
        SetWindowLongPtrA(state->win32.window_handle, GWLP_USERDATA,
                          (LONG_PTR)state.get());
        ShowWindow(state->win32.window_handle, SW_SHOW);
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
        state->win32.opengl.device_ctx = GetDC(state->win32.window_handle);
        auto choose_format = ChoosePixelFormat(state->win32.opengl.device_ctx, &pfd);
        SetPixelFormat(state->win32.opengl.device_ctx, choose_format, &pfd);
        state->win32.opengl.render_ctx = wglCreateContext(state->win32.opengl.device_ctx);
        wglMakeCurrent(state->win32.opengl.device_ctx, state->win32.opengl.render_ctx);
        // ((BOOL(WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT"))(0);
        UpdateWindow(state->win32.window_handle);
        gladLoadGL();
    }
}
void Window::close() {
    if (state->temp.is_open) {
        wglDeleteContext(state->win32.opengl.render_ctx);
        ReleaseDC(state->win32.window_handle, state->win32.opengl.device_ctx);
        DestroyWindow(state->win32.window_handle);
        state->temp.should_close = false;
        state->temp.is_open      = false;
    }
}
bool Window::is_open() {
    return state->temp.is_open;
}
void Window::bind_render_ctx() {
    wglMakeCurrent(state->win32.opengl.device_ctx, state->win32.opengl.render_ctx);
}
void Window::swap_buffers() {
    SwapBuffers(state->win32.opengl.device_ctx);
}
WindowDim Window::dim() {
    return state->dim;
}
WindowDim::value_type Window::dim(size_t idx) {
    return state->dim[idx];
}
