#include "device.hpp"

#include <sstream>
#include <cstdio>
#include <cstring>
#include <hidapi/hidapi.h>



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

HidDevice openDevice(unsigned pid) noexcept {
    auto fres = hid_open(0x3367, pid, nullptr);
    return fres;
}
}

std::pair<unsigned, MouseConfig> getMouseConfig() noexcept {
    for (const auto& [pid, config] : mouseConfigs) {
        if (openDevice(pid))
            return {pid, config};
    }
    return {0, {}};
}

bool writeConfig(unsigned pid, ConfigData& config) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return false;

    config.op = OpCode::storeConfig;
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&config), sizeof(config));
    return written == sizeof(config);

}

std::optional<ConfigData> readConfig(unsigned pid) noexcept{
    auto device = openDevice(pid);
    if (!device)
        return {};

    CommandData command{OpCode::loadConfig};
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    if (written != sizeof(command))
        return {};

    ConfigData config;
    const auto read_ = hid_get_feature_report(device, reinterpret_cast<unsigned char *>(&config) - 1, sizeof(config) - 4);
    if (read_ != sizeof(config) - 5)
        return {};

    return config;
}

bool factoryReset(unsigned pid) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return false;

    CommandData command{OpCode::factoryReset};
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    return written == sizeof(command);
}

std::string getVersion(unsigned pid) noexcept {
    auto device = openDevice(pid);
    if (!device)
        return "";

    CommandData command{OpCode::getFwVersion};
    const auto written = hid_send_feature_report(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    if (written != sizeof(command))
        return "";

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
