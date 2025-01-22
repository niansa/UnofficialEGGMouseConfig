#pragma once



struct MouseConfig {
    const char *name;
} constexpr mouseConfig {
#if !defined(PRODUCTID)
#define PRODUCT_WIRELESS
    "stub"

#elif PRODUCTID == 0x1966
#define PRODUCT_XM2_8K
    "XM2 8k"

#elif PRODUCTID == 0x1976
#define PRODUCT_OP1_8K_PURPLE_FROST
#define HAS_GLASS_MODE
#define HAS_MOTION_SYNC_AT_8K
    "OP1 8k Purple Frost"

#endif
};
