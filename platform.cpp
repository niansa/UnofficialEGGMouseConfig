#include "platform.hpp"

#include <string>
#include <cstdio>
#include <cstring>
#ifndef __EMSCRIPTEN__
#   include <fstream>
#endif
#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#endif
#ifdef __linux__
#   include <unistd.h>
#   include <linux/limits.h>
#endif



namespace Platform {
namespace {
#ifndef __EMSCRIPTEN__
namespace Native {
std::string fileDialog(bool saveMode) {
    char filename[PATH_MAX];
    FILE *f = popen(("zenity --file-selection"+std::string(saveMode?" --save":"")).c_str(), "r");
    fgets(filename, sizeof(filename), f);
    filename[strlen(filename) - 1] = 0;
    std::string file(filename);
    return file;
}

bool isElevated() {
    return getuid() == 0;
}

void restartElevated() {
    char self[PATH_MAX];
    ssize_t self_len = readlink("/proc/self/exe", self, sizeof(self));
    if (self_len >= 0) {
        self[self_len] = '\0';
        execlp("pkexec", "pkexec", self, nullptr);
        execlp("gksudo", "gksudo", self, nullptr);
        execlp("x-terminal-emulator", "terminal", "-e", "sudo", "setsid", self, nullptr);
    }
}

void openLink(const char *url) {
    if (fork() == 0)
        execlp("xdg-open", "browser launcher", url, nullptr);
}
}
#else
namespace Emscripten {
EM_ASYNC_JS(bool, loadFile, (char *buf, size_t len), {
    onFileUploadFinish = function (data) {
        console.log("Copying profile to memory");
        for (var i = 0; i < Math.min(data.byteLength, len); i++) {
            HEAPU8[buf+i] = data[i];
        }
        console.log("Import complete");
    };
    initUploader();
    triggerUploader();
});
EM_ASYNC_JS(bool, storeFile, (const char *buf, size_t len), {
    console.log("Copying profile from memory");
    var jsdata = new Uint8Array(len);
    for (var i = 0; i < len; i++) {
        jsdata[i] = HEAPU8[buf+i];
    }
    console.log("Export complete");

    download(jsdata, "profile.egg", "application/octet-stream");
});

EM_ASYNC_JS(void, openLink, (const char *url), {
    var string = "";
    for (let i = 0; ; i++) {
        const char = HEAPU8[url+i];
        if (char === 0)
            break;
        string += String.fromCharCode(char);
    }
    window.open(string, '_blank').focus();
});
}
#endif
}

bool loadFile(char *buf, size_t len) {
#ifndef __EMSCRIPTEN__
    const auto path = Native::fileDialog(false);
    if (path.empty())
        return false;

    std::ifstream file(path, std::ios::binary);
    file.read(buf, len);
    return true;
#else
    return Emscripten::loadFile(buf, len);
#endif
}

bool storeFile(const char *buf, size_t len) {
#ifndef __EMSCRIPTEN__
    const auto path = Native::fileDialog(true);
    if (path.empty())
        return false;

    std::ofstream file(path, std::ios::binary);
    file.write(buf, len);
    return true;
#else
    return Emscripten::storeFile(buf, len);
#endif
}

void restartElevated() {
#ifndef __EMSCRIPTEN__
    return Native::restartElevated();
#endif
}

bool isElevated() {
#ifndef __EMSCRIPTEN__
    return Native::isElevated();
#else
    return true;
#endif
}

void openLink(const char *url) {
#ifndef __EMSCRIPTEN__
    Native::openLink(url);
#else
    Emscripten::openLink(url);
#endif
}
}
