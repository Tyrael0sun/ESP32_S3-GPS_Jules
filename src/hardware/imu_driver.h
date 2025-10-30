#pragma once

#include <Wire.h>

#include "app/app_state.h"

namespace hardware {

class Lsm6dsrDriver {
 public:
  void begin(TwoWire &wire);
  bool available() const { return present_; }
  void read(app::ImuSnapshot &snapshot);

 private:
  bool writeReg(uint8_t reg, uint8_t value);
  bool readRegs(uint8_t reg, uint8_t *buffer, size_t len);

  TwoWire *wire_ = nullptr;
  bool present_ = false;
};

Lsm6dsrDriver &imu();

}  // namespace hardware
