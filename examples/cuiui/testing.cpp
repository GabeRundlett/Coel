#include "custom_components/win32_opengl_window.hpp"
#include "custom_components/utility.hpp"
#include <fmt/format.h>

#include <dwmapi.h>

struct W1 : Win32OpenGLWindow<W1> {
    bool    mouse_down : 1 = false;
    i32vec2 mouse_down_point;
    i32vec2 min_dim{200, 100};
    u32     grab_mode = 0;
    u32     vbo_id;
    u32     shader_id;

    LRESULT win32_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
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

    void init() {
        // enable transparency
        HRGN region = CreateRectRgn(0, 0, -1, -1);

        auto bb = DWM_BLURBEHIND{
            .dwFlags  = DWM_BB_ENABLE | DWM_BB_BLURREGION,
            .fEnable  = TRUE,
            .hRgnBlur = region,
        };

        DwmEnableBlurBehindWindow(win32.window_handle, &bb);
        DeleteObject(region);

        // create vbo and shaders
        auto verify_shader_compile = [](u32 id) {
            i32 success;
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            if (!success) {
                std::string info_log;
                info_log.resize(512);
                glGetShaderInfoLog(id, (GLsizei)info_log.size(), nullptr,
                                   info_log.data());
                auto message = fmt::format("Failed to compile shader\n{}", info_log);
                throw std::runtime_error(message.c_str());
            }
        };
        auto verify_shader_link = [](u32 id) {
            i32 success;
            glGetProgramiv(id, GL_LINK_STATUS, &success);
            if (!success) {
                std::string info_log;
                info_log.resize(512);
                glGetProgramInfoLog(id, (GLsizei)info_log.size(), nullptr,
                                    info_log.data());
                auto message = fmt::format("Failed to link shader program\n{}", info_log);
                throw std::runtime_error(message.c_str());
            }
        };

        glCreateBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        f32 data[]{-1, -1, -1, 1, 1, 1, 1, -1};
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, 0);

        shader_id = glCreateProgram();

        u32          vert_shader_id  = glCreateShader(GL_VERTEX_SHADER);
        const char * vert_source_str = R"glsl(#version 450
            layout (location = 0) in vec2 a_pos;
            void main() {
                gl_Position = vec4(a_pos, 0, 1);
            }
        )glsl";
        glShaderSource(vert_shader_id, 1, &vert_source_str, nullptr);
        glCompileShader(vert_shader_id);
        verify_shader_compile(vert_shader_id);

        u32          frag_shader_id  = glCreateShader(GL_FRAGMENT_SHADER);
        const char * frag_source_str = R"glsl(#version 450
            uniform vec2 window_dim;
            out vec4 o_col;
            vec2 calc_edge_uv(f32 edge_radius, vec2 input_uv, vec2 input_dim) {
                vec2 uv = input_uv;
                if (uv.x < edge_radius) {
                    uv.x = edge_radius - uv.x;
                } else if (uv.x > input_dim.x - edge_radius) {
                    uv.x = uv.x - input_dim.x + edge_radius;
                } else {
                    uv.x = 0;
                }
                if (uv.y < edge_radius) {
                    uv.y = edge_radius - uv.y;
                } else if (uv.y > input_dim.y - edge_radius) {
                    uv.y = uv.y - input_dim.y + edge_radius;
                } else {
                    uv.y = 0;
                }
                return uv;
            }
            void main() {

                vec2 shadow_uv = calc_edge_uv(72.0f, gl_FragCoord.xy, window_dim);
                vec2 uv = gl_FragCoord.xy / window_dim;
                f32 a = length(shadow_uv) / 72.0f;
                bool is_shadow = a > 0.0f;
                if (is_shadow) a += uv.y * 0.5f;
                a = clamp(a, 0.0f, 1.0f);
                vec3 col = vec3(0.1f);

                f32 round_radius = 8.0f;
                vec2 round_uv = calc_edge_uv(round_radius, gl_FragCoord.xy - 72.0f, window_dim - 144.0f);
                f32 round_a = length(round_uv) - round_radius + 1;
                round_a = clamp(round_a, 0, 1);
                // if (round_a > 1) discard;
                // col = mix(vec3(1), vec3(0), round_a);
                col = vec3(1);

                if (is_shadow) {
                    col = vec3(0.0f);
                    a = pow(1.0f - a, 2);
                    a *= 0.2f;
                } else {
                    a = 0.1;
                }
                a = clamp(a, 0, 1);
                col = clamp(col, 0, 1);

                o_col = vec4(col, a);
            }
        )glsl";
        glShaderSource(frag_shader_id, 1, &frag_source_str, nullptr);
        glCompileShader(frag_shader_id);
        verify_shader_compile(frag_shader_id);

        glAttachShader(shader_id, vert_shader_id);
        glAttachShader(shader_id, frag_shader_id);
        glLinkProgram(shader_id);
        glDeleteShader(vert_shader_id);
        glDeleteShader(frag_shader_id);
        verify_shader_link(shader_id);
    }

    void deinit() {
        // destroy vbo and shaders
        glDeleteProgram(shader_id);
        glDeleteBuffers(1, &vbo_id);
    }

    void draw() {
        bind_render_ctx();
        glViewport(0, 0, dim.x, dim.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glUseProgram(shader_id);
        glUniform2f(glGetUniformLocation(shader_id, "window_dim"), dim.x, dim.y);
        glDrawArrays(GL_QUADS, 0, 4);
        swap_buffers();
    }

    void update() {
        Win32OpenGLWindow<W1>::update();
        POINT mp;
        GetCursorPos(&mp);
        i32vec2 currentpos{mp.x, mp.y};
        if (mouse_down) {
            drag(grab_mode, pos, dim, min_dim, currentpos, mouse_down_point);
            MoveWindow(win32.window_handle, pos.x, pos.y, dim.x, dim.y, false);
            if (!GetAsyncKeyState(VK_LBUTTON)) mouse_down = false;
        }
    }
};

int main() try {
    W1 w1{};
    w1.register_class("w1");
    w1.open({.pos{100, 100}, .dim{500 + 144, 300 + 144}});
    w1.show();
    w1.init();
    while (true) {
        w1.update();
        if (!w1.is_open()) break;
        w1.draw();
    }
    w1.deinit();
    return EXIT_SUCCESS;
} catch (std::runtime_error & e) {
    std::cerr << e.what() << "\n";
    return EXIT_FAILURE;
}
