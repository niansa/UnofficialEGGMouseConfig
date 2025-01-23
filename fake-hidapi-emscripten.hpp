#ifdef __EMSCRIPTEN__
#pragma once

#include <cstddef>


struct hid_device_;
typedef struct hid_device_ hid_device; /**< opaque hidapi structure */

hid_device *hid_open(unsigned short vendor_id, unsigned short& product_id);
int hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length);
int hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length);
void hid_close(hid_device *dev);

static inline const char *hid_version_str(void) {return "Fake HIDAPI for Emscripten";}
#endif
