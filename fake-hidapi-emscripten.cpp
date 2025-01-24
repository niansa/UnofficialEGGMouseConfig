#ifdef __EMSCRIPTEN__
#include "fake-hidapi-emscripten.hpp"

#include <emscripten.h>

namespace {
hid_device *fake_dev_ptr = reinterpret_cast<hid_device *>(0x12345);
bool is_open = false;
unsigned short current_product_id = 0xffff;
}

EM_ASYNC_JS(unsigned short, webhid_open_device, (unsigned vendorId), {
    try {
        var hidDevices = await navigator.hid.requestDevice({filters: [{vendorId: vendorId}]});
        if (hidDevices === undefined || hidDevices === null || hidDevices.length === 0) {
            return 0;
        }
        globalThis.hidDevice = null;
        for (var i = 0; i < hidDevices.length; i++) {
            globalThis.hidDevice = hidDevices[i];
            console.log(globalThis.hidDevice);
            try {
                await globalThis.hidDevice.open();
                await globalThis.hidDevice.sendFeatureReport(0xa1, new Uint8Array([123, 456, 789, 123]));
                console.log("Device selected.");
                break;
            } catch (e) {
                console.log("Skipped unsuitable device: "+e);
                try {
                    await globalThis.hidDevice.close();
                } catch (e) {}
                globalThis.hidDevice = null;
            }
        }
        if (globalThis.hidDevice === null) {
            console.log("Could not find HID device that accepts feature reports");
            return 0;
        }
        return globalThis.hidDevice.productId;
    } catch (e) {
        return 0;
    }
});
EM_ASYNC_JS(bool, webhid_send_feature_report, (const unsigned char *data, size_t length), {
    var jsdata = new Uint8Array(length-1);
    for (var i = 0; i < length; i++) {
        jsdata[i]=HEAPU8[data+1+i];
    }

    try {
        await globalThis.hidDevice.sendFeatureReport(HEAPU8[data], jsdata);
        return true;
    } catch (e) {
        console.log(e);
        return false;
    }
});
EM_ASYNC_JS(int, webhid_get_feature_report, (unsigned char *data, size_t length), {
    try {
        var dataView = await globalThis.hidDevice.receiveFeatureReport(HEAPU8[data]);
        const len = Math.min(dataView.byteLength, length);
        for (var i = 0; i < len; i++) {
            HEAP8[data+i]=dataView.getUint8(i);
        }
        return len - 1;
    } catch (e) {
        console.log(e);
        return -1;
    }
});
EM_ASYNC_JS(bool, webhid_close, (), {
    try {
        await globalThis.hidDevice.close();
    } catch (e) {}
});

hid_device *hid_open(unsigned short vendor_id, unsigned short& product_id) {
    if (is_open)
        return nullptr;
    if (product_id == current_product_id) {
        is_open = true;
        return fake_dev_ptr;
    }
    webhid_close();
    unsigned short pid = webhid_open_device(vendor_id);
    if (pid) {
        product_id = current_product_id = pid;
        is_open = true;
        return fake_dev_ptr;
    }
    return nullptr;
}
int hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length) {
    if (!is_open)
        return -1;
    if (webhid_send_feature_report(data, length))
        return length;
    return -1;
}
int hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length) {
    if (!is_open)
        return -1;
    return webhid_get_feature_report(data, length);
}
void hid_close(hid_device *dev) {
    is_open = false;
}
#endif
