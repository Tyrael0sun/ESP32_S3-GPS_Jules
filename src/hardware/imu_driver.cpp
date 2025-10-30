#include "imu_driver.h"

#include <Arduino.h>

namespace {
constexpr uint8_t kAddr = 0x6A;
constexpr uint8_t kWhoAmI = 0x0F;
constexpr uint8_t kCtrl1Xl = 0x10;
constexpr uint8_t kCtrl2G = 0x11;
constexpr uint8_t kCtrl3C = 0x12;
constexpr uint8_t kOutTempL = 0x20;
constexpr float kAccelScale = 0.061f;  // mg/LSB @ +/-2g
constexpr float kGyroScale = 4.375f;    // mdps/LSB @ 125dps

auto g_driver = hardware::Lsm6dsrDriver();
}  // namespace

namespace hardware {

void Lsm6dsrDriver::begin(TwoWire &wire) {
  wire_ = &wire;
  wire_->beginTransmission(kAddr);
  if (wire_->endTransmission() != 0) {
    present_ = false;
    return;
  }
  uint8_t who = 0;
  readRegs(kWhoAmI, &who, 1);
  if (who != 0x6A && who != 0x6B) {
    present_ = false;
    return;
  }
  writeReg(kCtrl3C, 0x04);
  delay(10);
  writeReg(kCtrl1Xl, 0x60);
  writeReg(kCtrl2G, 0x60);
  present_ = true;
}

void Lsm6dsrDriver::read(app::ImuSnapshot &snapshot) {
  if (!present_ || !wire_) {
    return;
  }
  uint8_t buffer[14] = {0};
  if (!readRegs(kOutTempL, buffer, sizeof(buffer))) {
    return;
  }
  const int16_t tempRaw = static_cast<int16_t>(buffer[1] << 8 | buffer[0]);
  const int16_t gyroX = static_cast<int16_t>(buffer[3] << 8 | buffer[2]);
  const int16_t gyroY = static_cast<int16_t>(buffer[5] << 8 | buffer[4]);
  const int16_t gyroZ = static_cast<int16_t>(buffer[7] << 8 | buffer[6]);
  const int16_t accelX = static_cast<int16_t>(buffer[9] << 8 | buffer[8]);
  const int16_t accelY = static_cast<int16_t>(buffer[11] << 8 | buffer[10]);
  const int16_t accelZ = static_cast<int16_t>(buffer[13] << 8 | buffer[12]);

  snapshot.temperatureC = 25.0f + (tempRaw / 16.0f);
  snapshot.accelX = (accelX * kAccelScale) / 1000.0f;
  snapshot.accelY = (accelY * kAccelScale) / 1000.0f;
  snapshot.accelZ = (accelZ * kAccelScale) / 1000.0f;
  snapshot.gyroX = (gyroX * kGyroScale) / 1000.0f;
  snapshot.gyroY = (gyroY * kGyroScale) / 1000.0f;
  snapshot.gyroZ = (gyroZ * kGyroScale) / 1000.0f;
}

bool Lsm6dsrDriver::writeReg(uint8_t reg, uint8_t value) {
  if (!wire_) {
    return false;
  }
  wire_->beginTransmission(kAddr);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

bool Lsm6dsrDriver::readRegs(uint8_t reg, uint8_t *buffer, size_t len) {
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

Lsm6dsrDriver &imu() {
  return g_driver;
}

}  // namespace hardware
