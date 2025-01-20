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

HidDevice openDevice() noexcept {
#ifdef PRODUCTID
    auto fres = hid_open(0x3367, PRODUCTID, nullptr);
    if (!fres)
        fprintf(stderr, "hidapi open error: %ls\n", hid_error(fres));
    return fres;
#else
    return nullptr;
#endif
}
}

bool writeConfig(ConfigData& config) noexcept {
#ifdef PRODUCTID
    auto device = openDevice();
    if (!device)
        return false;

    config.op = OpCode::storeConfig;
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&config), sizeof(config));
    return written == sizeof(config);
#else
    return true;
#endif
}

std::optional<ConfigData> readConfig() noexcept{
#ifdef PRODUCTID
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
#else
    return ConfigData();
#endif
}

bool factoryReset() noexcept {
#ifdef PRODUCTID
    auto device = openDevice();
    if (!device)
        return false;

    CommandData command{OpCode::factoryReset};
    const auto written = hid_write(device, reinterpret_cast<const unsigned char *>(&command), sizeof(command));
    return written == sizeof(command);
#else
    return true;
#endif
}

std::string getVersion() noexcept {
#ifdef PRODUCTID
    auto device = openDevice();
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
#else
    return "stub";
#endif
}
}
