#include "storage_driver.h"

#include <Arduino.h>
#include <SD_MMC.h>

#include "pin_config.h"

namespace {
bool g_storageReady = false;
}  // namespace

namespace hardware {

bool initStorage() {
  if (g_storageReady) {
    return true;
  }
  SD_MMC.setPins(pins::kSdClk, pins::kSdCmd, pins::kSdD0, pins::kSdD1, pins::kSdD2, pins::kSdD3);
  if (!SD_MMC.begin("/sdcard", false)) {
    return false;
  }
  g_storageReady = true;
  return true;
}

bool ensureGpxDir() {
  if (!g_storageReady && !initStorage()) {
    return false;
  }
  if (!SD_MMC.exists("/GPX")) {
    return SD_MMC.mkdir("/GPX");
  }
  return true;
}

File openGpxLog(const char *filename) {
  if (!ensureGpxDir()) {
    return File();
  }
  char path[64];
  snprintf(path, sizeof(path), "/GPX/%s", filename);
  return SD_MMC.open(path, FILE_APPEND);
}

bool storageReady() {
  return g_storageReady;
}

}  // namespace hardware
