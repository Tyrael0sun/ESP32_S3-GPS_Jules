#pragma once

#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

#include "app/app_state.h"

namespace hardware {

class GnssDriver {
 public:
  void begin();
  void poll(app::GnssSnapshot &snapshot);
  void setUpdateRateHz(uint8_t hz);

 private:
  void sendUbloxCfg(const uint8_t *data, size_t len);
  void requestFixData();

  TinyGPSPlus parser_;
  HardwareSerial serial_{1};
  uint32_t lastUpdateMs_ = 0;
  uint8_t currentRateHz_ = 0;
};

GnssDriver &gnss();

}  // namespace hardware
