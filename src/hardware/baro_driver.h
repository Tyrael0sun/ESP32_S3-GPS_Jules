#pragma once

#include <Wire.h>

#include "app/app_state.h"

namespace hardware {

class Bmp388Driver {
 public:
  void begin(TwoWire &wire);
  bool available() const { return present_; }
  void read(app::BaroSnapshot &snapshot);

 private:
  bool readCalib();
  bool readRegisters(uint8_t reg, uint8_t *buffer, size_t len);
  bool writeRegister(uint8_t reg, uint8_t value);
  float compensateTemperature(int32_t raw);
  float compensatePressure(int32_t raw);

  TwoWire *wire_ = nullptr;
  bool present_ = false;
  float tLin_ = 0.0f;
  struct {
    uint16_t parT1 = 0;
    uint16_t parT2 = 0;
    int8_t parT3 = 0;
    int16_t parP1 = 0;
    int16_t parP2 = 0;
    int8_t parP3 = 0;
    int8_t parP4 = 0;
    int16_t parP5 = 0;
    int16_t parP6 = 0;
    int8_t parP7 = 0;
    int8_t parP8 = 0;
    int16_t parP9 = 0;
    int8_t parP10 = 0;
    int8_t parP11 = 0;
  } calib_;
};

Bmp388Driver &barometer();

}  // namespace hardware
