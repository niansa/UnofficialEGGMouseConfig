#pragma once

#include <map>



struct MouseConfig {
    const char *name;
    bool hasGlassMode = false;
    bool hasMotionSyncAt8k = false;
};

extern const std::map<unsigned short, MouseConfig> mouseConfigs;
