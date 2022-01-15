#pragma once
#include "window.hpp"
#include <iostream>
#include <concepts>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

template <typename T>
concept HasWin32WindowProc = requires(T & wb) {
    { wb.win32_window_proc(HWND{}, 0, 0, 0) } -> std::same_as<LRESULT>;
};

template <typename UserWindow> struct Win32Window : Window {
    struct Win32 {
        HWND         window_handle     = nullptr;
        const char * window_class_name = nullptr;
        bool         should_close = false, is_open = false;
    } win32;

    LRESULT win32_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
        case WM_CLOSE: win32.should_close = true; break;
        }
        if constexpr (HasWin32WindowProc<UserWindow>)
            return ((UserWindow *)(this))->win32_window_proc(hwnd, msg, wp, lp);
        return DefWindowProcA(hwnd, msg, wp, lp);
    }

    struct OpenConfig {
        i32vec2 pos;
        i32vec2 dim;
    };
    void update() {
        if (win32.should_close) close();
        if (win32.is_open) {
            MSG msg;
            while (PeekMessageA(&msg, win32.window_handle, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
    }
    bool is_open() {
        return win32.is_open;
    }
    void register_class(const char * name) {
        auto app_instance_handle = GetModuleHandleA(nullptr);
        win32.window_class_name  = name;
        WNDCLASSEXA win32_window_class{
            .cbSize      = sizeof(WNDCLASSEXA),
            .style       = 0,
            .lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
                if constexpr (HasWin32WindowProc<UserWindow>) {
                    if (msg == WM_NCCREATE) {
                        auto & cs = *(CREATESTRUCTA *)lp;
                        SetWindowLongPtrA(hwnd, GWLP_USERDATA,
                                          (LONG_PTR)cs.lpCreateParams);
                    }
                    auto window_ptr =
                        (Win32Window<UserWindow> *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
                    if (window_ptr)
                        return window_ptr->win32_window_proc(hwnd, msg, wp, lp);
                }
                return DefWindowProcA(hwnd, msg, wp, lp);
            },
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = app_instance_handle,
            .hIcon         = nullptr,
            .hCursor       = nullptr,
            .hbrBackground = CreateSolidBrush(RGB(255, 0, 255)),
            .lpszMenuName  = nullptr,
            .lpszClassName = win32.window_class_name,
            .hIconSm       = nullptr,
        };
        RegisterClassExA(&win32_window_class);
    }
    void open(const OpenConfig & config = {}) {
        auto app_instance_handle = GetModuleHandleA(nullptr);
        pos = config.pos, dim = config.dim;
        win32.window_handle =
            CreateWindowExA(0, win32.window_class_name, "cuiui default window title",
                            WS_OVERLAPPEDWINDOW, pos.x, pos.y, dim.x, dim.y, nullptr,
                            nullptr, app_instance_handle, (LPVOID)this);
        win32.should_close = false;
        win32.is_open      = true;
        SetWindowLongPtrA(win32.window_handle, GWLP_USERDATA, (LONG_PTR)this);
    }
    void close() {
        DestroyWindow(win32.window_handle);
        win32.should_close = false;
        win32.is_open      = false;
    }
    void show() {
        ShowWindow(win32.window_handle, SW_SHOW);
    }
};
