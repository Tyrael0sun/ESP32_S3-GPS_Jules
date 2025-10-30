#include "baro_driver.h"

#include <Arduino.h>
#include <math.h>

namespace {
constexpr uint8_t kAddr = 0x77;
constexpr uint8_t kChipIdReg = 0x00;
constexpr uint8_t kChipId = 0x50;
constexpr uint8_t kCmdSoftReset = 0xB6;
constexpr uint8_t kCmdReg = 0x7E;
constexpr uint8_t kPwrCtrl = 0x1B;
constexpr uint8_t kOsr = 0x1C;
constexpr uint8_t kOdr = 0x1D;
constexpr uint8_t kConfig = 0x1F;
constexpr uint8_t kDataReg = 0x04;
constexpr uint8_t kCalibReg = 0x31;

auto g_bmp = hardware::Bmp388Driver();
}  // namespace

namespace hardware {

void Bmp388Driver::begin(TwoWire &wire) {
  wire_ = &wire;
  wire_->beginTransmission(kAddr);
  if (wire_->endTransmission() != 0) {
    present_ = false;
    return;
  }
  uint8_t chipId = 0;
  readRegisters(kChipIdReg, &chipId, 1);
  if (chipId != kChipId) {
    present_ = false;
    return;
  }
  writeRegister(kCmdReg, kCmdSoftReset);
  delay(10);
  if (!readCalib()) {
    present_ = false;
    return;
  }
  writeRegister(kPwrCtrl, 0x33);
  writeRegister(kOsr, 0x15);
  writeRegister(kOdr, 0x00);
  writeRegister(kConfig, 0x02);
  present_ = true;
}

void Bmp388Driver::read(app::BaroSnapshot &snapshot) {
  if (!present_ || !wire_) {
    return;
  }
  uint8_t buffer[6] = {0};
  if (!readRegisters(kDataReg, buffer, sizeof(buffer))) {
    return;
  }
  const int32_t rawPressure = static_cast<int32_t>((static_cast<uint32_t>(buffer[2]) << 16) |
                                                   (static_cast<uint32_t>(buffer[1]) << 8) |
                                                   buffer[0]);
  const int32_t rawTemp = static_cast<int32_t>((static_cast<uint32_t>(buffer[5]) << 16) |
                                               (static_cast<uint32_t>(buffer[4]) << 8) |
                                               buffer[3]);
  const float temperature = compensateTemperature(rawTemp);
  const float pressure = compensatePressure(rawPressure);

  snapshot.temperatureC = temperature;
  snapshot.pressureHpa = pressure / 100.0f;
  snapshot.altitudeM = 44330.0f * (1.0f - pow(snapshot.pressureHpa / 1013.25f, 0.1903f));
}

bool Bmp388Driver::readCalib() {
  uint8_t buffer[21] = {0};
  if (!readRegisters(kCalibReg, buffer, sizeof(buffer))) {
    return false;
  }
  calib_.parT1 = static_cast<uint16_t>(buffer[1] << 8 | buffer[0]);
  calib_.parT2 = static_cast<uint16_t>(buffer[3] << 8 | buffer[2]);
  calib_.parT3 = static_cast<int8_t>(buffer[4]);
  calib_.parP1 = static_cast<int16_t>(buffer[6] << 8 | buffer[5]);
  calib_.parP2 = static_cast<int16_t>(buffer[8] << 8 | buffer[7]);
  calib_.parP3 = static_cast<int8_t>(buffer[9]);
  calib_.parP4 = static_cast<int8_t>(buffer[10]);
  calib_.parP5 = static_cast<int16_t>(buffer[12] << 8 | buffer[11]);
  calib_.parP6 = static_cast<int16_t>(buffer[14] << 8 | buffer[13]);
  calib_.parP7 = static_cast<int8_t>(buffer[15]);
  calib_.parP8 = static_cast<int8_t>(buffer[16]);
  calib_.parP9 = static_cast<int16_t>(buffer[18] << 8 | buffer[17]);
  calib_.parP10 = static_cast<int8_t>(buffer[19]);
  calib_.parP11 = static_cast<int8_t>(buffer[20]);
  return true;
}

float Bmp388Driver::compensateTemperature(int32_t raw) {
  const float partialData1 = static_cast<float>(raw - (static_cast<int32_t>(calib_.parT1) << 8));
  const float partialData2 = partialData1 * static_cast<float>(calib_.parT2) / pow(2.0f, 30);
  tLin_ = partialData2 + (partialData1 * partialData1) * static_cast<float>(calib_.parT3) / pow(2.0f, 48);
  return tLin_;
}

float Bmp388Driver::compensatePressure(int32_t raw) {
  const float partialData1 = tLin_ * tLin_;
  const float partialData2 = partialData1 * tLin_;
  const float partialData3 = static_cast<float>(calib_.parP6) * partialData1 / pow(2.0f, 48);
  const float partialData4 = static_cast<float>(calib_.parP7) * partialData2 / pow(2.0f, 65);
  const float partialData5 = static_cast<float>(calib_.parP8) * partialData1 / pow(2.0f, 48);
  float offset = static_cast<float>(calib_.parP5) * pow(2.0f, 3) + partialData3 + partialData4 + partialData5;
  offset += static_cast<float>(calib_.parP4) * tLin_ / pow(2.0f, 20);

  float sensitivity = static_cast<float>(calib_.parP1) * pow(2.0f, 20) +
                      static_cast<float>(calib_.parP2) * tLin_ / pow(2.0f, 12) +
                      static_cast<float>(calib_.parP3) * partialData1 / pow(2.0f, 17) +
                      static_cast<float>(calib_.parP9) * partialData2 / pow(2.0f, 69);

  const float partialData6 = static_cast<float>(raw) - offset;
  const float pressure = partialData6 * pow(2.0f, 4) / sensitivity;
  return pressure;
}

bool Bmp388Driver::readRegisters(uint8_t reg, uint8_t *buffer, size_t len) {
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

bool Bmp388Driver::writeRegister(uint8_t reg, uint8_t value) {
  if (!wire_) {
    return false;
  }
  wire_->beginTransmission(kAddr);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

Bmp388Driver &barometer() {
  return g_bmp;
}

}  // namespace hardware
