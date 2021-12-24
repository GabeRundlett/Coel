#include <cuiui/cuiui.hpp>

namespace cuiui {
    UiContext::UiContext() {
        native::windowmanager_init();
    }

    Window UiContext::window(std::string_view id, const WindowConfig & config) {
        auto iter = windows.find(id);
        if (iter == windows.end()) {
            auto & result = windows[id];
            result.open(config);
            return result;
        } else {
            auto & result = iter->second;
            result.update();
            return result;
        }
    }
} // namespace cuiui
