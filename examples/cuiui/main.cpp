#include <cuiui/cuiui.hpp>

#include <chrono>
#include <thread>
#include <iostream>
#include <fmt/format.h>

#include <glad/glad.h>

int main() {
    auto ui_ctx = cuiui::UiContext();

    using namespace std::literals;
    using Clock   = std::chrono::high_resolution_clock;
    auto prev_now = Clock::now();

    auto     prev_title_update_time = prev_now;
    uint64_t frames_since_last      = 0;

    std::string subwindow_id_string = "sub";

    while (true) {
        auto now       = Clock::now();
        auto elapsed   = now - prev_now;
        prev_now       = now;
        auto elapsed_s = std::chrono::duration<double>(elapsed).count();
        auto est_fps   = 1.0 / elapsed_s;

        ++frames_since_last;

        bool should_close = false;
        for (auto [id_string, col_r] :
             {std::pair{"w1"sv, 0xff}, std::pair{"w2"sv, 0x64}}) {
            auto window = ui_ctx.window(id_string, {.dim{400, 400}});
            if (!window.is_open()) {
                should_close = true;
                continue;
            }

            subwindow_id_string = "sub";
            subwindow_id_string += id_string;
            auto subwindow = ui_ctx.window(subwindow_id_string);

            window.bind_render_ctx();
            glClearColor(col_r / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBegin(GL_TRIANGLES);
            glVertex2f(-0.5f, -0.5f);
            glVertex2f(0.0f, 0.5f);
            glVertex2f(0.5f, -0.5f);
            glEnd();
            window.swap_buffers();

            if (now - prev_title_update_time > 100ms) {
                // auto & title = window.get_title();
                // title.clear();
                // fmt::format_to(std::back_inserter(title), "fps={}",
                //                frames_since_last * 10);
                // window.update_title();

                prev_title_update_time = now;
                frames_since_last      = 0;
            }

            // std::this_thread::sleep_for(1ms);
        }
        if (should_close) break;
    }
}
