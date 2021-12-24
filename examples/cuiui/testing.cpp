#include "custom_components/win32_window.hpp"

int main() {
    windowmanager_init();

    Window w1{};
    w1.open({.dim{800, 600}});

    while (true) {
        w1.update();
        if (!w1.is_open()) break;

        w1.bind_render_ctx();
        glViewport(0, 0, w1.dim(0), w1.dim(1));
        glClearColor(0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();
        w1.swap_buffers();
    }
}
