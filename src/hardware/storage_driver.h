#pragma once

#include <FS.h>

namespace hardware {

bool initStorage();
bool ensureGpxDir();
File openGpxLog(const char *filename);

}  // namespace hardware
