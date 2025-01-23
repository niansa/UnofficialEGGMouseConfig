#pragma once

#include "device.hpp"
#include "config.hpp"

#include <optional>


class Application {
    static constexpr unsigned max_polling_rate_divider = 128;

    std::optional<Device::ConfigData> config;
    std::optional<std::string> fw_version;

    std::pair<unsigned, MouseConfig> mouse_config {0, {}};

    bool split_xy;
    bool custom_polling_rate;

    void readConfig();
    void onNewConfig();

    void basicSettings();
    void advancedSettings();
    void experimentalSettings();
    void info();

    static std::string fileDialog(bool saveMode);

public:
    Application();

    void render();
};
