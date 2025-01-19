#include "device.hpp"

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

HidDevice openDevice() noexcept {
    auto fres = hid_open(0x3367, 0x1966, nullptr);
    if (!fres)
        fprintf(stderr, "hidapi open error: %ls\n", hid_error(fres));
    return fres;
}
}

bool writeConfig(ConfigData& config) noexcept {
    auto device = openDevice();
    if (!device)
        return false;

    config.op = OpCode::storeConfig;
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&config), sizeof(config));
    return written == sizeof(config);
}

std::optional<ConfigData> readConfig() noexcept{
    auto device = openDevice();
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

bool factoryReset() noexcept {
    auto device = openDevice();
    if (!device)
        return false;

    CommandData command{OpCode::factoryReset};
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    return written == sizeof(command);
}
}
