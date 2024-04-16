#pragma once

#include "base/window.hpp"

namespace wen {

class Settings {
public:
    Settings() = default;

    Window::Info windowInfo;
    bool debug = false;
    std::string appName = "app name";
    uint32_t appVersion = 1;
    std::string engineName = "engine name";
    uint32_t engineVersion = 1;
    std::vector<std::string> deviceRequestedExtensions = {};

private:
};

extern std::shared_ptr<Settings> settings;

} // namespace wen