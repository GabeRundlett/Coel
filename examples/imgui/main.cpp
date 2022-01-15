#include <imgui.h>
#include "impl/imgui_impl_wm.h"
#include "impl/imgui_impl_opengl3.h"
#include <iostream>
int main() try {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ImFont * font1 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 16);
    ImGui::StyleColorsDark();
    imgui_impl_wm::init();
    auto window = imgui_impl_wm::setup_window();
    ImGui_ImplOpenGL3_Init();
    while (true) {
        if (imgui_impl_wm::update(window)) break;
        ImGui_ImplOpenGL3_NewFrame();
        imgui_impl_wm::new_frame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        glClearColor(0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            imgui_impl_wm::update_viewports();
        imgui_impl_wm::swap_buffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    imgui_impl_wm::shutdown();
    ImGui::DestroyContext();
    imgui_impl_wm::close(window);
} catch (const std::runtime_error & e) {
    std::cerr << "[ERROR]: " << e.what() << std::endl;
}
