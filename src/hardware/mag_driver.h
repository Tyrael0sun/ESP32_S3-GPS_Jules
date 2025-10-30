#pragma once

#include <Wire.h>

#include "app/app_state.h"

namespace hardware {

class Lis2mdlDriver {
 public:
  void begin(TwoWire &wire);
  bool available() const { return present_; }
  void read(app::MagSnapshot &snapshot);

 private:
  bool writeReg(uint8_t reg, uint8_t value);
  bool readRegs(uint8_t reg, uint8_t *buffer, size_t len);

  TwoWire *wire_ = nullptr;
  bool present_ = false;
};

Lis2mdlDriver &magnetometer();

}  // namespace hardware
