#include "config.hpp"


const std::map<unsigned short, MouseConfig> mouseConfigs {
    {0x1964, {
                 .name = "OP1 8k Standard",
                 .hasGlassMode = true,
                 .hasMotionSyncAt8k = true
             }},
    {0x1966, {
                 .name = "XM2 8k"
             }},
    {0x1976, {
                 .name = "OP1 8k Purple Frost",
                 .hasGlassMode = true,
                 .hasMotionSyncAt8k = true
             }},
    {0x1978, {
                 .name = "OP1 8k v2 (Experimental)",
                 .hasGlassMode = true,
                 .hasMotionSyncAt8k = true
             }},
};
