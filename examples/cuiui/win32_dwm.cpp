#include "custom_components/win32_window.hpp"
#include "custom_components/utility.hpp"
#include <dwmapi.h>

#include <thread>
using namespace std::literals;

#include "math/types.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmt/format.h>

struct W1 : Win32Window<W1> {
    bool    mouse_down : 1 = false;
    i32vec2 mouse_down_point;
    u32     grab_mode = 0;
    i32vec2 min_dim{200, 100};

    HCURSOR cursor_arrow;
    HCURSOR cursor_hand;
    HCURSOR cursor_sizens;
    HCURSOR cursor_sizewe;
    HCURSOR cursor_sizenwse;
    HCURSOR cursor_sizenesw;

    std::chrono::steady_clock::time_point start_time;

    LRESULT win32_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        LRESULT result = 0;
        if (msg == WM_NCCALCSIZE)
            if (wp) return 0;

        switch (msg) {
        case WM_NCHITTEST:
            if (mouse_down) return -1;
            break;
        case WM_LBUTTONDOWN: {
            mouse_down = true;
            POINT mp;
            GetCursorPos(&mp);
            mouse_down_point.x = mp.x, mouse_down_point.y = mp.y;
            RECT r;
            GetWindowRect(win32.window_handle, &r);
            i32rect window_rect{{r.left, r.top}, {r.right, r.bottom}};
            pos       = window_rect.p0;
            dim       = window_rect.p1 - window_rect.p0;
            grab_mode = get_hovered_edge(window_rect, mouse_down_point);
            mouse_down_point -= window_rect.p0;
            if (grab_mode & 0x02) mouse_down_point.x = dim.x - mouse_down_point.x;
            if (grab_mode & 0x08) mouse_down_point.y = dim.y - mouse_down_point.y;
        } break;
        case WM_LBUTTONUP: {
            mouse_down = false;
        } break;
        }

        return DefWindowProcA(hwnd, msg, wp, lp);
    }
    void open(const typename Win32Window<W1>::OpenConfig & config) {
        Win32Window<W1>::open(config);

        RECT window_rect;
        GetWindowRect(win32.window_handle, &window_rect);
        SetWindowPos(win32.window_handle, NULL, window_rect.left, window_rect.top,
                     window_rect.right - window_rect.left,
                     window_rect.bottom - window_rect.top, SWP_FRAMECHANGED);

        DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
        DwmSetWindowAttribute(win32.window_handle, DWMWA_NCRENDERING_POLICY, &policy,
                              sizeof(policy));

        MARGINS margins{
            .cxLeftWidth    = 0,
            .cxRightWidth   = 0,
            .cyTopHeight    = 1,
            .cyBottomHeight = 0,
        };

        DwmExtendFrameIntoClientArea(win32.window_handle, &margins);

        cursor_arrow    = LoadCursorA(nullptr, IDC_ARROW);
        cursor_hand     = LoadCursorA(nullptr, IDC_HAND);
        cursor_sizens   = LoadCursorA(nullptr, IDC_SIZENS);
        cursor_sizewe   = LoadCursorA(nullptr, IDC_SIZEWE);
        cursor_sizenwse = LoadCursorA(nullptr, IDC_SIZENWSE);
        cursor_sizenesw = LoadCursorA(nullptr, IDC_SIZENESW);

        start_time = std::chrono::steady_clock::now();
    }
    void close() {
        Win32Window<W1>::close();
    }
    void update() {
        Win32Window<W1>::update();

        POINT mp;
        GetCursorPos(&mp);
        i32vec2 currentpos{mp.x, mp.y};
        RECT    r;
        GetWindowRect(win32.window_handle, &r);
        i32rect window_rect{{r.left, r.top}, {r.right, r.bottom}};

        auto    hovered_edge = get_hovered_edge(window_rect, currentpos);
        HCURSOR hovered_edge_cursor_table[]{
            cursor_arrow,  cursor_sizewe,   cursor_sizewe,   nullptr,
            cursor_sizens, cursor_sizenwse, cursor_sizenesw, nullptr,
            cursor_sizens, cursor_sizenesw, cursor_sizenwse,
        };
        SetCursor(hovered_edge_cursor_table[hovered_edge]);

        if (mouse_down) {
            drag(grab_mode, pos, dim, min_dim, currentpos, mouse_down_point);
            MoveWindow(win32.window_handle, pos.x, pos.y, dim.x, dim.y, false);
            if (!GetAsyncKeyState(VK_LBUTTON)) mouse_down = false;
        }

        auto hdc = GetDC(win32.window_handle);
        RECT client_rect{-1, -1, -1, -1};
        GetClientRect(win32.window_handle, &client_rect);

        auto now     = std::chrono::steady_clock::now();
        auto elapsed = now - start_time;
        auto time    = std::chrono::duration<double>(elapsed).count();

        HBRUSH background_brush =
            CreateSolidBrush(RGB(255 * (sin(time * 5) + 1) / 2, 0, 255));
        FillRect(hdc, &client_rect, background_brush);
        DeleteObject(background_brush);
    }
};

template <> struct fmt::formatter<glm::mat4> {
    template <typename ParseContext> constexpr auto parse(ParseContext & ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::mat4 & m, FormatContext & ctx) {
        // clang-format off
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[0][0], m[0][1], m[0][2], m[0][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[1][0], m[1][1], m[1][2], m[1][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[2][0], m[2][1], m[2][2], m[2][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[3][0], m[3][1], m[3][2], m[3][3]);
        // clang-format on
        return ctx.out();
    }
};

template <> struct fmt::formatter<mat<f32, 4, 4>> {
    template <typename ParseContext> constexpr auto parse(ParseContext & ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const mat<f32, 4, 4> & m, FormatContext & ctx) {
        // clang-format off
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[0][0], m[0][1], m[0][2], m[0][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[1][0], m[1][1], m[1][2], m[1][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[2][0], m[2][1], m[2][2], m[2][3]);
        fmt::format_to(ctx.out(), "{: .3f}, {: .3f}, {: .3f}, {: .3f}\n", m[3][0], m[3][1], m[3][2], m[3][3]);
        // clang-format on
        return ctx.out();
    }
};

int main() {
    W1 w1{};
    w1.register_class("w1");
    w1.open({.pos{150, 120}, .dim{300, 200}});
    w1.show();
    while (true) {
        w1.update();
        if (!w1.is_open()) break;
        // std::this_thread::sleep_for(1ms);
    }

    // {
    //     auto m1 = glm::translate(glm::mat4(1), {1, 1, 1});
    //     auto m2 = glm::rotate(glm::mat4(1), 1.0f, {0, 1, 0});
    //     auto m4 = m1 * m2;
    //     std::cout << fmt::format("{}\n{}\n{}----------------------------------\n", m1,
    //     m2, m4);
    // }

    // {
    //     auto m1 = translate(mat<f32, 4, 4>::identity(), {1, 1, 1});
    //     auto m2 = rotate(mat<f32, 4, 4>::identity(), 1.0f, {0, 1, 0});
    //     auto m4 = m2 * m1;
    //     std::cout << fmt::format("{}\n{}\n{}----------------------------------\n", m1,
    //     m2, m4);
    // }
}
