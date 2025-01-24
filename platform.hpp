#pragma once

#include <cstddef>


namespace Platform {
bool loadFile(char *buf, size_t len);
bool storeFile(const char *buf, size_t len);
void restartElevated();
bool isElevated();
}
