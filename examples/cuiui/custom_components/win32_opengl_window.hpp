#pragma once
#include "win32_window.hpp"
#include <glad/glad.h>

template <typename T> struct Win32OpenGLWindow : Win32Window<T> {
    struct OpenGL {
        HDC   device_ctx;
        HGLRC render_ctx;
    } opengl;

    void open(const typename Win32Window<T>::OpenConfig & config) {
        Win32Window<T>::open(config);
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
        opengl.device_ctx  = GetDC(Win32Window<T>::win32.window_handle);
        auto choose_format = ChoosePixelFormat(opengl.device_ctx, &pfd);
        SetPixelFormat(opengl.device_ctx, choose_format, &pfd);
        opengl.render_ctx = wglCreateContext(opengl.device_ctx);
        wglMakeCurrent(opengl.device_ctx, opengl.render_ctx);
        // ((BOOL(WINAPI *)(i32))wglGetProcAddress("wglSwapIntervalEXT"))(0);
        UpdateWindow(Win32Window<T>::win32.window_handle);
        gladLoadGL();
    }
    void close() {
        wglDeleteContext(opengl.render_ctx);
        ReleaseDC(Win32Window<T>::win32.window_handle, opengl.device_ctx);
        Win32Window<T>::close();
    }
    void bind_render_ctx() {
        wglMakeCurrent(opengl.device_ctx, opengl.render_ctx);
    }
    void swap_buffers() {
        SwapBuffers(opengl.device_ctx);
    }
};
