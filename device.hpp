#pragma once

#include "config.hpp"

#include <string>
#include <optional>
#include <cstdint>
#include <cstddef>


namespace Device {
enum class OpCode : uint16_t {
    storeConfig = 0x11a0,
    loadConfig = 0x12a1,
    factoryReset = 0x13a1,
    getFwVersion = 0x02a1
};

struct __attribute__((__packed__)) ConfigData {
    static constexpr unsigned cpi_count = 4,
                              button_config_count = 7;

    OpCode op;
    uint8_t pad0[19] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint8_t polling_rate_divider = 1; // polling_rate = 8KHz / polling_rate_divider
    struct FilterFlags {
        static constexpr uint8_t slamclickFilter = 0x01,
                                 motionJitterFilter = 0x10;
    };
    uint8_t filter_flags = FilterFlags::slamclickFilter;
    uint8_t pad1[2] = {0x00, 0x00};
    uint8_t lod = 2;
    bool angle_snapping = false;
    bool ripple_control = false;
    bool motion_sync = false;
    uint8_t pad2[1] = {0x2};
    uint8_t cpi_levels = 4;
    uint8_t pad3[20] = {0xff, 0xff, 0x0, 0x1, 0x1, 0x0, 0x0, 0xff, 0x1, 0x2, 0xff, 0x0, 0x0, 0x1, 0x3, 0x0, 0xff, 0x0, 0x1, 0x4};
    struct __attribute__((__packed__)) CPI {
        bool xy_split = true;
        uint16_t x = 800;
        uint16_t y = 900;
    } cpis[cpi_count];
    uint8_t pad4[6] = {0x0, 0x1, 0x0, 0x0, 0x0, 0x0};
    enum class ButtonIndex {
        left,
        right,
        middle,
        forward,
        back,
        _end
    };
    struct __attribute__((__packed__)) ButtonConfig {
        enum class SPDTMode : uint8_t {
            off,
            safe = 0xf0,
            speed = 0xf1
        };
        union __attribute__((__packed__)) {
            SPDTMode spdt;
            uint8_t multiclick = 8;
        };
        enum class MappingType : int8_t {
            mouse = 0,
            scroll = 1,
            keyboard = 2,
            cpi_loop = 9,
            cpi = 12,
            media = 32,
            disable = -1
        };
        enum class MouseKeys : uint8_t {
            left = 1,
            right = 2,
            middle = 4,
            back = 8,
            forward = 16,
        };
        enum class ScrollWheel : int8_t {
            up = 1,
            down = -1
        };
        enum class MediaKeys : uint8_t {
            playPause = 0xcd,
            next = 0xb5,
            previous = 0xb6,
            mute = 0xe2,
            volumeUp = 0xe9,
            volumeDown = 0xea,
            browser = 0x96,
            explorer = 0x94
        };
        struct __attribute__((__packed__)) Mapping {
            MappingType type = MappingType::mouse;
            union __attribute__((__packed__)) {
                uint8_t keyboard = 0;
                MouseKeys mouse;
                ScrollWheel scroll;
                MediaKeys media;
                CPI cpi;
            };
        } mapping;

        SPDTMode getSPDTMode() const {
            if (multiclick >= 0xf0)
                return spdt;
            return SPDTMode::off;
        }
    } button_configs[button_config_count];

    struct CustomFlags {
        static constexpr uint8_t experimental = 1 << 0;
    };
    uint16_t custom_flags = 0;
    uint8_t pad5[913];
};
static_assert(sizeof(bool) == 1);
static_assert(offsetof(ConfigData, polling_rate_divider) == 21);
static_assert(offsetof(ConfigData, filter_flags) == 22);
static_assert(offsetof(ConfigData, lod) == 25);
static_assert(offsetof(ConfigData, angle_snapping) == 26);
static_assert(offsetof(ConfigData, ripple_control) == 27);
static_assert(offsetof(ConfigData, motion_sync) == 28);
static_assert(offsetof(ConfigData, cpi_levels) == 30);
static_assert(offsetof(ConfigData, cpis[0].xy_split) == 51);
static_assert(offsetof(ConfigData, cpis[0].x) == 52);
static_assert(offsetof(ConfigData, cpis[0].y) == 54);
static_assert(offsetof(ConfigData, cpis[3].y) == 69);
static_assert(offsetof(ConfigData, button_configs[0].spdt) == 77);
static_assert(sizeof(ConfigData) == 1041);


struct __attribute__((__packed__)) CommandData {
    OpCode op;
    uint8_t pad0[62] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
};
static_assert(sizeof(CommandData) == 64);


std::pair<unsigned, MouseConfig> getMouseConfig() noexcept;
bool writeConfig(unsigned pid, ConfigData&) noexcept;
std::optional<ConfigData> readConfig(unsigned pid) noexcept;
bool factoryReset(unsigned pid) noexcept;
std::string getVersion(unsigned pid) noexcept;
}
