#include "application.hpp"
#include "device.hpp"

#include <string>
#include <optional>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <linux/limits.h>
#include <imgui.h>
#include <hidapi/hidapi.h>
#include <GL/gl.h>

using namespace ImGui;


template<std::integral T, T v>
T *ptrFromConst() {
    static T fres = v;
    return &fres;
}


Application::Application() {
    readConfig();

    // Discord (Dark) style by BttrDrgn from ImThemes
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2117647081613541f, 0.2235294133424759f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.2313725501298904f, 0.6470588445663452f, 0.364705890417099f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.125490203499794f, 0.1333333402872086f, 0.1450980454683304f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3294117748737335f, 0.3607843220233917f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.407843142747879f, 0.4274509847164154f, 0.4509803950786591f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2705882489681244f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.2588235437870026f, 0.2745098173618317f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.3607843220233917f, 0.4000000059604645f, 0.4274509847164154f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.05098039284348488f, 0.4196078479290009f, 0.8588235378265381f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.3450980484485626f, 0.3960784375667572f, 0.9490196108818054f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

void Application::readConfig() {
    config = Device::readConfig();
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
            InputScalar("X", ImGuiDataType_U16, &cpi.x, ptrFromConst<int, 50>(), ptrFromConst<int, 100>());
            cpi.y = cpi.x;
        }
        PopID();

        cpi.x = std::clamp(cpi.x + 25, 50, 26000)/50*50;
        cpi.y = std::clamp(cpi.y + 25, 50, 26000)/50*50;
    }
}

void Application::advancedSettings() {
    if (config->polling_rate_divider <= 1) {
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
        fw_version = Device::getVersion();

    SeparatorText("Unofficial EGG Mouse Config");
    TextUnformatted("Software: " PROJECT_VERSION);
    Text("HIDAPI: %s", hid_version_str());
    Text("OpenGL: %s", glGetString(GL_RENDERER));
    TextUnformatted("Compiler: " COMPILER_VERSION);

    SeparatorText("XM2 8k");
    Text("Firmware: %s", fw_version->c_str());
    if (Button("Factory Reset")) {
        Device::factoryReset();
        readConfig();
    }

    SeparatorText("Settings Backup");
    if (Button("Export")) {
        const auto path = fileDialog(true);
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char *>(&*config), sizeof(*config));
    }
    SameLine();
    if (Button("Import")) {
        const auto path = fileDialog(false);
        std::ifstream file(path, std::ios::binary);
        file.read(reinterpret_cast<char *>(&*config), sizeof(*config));
    }
}

std::string Application::fileDialog(bool saveMode) {
    char filename[1024];
    FILE *f = popen(("zenity --file-selection"+std::string(saveMode?" --save":"")).c_str(), "r");
    fgets(filename, 1024, f);
    filename[strlen(filename) - 1] = 0;
    std::string file(filename);
    return file;
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
                    basicSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Advanced")) {
                    advancedSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Experimental")) {
                    experimentalSettings();
                    EndTabItem();
                }
                if (BeginTabItem("Info")) {
                    info();
                    EndTabItem();
                    show_apply = false;
                }
                EndTabBar();
            }
            if (show_apply) {
                Separator();
                if (Button("Apply")) {
                    Device::writeConfig(config.value());
                    onNewConfig();
                }
            }
        } else {
            TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Waiting for device...");
            Spacing();
            if (Button("Use sudo")) {
                // Try for permission if config couldn't be read
                if (!config.has_value() && getuid() != 0) {
                    char self[PATH_MAX];
                    ssize_t self_len = readlink("/proc/self/exe", self, sizeof(self));
                    if (self_len >= 0) {
                        self[self_len] = '\0';
                        execlp("pkexec", "pkexec", self, nullptr);
                        execlp("gksudo", "gksudo", self, nullptr);
                        execlp("x-terminal-emulator", "terminal", "-e", "sudo", "setsid", self, nullptr);
                    }
                }
            }
            readConfig();
        }
    }
    End();
    PopStyleVar(1);
}
