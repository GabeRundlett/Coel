#pragma once

#include <string>
#include <memory>
#include <array>
#include <unordered_map>
#include <stack>

#if defined(_WIN32)
#include <cuiui/platform/win32.hpp>
#endif

namespace cuiui {
    using WindowDim = std::array<uint32_t, 2>;

    struct WindowState {
        native::WindowState native;
        WindowDim           dim;
    };

    struct WindowConfig {
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
        void open(const WindowConfig & config = {});
        void close();

        WindowDim             dim();
        WindowDim::value_type dim(size_t idx);

        friend class UiContext;

        // TODO:
        void bind_render_ctx();
        void swap_buffers();
    };

    class UiContainer {
      private:
    };

    class UiContext {
      private:
        // UiContainer root;
        using WindowCollection = std::unordered_map<std::string_view, Window>;
        WindowCollection windows;

      public:
        UiContext();
        ~UiContext() = default;

        UiContext(const UiContext &) = delete;
        UiContext & operator=(const UiContext &) = delete;

        UiContext(UiContext &&) = default;
        UiContext & operator=(UiContext &&) = default;

        Window window(std::string_view id, const WindowConfig & config = {});
    };
} // namespace cuiui
