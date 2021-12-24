#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "impl/imgui_impl_glfw.h"
#include "impl/imgui_impl_opengl3.h"

#include <iostream>
#include <stdexcept>

int main() try {
    glfwInit();

    auto glfw_window_ptr = glfwCreateWindow(800, 600, "title", nullptr, nullptr);
    if (!glfw_window_ptr) throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(glfw_window_ptr);
    gladLoadGLLoader(GLADloadproc(glfwGetProcAddress));
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // | ImGuiConfigFlags_ViewportsEnable;

    ImFont * font1 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(glfw_window_ptr, true);
    ImGui_ImplOpenGL3_Init();

    while (true) {
        glfwPollEvents();
        if (glfwWindowShouldClose(glfw_window_ptr)) break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(glfw_window_ptr, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow * backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(glfw_window_ptr);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(glfw_window_ptr);
    glfwTerminate();
} catch (const std::runtime_error & e) {
    std::cerr << "[ERROR]: " << e.what() << std::endl;
}
