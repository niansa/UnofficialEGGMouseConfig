#include "device.hpp"

#include <thread>
#include <chrono>
#include <sstream>
#include <cstdio>
#include <cstring>
#ifdef __EMSCRIPTEN__
#   include "fake-hidapi-emscripten.hpp"
#else
#   include <hidapi/hidapi.h>
#endif



namespace Device {
namespace {
class HidDevice {
    hid_device *dev;

public:
    HidDevice(hid_device *device) : dev(device) {}
    HidDevice(HidDevice&& o) : dev(o.dev) {
        o.dev = nullptr;
    }
    HidDevice(const HidDevice&) = delete;
    ~HidDevice() {
        if (dev)
            hid_close(dev);
    }

    operator bool() const {
        return dev;
    }
    operator hid_device *() {
        return dev;
    }
};

HidDevice openDevice(unsigned short pid) noexcept {
#ifndef __EMSCRIPTEN__
    return hid_open(0x3367, pid, nullptr);
#else
    return hid_open(0x3367, pid);
#endif
}
}

std::pair<unsigned short, MouseConfig> getMouseConfig() noexcept {
#ifndef __EMSCRIPTEN__
    for (const auto& [pid, config] : mouseConfigs) {
        if (openDevice(pid))
            return {pid, config};
    }
#else
    unsigned short pid;
    HidDevice device = hid_open(0x3367, pid);
    auto res = mouseConfigs.find(pid);
    if (res != mouseConfigs.end())
        return *res;
#endif
    return {0, {}};
}

bool writeConfig(unsigned short pid, ConfigData& config) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return false;

    config.op = OpCode::storeConfig;
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&config), sizeof(config));
    return written == sizeof(config);

}

std::optional<ConfigData> readConfig(unsigned short pid) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return {};

    CommandData command{OpCode::loadConfig};
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    if (written != sizeof(command))
        return {};

    ConfigData config;
    const auto read_ = hid_get_feature_report(device, reinterpret_cast<unsigned char *>(&config), sizeof(config) - 3);
    if (read_ != sizeof(config) - 4)
        return {};

    return config;
}

bool factoryReset(unsigned short pid) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return false;

    CommandData command{OpCode::factoryReset};
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    return written == sizeof(command);
}

std::string getVersion(unsigned short pid) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return "";

    CommandData command{OpCode::getFwVersion};
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    if (written != sizeof(command))
        return "";

    command.op = OpCode::none;
    const auto read_ = hid_get_feature_report(device, reinterpret_cast<unsigned char *>(&command), sizeof(command) - 1);
    if (read_ != sizeof(command) - 2)
        return "";

    const auto major = command.pad0[15],
               minor = command.pad0[16];
    std::stringstream stream;
    stream << std::hex << unsigned(minor) << '.' << unsigned(major);
    return stream.str();
}
}
