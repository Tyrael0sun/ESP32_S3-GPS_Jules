#include "mag_driver.h"

#include <Arduino.h>

namespace {
constexpr uint8_t kAddr = 0x1E;
constexpr uint8_t kWhoAmI = 0x4F;
constexpr uint8_t kCfgRegA = 0x60;
constexpr uint8_t kCfgRegB = 0x61;
constexpr uint8_t kCfgRegC = 0x62;
constexpr uint8_t kStatus = 0x67;
constexpr uint8_t kOutXL = 0x68;
constexpr float kScale = 1.5f;  // mG/LSB

auto g_mag = hardware::Lis2mdlDriver();
}  // namespace

namespace hardware {

void Lis2mdlDriver::begin(TwoWire &wire) {
  wire_ = &wire;
  wire_->beginTransmission(kAddr);
  if (wire_->endTransmission() != 0) {
    present_ = false;
    return;
  }
  uint8_t who = 0;
  readRegs(kWhoAmI, &who, 1);
  if (who != 0x40) {
    present_ = false;
    return;
  }
  writeReg(kCfgRegA, 0x80);
  writeReg(kCfgRegB, 0x02);
  writeReg(kCfgRegC, 0x10);
  present_ = true;
}

void Lis2mdlDriver::read(app::MagSnapshot &snapshot) {
  if (!present_ || !wire_) {
    return;
  }
  uint8_t status = 0;
  if (!readRegs(kStatus, &status, 1) || (status & 0x08) == 0) {
    return;
  }
  uint8_t buffer[8] = {0};
  if (!readRegs(kOutXL, buffer, sizeof(buffer))) {
    return;
  }
  const int16_t rawX = static_cast<int16_t>(buffer[1] << 8 | buffer[0]);
  const int16_t rawY = static_cast<int16_t>(buffer[3] << 8 | buffer[2]);
  const int16_t rawZ = static_cast<int16_t>(buffer[5] << 8 | buffer[4]);
  const int16_t rawTemp = static_cast<int16_t>(buffer[7] << 8 | buffer[6]);

  const float measuredX = rawX * kScale / 1000.0f;
  const float measuredY = rawY * kScale / 1000.0f;
  const float measuredZ = rawZ * kScale / 1000.0f;

  snapshot.magX = measuredY;
  snapshot.magY = -measuredX;
  snapshot.magZ = -measuredZ;
  snapshot.temperatureC = 25.0f + (static_cast<float>(rawTemp) / 8.0f);
}

bool Lis2mdlDriver::writeReg(uint8_t reg, uint8_t value) {
  if (!wire_) {
    return false;
  }
  wire_->beginTransmission(kAddr);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

bool Lis2mdlDriver::readRegs(uint8_t reg, uint8_t *buffer, size_t len) {
  if (!wire_) {
    return false;
  }
  wire_->beginTransmission(kAddr);
  wire_->write(reg);
  if (wire_->endTransmission(false) != 0) {
    return false;
  }
  const size_t readLen = wire_->requestFrom(kAddr, static_cast<uint8_t>(len));
  if (readLen != len) {
    return false;
  }
  for (size_t i = 0; i < len; ++i) {
    buffer[i] = wire_->read();
  }
  return true;
}

Lis2mdlDriver &magnetometer() {
  return g_mag;
}

}  // namespace hardware
