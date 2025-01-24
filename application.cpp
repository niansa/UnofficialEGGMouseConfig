#include "application.hpp"
#include "device.hpp"
#include "config.hpp"
#include "platform.hpp"

#include <string>
#include <optional>
#include <algorithm>
#include <fstream>
#include <imgui.h>
#ifdef __EMSCRIPTEN__
#   include "fake-hidapi-emscripten.hpp"
#else
#   include <hidapi/hidapi.h>
#endif
#include <GL/gl.h>

#include "themes.inc"

using namespace ImGui;


template<std::integral T, T v>
T *ptrFromConst() {
    static T fres = v;
    return &fres;
}


Application::Application() {
    readConfig();
}

void Application::readConfig() {
    if (mouse_config.first == 0) {
        mouse_config = Device::getMouseConfig();

        // Select theme
        switch (mouse_config.first) {
        case 0x1976: Themes::purpleComfy(); break;
        default: Themes::defaultTheme();
        }
    }
    config = Device::readConfig(mouse_config.first);
    if (!config.has_value())
        return;

    onNewConfig();
}

void Application::onNewConfig() {
    split_xy = false;
    for (unsigned cpi = 0; cpi != config->cpi_count; ++cpi) {
        if ((config->cpis[cpi].xy_split = config->cpis[cpi].x != config->cpis[cpi].y))
            split_xy = true;
    }

    custom_polling_rate = true;
    for (unsigned divider = 1; divider != max_polling_rate_divider; divider *= 2) {
        if (divider == config->polling_rate_divider)
            custom_polling_rate = false;
    }
}

void Application::basicSettings() {
    InputScalar("LOD", ImGuiDataType_U8, &config->lod, ptrFromConst<int, 1>(), nullptr, "%i mm");
    config->lod = std::clamp<uint8_t>(config->lod, 1, 2);

    Checkbox("Ripple Control", &config->ripple_control);
    SameLine();
    Checkbox("Angle Snapping", &config->angle_snapping);

    Spacing();

    InputScalar("CPI Levels", ImGuiDataType_U8, &config->cpi_levels, ptrFromConst<int, 1>());
    config->cpi_levels = std::clamp<uint8_t>(config->cpi_levels, 1, config->cpi_count);

    Checkbox("Split X/Y", &split_xy);

    for (unsigned idx = 0; idx != config->cpi_levels; ++idx) {
        auto& cpi = config->cpis[idx];
        SeparatorText(("CPI "+std::to_string(idx + 1)).c_str());

        PushID(idx);
        if (split_xy) {
            InputScalar("X", ImGuiDataType_U16, &cpi.x, ptrFromConst<int, 50>(), ptrFromConst<int, 100>());
            InputScalar("Y", ImGuiDataType_U16, &cpi.y, ptrFromConst<int, 50>(), ptrFromConst<int, 100>());
        } else {
            InputScalar("X/Y", ImGuiDataType_U16, &cpi.x, ptrFromConst<int, 50>(), ptrFromConst<int, 100>());
            cpi.y = cpi.x;
        }
        PopID();

        cpi.x = std::clamp(cpi.x + 25, 50, 26000)/50*50;
        cpi.y = std::clamp(cpi.y + 25, 50, 26000)/50*50;
    }
}

void Application::advancedSettings() {
    if (!mouse_config.second.hasMotionSyncAt8k && config->polling_rate_divider <= 1) {
        config->motion_sync = false;
        BeginDisabled(true);
    } else{
        BeginDisabled(false);
    }
    Checkbox("Motion Sync", &config->motion_sync);
    EndDisabled();

    const auto get_polling_rate_str = [] (unsigned rate) {
        return std::to_string(8000/(rate?rate:1))+" Hz";
    };

    BeginDisabled(custom_polling_rate);
    if (BeginCombo("Polling Rate", get_polling_rate_str(config->polling_rate_divider).c_str())) {
        for (unsigned divider = 1; divider != max_polling_rate_divider; divider *= 2) {
            const bool is_selected = divider == config->polling_rate_divider;
            if (Selectable(get_polling_rate_str(divider).c_str(), is_selected))
                config->polling_rate_divider = divider;
            if (is_selected)
                SetItemDefaultFocus();
        }
        EndCombo();
    }
    EndDisabled();

    {
        bool slamclick_filter = static_cast<uint8_t>(config->filter_flags) & static_cast<uint8_t>(Device::ConfigData::FilterFlags::slamclickFilter);
        bool motion_jitter_filter = static_cast<uint8_t>(config->filter_flags) & static_cast<uint8_t>(Device::ConfigData::FilterFlags::motionJitterFilter);
        Checkbox("Slamclick Filter", &slamclick_filter);
        SameLine();
        Checkbox("Motion Jitter Filter", &motion_jitter_filter);
        config->filter_flags =
            (slamclick_filter ? Device::ConfigData::FilterFlags::slamclickFilter : 0) |
            (motion_jitter_filter ? Device::ConfigData::FilterFlags::motionJitterFilter : 0);
    }

    Spacing();

    SeparatorText("Multiclick Filter");
    for (unsigned idx = 0; idx != static_cast<unsigned>(Device::ConfigData::ButtonIndex::_end); ++idx) {
        auto& button = config->button_configs[idx];

        const char *label;
        switch (static_cast<Device::ConfigData::ButtonIndex>(idx)) {
        using enum Device::ConfigData::ButtonIndex;
        case left: label = "Left Click"; break;
        case right: label = "Right Click"; break;
        case middle: label = "Middle Click"; break;
        case forward: label = "Forward Button"; break;
        case back: label = "Back Button"; break;
        case _end: __builtin_unreachable();
        }

        using SPDTMode = Device::ConfigData::ButtonConfig::SPDTMode;
        const bool noSPDT = button.getSPDTMode() == SPDTMode::off;
        BeginDisabled(!noSPDT);
        InputScalar(label, ImGuiDataType_U8, noSPDT?&button.multiclick:ptrFromConst<uint8_t, 8>(), ptrFromConst<int, 1>(), ptrFromConst<int, 10>());
        EndDisabled();
        if (noSPDT)
            button.multiclick = std::clamp<uint8_t>(button.multiclick, 0, 25);

        if (idx < 2) {
            const auto get_spdt_str = [] (SPDTMode mode) {
                switch (mode) {
                using enum SPDTMode;
                case safe: return "GX Safe";
                case speed: return "GX Speed";
                case off:
                default: return "Off";
                }
            };
            const auto get_spdt_by_idx = [] (unsigned idx) {
                switch (idx) {
                case 0: return SPDTMode::off;
                case 1: return SPDTMode::speed;
                case 2: return SPDTMode::safe;
                default: __builtin_unreachable();
                }
            };

            PushID(idx);
            if (BeginCombo("SPDT", get_spdt_str(button.getSPDTMode()))) {
                for (unsigned mode_idx = 0; mode_idx != 3; ++mode_idx) {
                    const auto mode = get_spdt_by_idx(mode_idx);
                    const bool is_selected = button.spdt == mode;
                    if (Selectable(get_spdt_str(mode), is_selected)) {
                        if (mode != SPDTMode::off)
                            button.spdt = mode;
                        else
                            button.multiclick = 8;
                    }
                    if (is_selected)
                        SetItemDefaultFocus();
                }
                EndCombo();
            }
            PopID();

            Spacing();
            Spacing();
        }
    }
}

void Application::experimentalSettings() {
    TextUnformatted("Experimental features can brick your mouse");
    if (~config->custom_flags & Device::ConfigData::CustomFlags::experimental) {
        if (Button("Accept risk"))
            config->custom_flags |= Device::ConfigData::CustomFlags::experimental;
    } else {
        Separator();
        Checkbox("Custom Polling Rate Divider", &custom_polling_rate);
        BeginDisabled(!custom_polling_rate);
        InputScalar("##9845", ImGuiDataType_U8, &config->polling_rate_divider, ptrFromConst<int, 1>(), nullptr, "%u");
        if (config->polling_rate_divider == 0)
            config->polling_rate_divider = 1;
        Text("%u Hz", 8000 / config->polling_rate_divider);
        EndDisabled();
    }
}

void Application::info() {
    if (!fw_version.has_value())
        fw_version = Device::getVersion(mouse_config.first);

    SeparatorText("Unofficial EGG Mouse Config");
    TextUnformatted("Software: " PROJECT_VERSION);
    Text("HIDAPI: %s", hid_version_str());
    Text("OpenGL: %s", glGetString(GL_RENDERER));
    TextUnformatted("Compiler: " COMPILER_VERSION);

    Spacing();
    SeparatorText(mouse_config.second.name);
    Text("Firmware: %s", fw_version->c_str());
    if (Button("Factory Reset")) {
        Device::factoryReset(mouse_config.first);
        readConfig();
    }

    Spacing();
    SeparatorText("Settings Backup");
    if (Button("Export"))
        Platform::storeFile(reinterpret_cast<const char *>(&*config), sizeof(*config));
    SameLine();
    if (Button("Import"))
        Platform::loadFile(reinterpret_cast<char *>(&*config), sizeof(*config));

    Spacing();
    SeparatorText("Author");
    TextUnformatted("niansa (@tuxifan)");
    Spacing();
    Link("GitHub Repository", "https://github.com/niansa/UnofficialEGGMouseConfig");
}

void Application::Link(const char *label, const char *url) {
    if (Button(label))
        Platform::openLink(url);
}

void Application::render() {
    SetNextWindowPos(ImVec2(0.0f, 0.0f));
    SetNextWindowSize(GetIO().DisplaySize);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    Begin("Main Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    {
        if (config.has_value()) {
            bool show_apply = true;
            if (BeginTabBar("Main Tabs", ImGuiTabBarFlags_None)) {
                if (BeginTabItem("Basic")) {
                    Spacing();
                    basicSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Advanced")) {
                    Spacing();
                    advancedSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Experimental")) {
                    Spacing();
                    experimentalSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Info")) {
                    Spacing();
                    info();
                    EndTabItem();
                    show_apply = false;
                }
                EndTabBar();
            }
            if (show_apply) {
                Separator();
                if (Button("Apply")) {
                    Device::writeConfig(mouse_config.first, config.value());
                    onNewConfig();
                }
            }
        } else {
            TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Waiting for device...");
            if (!Platform::isElevated()) {
                Spacing();
                if (Button("Use sudo"))
                    // Try restarting with elevated permissions if config couldn't be read
                    Platform::restartElevated();
            }
#ifdef __EMSCRIPTEN__
            Button("Rescan");
#endif
            readConfig();
        }
    }
    End();
    PopStyleVar(1);
}
