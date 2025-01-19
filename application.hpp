#pragma once

#include "device.hpp"

#include <optional>


class Application {
    static constexpr unsigned max_polling_rate_divider = 128;

    std::optional<Device::ConfigData> config;

    bool split_xy;
    bool custom_polling_rate;

    void readConfig();
    void onNewConfig();

    void basicSettings();
    void advancedSettings();
    void experimentalSettings();
    void info();

public:
    Application();

    void render();
};
