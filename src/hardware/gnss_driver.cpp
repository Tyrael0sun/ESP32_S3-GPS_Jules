#include "gnss_driver.h"

#include <Arduino.h>

#include "config.h"
#include "pin_config.h"

namespace {
const uint8_t kUbxRateTpl[] = {
    0xB5, 0x62, 0x06, 0x08, 0x06, 0x00,
    0xF4, 0x01,
    0x01, 0x00,
    0x00, 0x00,
    0x00, 0x00};

uint16_t ubxChecksum(const uint8_t *data, size_t len) {
  uint8_t ckA = 0;
  uint8_t ckB = 0;
  for (size_t i = 0; i < len; ++i) {
    ckA += data[i];
    ckB += ckA;
  }
  return static_cast<uint16_t>(ckB << 8 | ckA);
}

hardware::GnssDriver g_gnss;
}  // namespace

namespace hardware {

void GnssDriver::begin() {
  pinMode(pins::kGnssLdoEn, OUTPUT);
  digitalWrite(pins::kGnssLdoEn, HIGH);
  delay(50);

  serial_.begin(115200, SERIAL_8N1, pins::kGnssRx, pins::kGnssTx);
  parser_ = TinyGPSPlus();
  currentRateHz_ = 0;
  setUpdateRateHz(config::kDefaultGnssRateHz);
  lastUpdateMs_ = millis();
}

void GnssDriver::poll(app::GnssSnapshot &snapshot) {
  while (serial_.available()) {
    const char c = static_cast<char>(serial_.read());
    parser_.encode(c);
  }

  if (parser_.location.isUpdated()) {
    snapshot.latitude = parser_.location.lat();
    snapshot.longitude = parser_.location.lng();
  }
  if (parser_.satellites.isUpdated()) {
    snapshot.satellites = static_cast<uint8_t>(parser_.satellites.value());
  }
  if (parser_.time.isUpdated()) {
    snapshot.fix = parser_.location.isValid() && parser_.hdop.isValid();
  }
  if (parser_.speed.isUpdated()) {
    snapshot.speedKmh = parser_.speed.kmph();
  }
  requestFixData();
}

void GnssDriver::setUpdateRateHz(uint8_t hz) {
  if (hz == currentRateHz_ || hz == 0) {
    return;
  }
  uint16_t rateMs = static_cast<uint16_t>(1000 / hz);
  uint8_t payload[] = {0x06, 0x00, static_cast<uint8_t>(rateMs & 0xFF), static_cast<uint8_t>(rateMs >> 8), 0x01, 0x00};
  sendUbloxCfg(payload, sizeof(payload));
  currentRateHz_ = hz;
}

void GnssDriver::sendUbloxCfg(const uint8_t *data, size_t len) {
  HardwareSerial &ser = serial_;
  ser.write(0xB5);
  ser.write(0x62);
  ser.write(0x06);
  ser.write(0x08);
  ser.write(static_cast<uint8_t>(len));
  ser.write(0x00);
  ser.write(data, len);
  uint16_t chk = ubxChecksum(reinterpret_cast<const uint8_t *>(data), len);
  ser.write(static_cast<uint8_t>(chk & 0xFF));
  ser.write(static_cast<uint8_t>(chk >> 8));
}

void GnssDriver::requestFixData() {
  const uint32_t now = millis();
  if (now - lastUpdateMs_ < 1000) {
    return;
  }
  serial_.println(F("$PCAS03,2*1B"));
  lastUpdateMs_ = now;
}

GnssDriver &gnss() {
  return g_gnss;
}

}  // namespace hardware
