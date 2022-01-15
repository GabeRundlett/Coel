#pragma once

#include <stdexcept>
#include <glad/glad.h>

// #define USING_GLFW 1
// #define USING_WIN32 1
#define USING_CUIUI 1

#if USING_GLFW
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#elif USING_WIN32
#include "imgui_impl_win32.h"
#elif USING_CUIUI
#include "../../cuiui/custom_components/win32_window.hpp"
#include "imgui_impl_win32.h"
#endif

namespace imgui_impl_wm {
    struct Window {
#if USING_GLFW
        GLFWwindow * glfw_window_ptr;
#elif USING_WIN32
#elif USING_CUIUI
        static inline ::Window prev_window;

        ::Window cuiui_window;
#endif
    };

    inline Window setup_window() {
        Window result;

#if USING_GLFW
        result.glfw_window_ptr = glfwCreateWindow(800, 600, "title", nullptr, nullptr);
        if (!result.glfw_window_ptr) throw std::runtime_error("Failed to create window");
        glfwMakeContextCurrent(result.glfw_window_ptr);
        gladLoadGLLoader(GLADloadproc(glfwGetProcAddress));
        glfwSwapInterval(1);
        ImGui_ImplGlfw_InitForOpenGL(result.glfw_window_ptr, true);
#elif USING_WIN32
#elif USING_CUIUI
        result.cuiui_window.open({.dim{800, 600}});
        Window::prev_window = result.cuiui_window;

        ImGui_ImplWin32_Init(result.cuiui_window.state->win32.window_handle);
#endif

        return result;
    }

    inline void init() {
#if USING_GLFW
        glfwInit();
#elif USING_WIN32
#elif USING_CUIUI
        windowmanager_init();
#endif
    }

    inline bool update(Window window) {
#if USING_GLFW
        glfwPollEvents();
        return glfwWindowShouldClose(window.glfw_window_ptr);
#elif USING_WIN32
#elif USING_CUIUI
        window.cuiui_window.update();
        return !window.cuiui_window.is_open();
#endif
    }

    inline void new_frame() {
#if USING_GLFW
        ImGui_ImplGlfw_NewFrame();
#elif USING_WIN32
#elif USING_CUIUI
        ImGui_ImplWin32_NewFrame();
#endif
    }

    inline void update_viewports() {
#if USING_GLFW
        GLFWwindow * backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
#elif USING_WIN32
#elif USING_CUIUI
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        Window::prev_window.bind_render_ctx();
#endif
    }

    inline void swap_buffers(Window window) {
#if USING_GLFW
        glfwSwapBuffers(window.glfw_window_ptr);
#elif USING_WIN32
#elif USING_CUIUI
        window.cuiui_window.swap_buffers();
#endif
    }

    inline void shutdown() {
#if USING_GLFW
        ImGui_ImplGlfw_Shutdown();
#elif USING_WIN32
#elif USING_CUIUI
#endif
    }

    inline void close(Window window) {
#if USING_GLFW
        glfwDestroyWindow(window.glfw_window_ptr);
        glfwTerminate();
#elif USING_WIN32
#elif USING_CUIUI
#endif
    }
} // namespace imgui_impl_wm
