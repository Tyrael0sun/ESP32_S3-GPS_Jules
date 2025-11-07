#include "gnss_driver.h"

#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include "config.h"
#include "pin_config.h"

namespace {
hardware::GnssDriver g_gnss;
constexpr uint32_t kSatelliteStaleTimeoutMs = 15000;

app::GnssConstellation constellationFromTalker(const char *systemId) {
  if (!systemId || std::strlen(systemId) < 2) {
    return app::GnssConstellation::Unknown;
  }
  if (std::strncmp(systemId, "GP", 2) == 0) {
    return app::GnssConstellation::GPS;
  }
  if (std::strncmp(systemId, "GL", 2) == 0) {
    return app::GnssConstellation::GLONASS;
  }
  if (std::strncmp(systemId, "GA", 2) == 0) {
    return app::GnssConstellation::Galileo;
  }
  if (std::strncmp(systemId, "GB", 2) == 0 || std::strncmp(systemId, "BD", 2) == 0) {
    return app::GnssConstellation::BeiDou;
  }
  if (std::strncmp(systemId, "GQ", 2) == 0) {
    return app::GnssConstellation::QZSS;
  }
  if (std::strncmp(systemId, "GS", 2) == 0) {
    return app::GnssConstellation::SBAS;
  }
  if (std::strncmp(systemId, "GN", 2) == 0) {
    return app::GnssConstellation::Mixed;
  }
  return app::GnssConstellation::Unknown;
}
}  // namespace

namespace hardware {

void GnssDriver::begin() {
  pinMode(pins::kGnssLdoEn, OUTPUT);
  digitalWrite(pins::kGnssLdoEn, HIGH);
  delay(50);

  serial_.begin(config::kGnssStartupBaud, SERIAL_8N1, pins::kGnssRx, pins::kGnssTx);
  delay(200);

  parser_ = TinyGPSPlus();
  currentRateHz_ = 0;
  logWriteIndex_ = 0;
  logCount_ = 0;
  currentLinePos_ = 0;
  for (auto &line : logLines_) {
    line.fill('\0');
  }
  currentLine_.fill('\0');
  satellites_.fill({});
  satelliteCount_ = 0;

  // Apply configuration while the receiver is still at its default baud rate.
  setUpdateRateHz(config::kDefaultGnssRateHz);
  configureNmeaOutput();
  configureDynamicModel();

  if (config::kGnssTargetBaud != config::kGnssStartupBaud) {
    const bool switched = configureSerialPort(config::kGnssTargetBaud);
    serial_.flush();
    delay(100);
    if (switched) {
      serial_.end();
      delay(10);
      serial_.begin(config::kGnssTargetBaud, SERIAL_8N1, pins::kGnssRx, pins::kGnssTx);
    }
  }
}

void GnssDriver::poll(app::GnssSnapshot &snapshot) {
  while (serial_.available()) {
    const char c = static_cast<char>(serial_.read());
    appendLogChar(c);
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
  if (parser_.altitude.isUpdated()) {
    snapshot.altitudeM = parser_.altitude.meters();
    snapshot.altitudeValid = parser_.altitude.isValid();
  }

  copyLogsToSnapshot(snapshot);
  copySatellitesToSnapshot(snapshot);
}

void GnssDriver::setUpdateRateHz(uint8_t hz) {
  if (hz == currentRateHz_ || hz == 0) {
    return;
  }
  const uint16_t rateMs = static_cast<uint16_t>(1000u / hz);
  const uint8_t payload[] = {
      static_cast<uint8_t>(rateMs & 0xFF),
      static_cast<uint8_t>((rateMs >> 8) & 0xFF),
      0x01, 0x00,
      0x01, 0x00};
  if (sendUbloxCommand(0x06, 0x08, payload, sizeof(payload))) {
    currentRateHz_ = hz;
  }
}

bool GnssDriver::configureSerialPort(uint32_t baud) {
  const uint8_t payload[20] = {
      0x01, 0x00,
      0x00, 0x00,
      0xD0, 0x08, 0x00, 0x00,
      static_cast<uint8_t>(baud & 0xFF),
      static_cast<uint8_t>((baud >> 8) & 0xFF),
      static_cast<uint8_t>((baud >> 16) & 0xFF),
      static_cast<uint8_t>((baud >> 24) & 0xFF),
      0x07, 0x00,
      0x03, 0x00,
      0x00, 0x00,
      0x00, 0x00};
  return sendUbloxCommand(0x06, 0x00, payload, sizeof(payload));
}

bool GnssDriver::sendUbloxCommand(uint8_t cls, uint8_t id, const uint8_t *payload, size_t len) {
  HardwareSerial &ser = serial_;
  constexpr int kMaxAttempts = 3;
  const uint8_t lenLsb = static_cast<uint8_t>(len & 0xFF);
  const uint8_t lenMsb = static_cast<uint8_t>((len >> 8) & 0xFF);

  auto computeChecksum = [&](uint8_t &ckA, uint8_t &ckB) {
    ckA = 0;
    ckB = 0;
    auto addByte = [&](uint8_t byte) {
      ckA = static_cast<uint8_t>(ckA + byte);
      ckB = static_cast<uint8_t>(ckB + ckA);
    };

    addByte(cls);
    addByte(id);
    addByte(lenLsb);
    addByte(lenMsb);
    for (size_t i = 0; i < len; ++i) {
      addByte(payload ? payload[i] : 0);
    }
  };

  for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
    while (ser.available()) {
      const char junk = static_cast<char>(ser.read());
      parser_.encode(junk);
    }

    uint8_t ckA = 0;
    uint8_t ckB = 0;
    computeChecksum(ckA, ckB);

    ser.write(0xB5);
    ser.write(0x62);
    ser.write(cls);
    ser.write(id);
    ser.write(lenLsb);
    ser.write(lenMsb);
    if (payload && len > 0) {
      ser.write(payload, len);
    }
    ser.write(ckA);
    ser.write(ckB);
    ser.flush();

    if (waitForAck(cls, id)) {
      return true;
    }
    delay(20);
  }

  return false;
}

void GnssDriver::configureDynamicModel() {
  uint8_t payload[36] = {};
  payload[0] = 0x03;  // apply dynamic model and fix mode
  payload[2] = 0x06;  // automotive dynamic model for bike computer use
  payload[3] = 0x03;  // auto 2D/3D fix mode
  sendUbloxCommand(0x06, 0x24, payload, sizeof(payload));
}

void GnssDriver::configureNmeaOutput() {
  // Force GPS-only constellation tracking (disable all other systems).
  {
    const uint8_t payload[] = {
        0x00, 0x20, 0x20, 0x07,
        0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,  // GPS enabled
        0x01, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,  // SBAS disabled
        0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,  // Galileo disabled
        0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,  // BeiDou disabled
        0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,  // IMES disabled
        0x05, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,  // QZSS disabled
        0x06, 0x0A, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00   // GLONASS disabled
    };
    sendUbloxCommand(0x06, 0x3E, payload, sizeof(payload));
  }

  auto configureMessage = [&](uint8_t msgClass, uint8_t msgId, uint8_t rate) {
    const uint8_t payload[8] = {
        msgClass,
        msgId,
        0x00,  // I2C
        rate,  // UART1 (active port)
        0x00,  // UART2
        0x00,  // USB
        0x00,  // SPI
        0x00}; // Reserved
    sendUbloxCommand(0x06, 0x01, payload, sizeof(payload));
  };

  constexpr uint8_t kDisableStdIds[] = {0x01, 0x05, 0x06, 0x0A, 0x0D, 0x0F, 0x41};
  for (uint8_t msgId : kDisableStdIds) {
    configureMessage(0xF0, msgId, 0);
  }

  configureMessage(0xF0, 0x00, 1);  // GGA for position
  configureMessage(0xF0, 0x04, 1);  // RMC for time/speed
  configureMessage(0xF0, 0x02, 1);  // GSA for satellites used
  configureMessage(0xF0, 0x03, 1);  // GSV for satellites in view

  // Disable non-GPS talker sentences entirely.
  constexpr uint8_t kDisableOtherTalkers[] = {0x00, 0x01, 0x02, 0x03};
  for (uint8_t msgId : kDisableOtherTalkers) {
    configureMessage(0xF1, msgId, 0);
  }
}

void GnssDriver::appendLogChar(char c) {
  const uint8_t uc = static_cast<uint8_t>(c);
  if (uc == '\r') {
    return;
  }
  if (uc == '\n') {
    finalizeLogLine();
    return;
  }
  if (uc < 0x20 || uc > 0x7E) {
    return;
  }
  if (currentLinePos_ >= app::GnssSnapshot::kLogLineLength - 1) {
    return;
  }
  currentLine_[currentLinePos_++] = static_cast<char>(uc);
  currentLine_[currentLinePos_] = '\0';
}

void GnssDriver::finalizeLogLine() {
  if (currentLinePos_ == 0) {
    return;
  }

  currentLine_[currentLinePos_] = '\0';
  parseNmeaSentence(currentLine_.data());
  logLines_[logWriteIndex_] = currentLine_;
  logWriteIndex_ = (logWriteIndex_ + 1) % app::GnssSnapshot::kLogLines;
  if (logCount_ < app::GnssSnapshot::kLogLines) {
    ++logCount_;
  }
  currentLine_.fill('\0');
  currentLinePos_ = 0;
}

void GnssDriver::copyLogsToSnapshot(app::GnssSnapshot &snapshot) const {
  const size_t capacity = app::GnssSnapshot::kLogLines;
  snapshot.logCount = logCount_;
  for (size_t i = 0; i < capacity; ++i) {
    snapshot.logs[i][0] = '\0';
  }
  if (logCount_ == 0) {
    return;
  }

  const size_t start = (logCount_ < capacity)
                           ? (logWriteIndex_ + capacity - logCount_) % capacity
                           : logWriteIndex_;
  for (size_t i = 0; i < logCount_; ++i) {
    const size_t src = (start + i) % capacity;
    std::strncpy(snapshot.logs[i], logLines_[src].data(), app::GnssSnapshot::kLogLineLength - 1);
    snapshot.logs[i][app::GnssSnapshot::kLogLineLength - 1] = '\0';
  }
}

void GnssDriver::parseNmeaSentence(const char *line) {
  if (!line || line[0] != '$') {
    return;
  }

  const char *asterisk = std::strchr(line, '*');
  const size_t rawLen = asterisk ? static_cast<size_t>(asterisk - line - 1) : std::strlen(line + 1);
  const size_t copyLen = std::min(rawLen, static_cast<size_t>(app::GnssSnapshot::kLogLineLength - 1));
  if (copyLen == 0) {
    return;
  }

  char buffer[app::GnssSnapshot::kLogLineLength] = {};
  std::memcpy(buffer, line + 1, copyLen);
  buffer[copyLen] = '\0';

  char *ctx = nullptr;
  char *token = strtok_r(buffer, ",", &ctx);
  if (!token) {
    return;
  }

  if (std::strlen(token) < 5) {
    return;
  }

  char systemCode[3] = {token[0], token[1], '\0'};
  const char *sentenceId = token + 2;

  if (std::strncmp(sentenceId, "GSV", 3) == 0) {
    handleGsvSentence(systemCode, ctx);
  } else if (std::strncmp(sentenceId, "GSA", 3) == 0) {
    handleGsaSentence(systemCode, ctx);
  }
}

void GnssDriver::handleGsvSentence(const char *systemId, char *&ctx) {
  (void)strtok_r(nullptr, ",", &ctx);
  (void)strtok_r(nullptr, ",", &ctx);
  (void)strtok_r(nullptr, ",", &ctx);

  const app::GnssConstellation constellation = constellationFromTalker(systemId);
  const uint32_t now = millis();

  for (int slot = 0; slot < 4; ++slot) {
  char *prnStr = strtok_r(nullptr, ",", &ctx);
  (void)strtok_r(nullptr, ",", &ctx);
  (void)strtok_r(nullptr, ",", &ctx);
  char *cn0Str = strtok_r(nullptr, ",", &ctx);
    if (!prnStr) {
      break;
    }

    const int prnVal = std::atoi(prnStr);
    if (prnVal <= 0) {
      continue;
    }

    const uint8_t cn0 = (cn0Str && *cn0Str) ? static_cast<uint8_t>(std::max(0, std::atoi(cn0Str))) : 0;
    updateSatellite(constellation, static_cast<uint8_t>(prnVal), cn0, now);
  }

  pruneSatellites(now);
}

void GnssDriver::handleGsaSentence(const char *systemId, char *&ctx) {
  (void)strtok_r(nullptr, ",", &ctx);
  (void)strtok_r(nullptr, ",", &ctx);

  const app::GnssConstellation constellation = constellationFromTalker(systemId);
  const bool global = (constellation == app::GnssConstellation::Mixed);
  const uint32_t now = millis();

  clearSatelliteUsage(constellation, global);

  for (int i = 0; i < 12; ++i) {
  char *idStr = strtok_r(nullptr, ",", &ctx);
    if (!idStr) {
      break;
    }
    if (!*idStr) {
      continue;
    }

    const int idVal = std::atoi(idStr);
    if (idVal <= 0) {
      continue;
    }

    markSatelliteUsed(static_cast<uint8_t>(idVal), constellation, global, now);
  }

  pruneSatellites(now);
}

void GnssDriver::updateSatellite(app::GnssConstellation constellation, uint8_t nmeaId, uint8_t cn0, uint32_t nowMs) {
  for (size_t i = 0; i < satelliteCount_; ++i) {
    auto &sat = satellites_[i];
    if (sat.nmeaId == nmeaId && (sat.constellation == constellation || constellation == app::GnssConstellation::Mixed)) {
      sat.constellation = (constellation == app::GnssConstellation::Mixed) ? sat.constellation : constellation;
      sat.cn0 = cn0;
      sat.tracked = cn0 > 0;
      sat.lastSeenMs = nowMs;
      return;
    }
  }

  if (satelliteCount_ >= satellites_.size()) {
    size_t oldestIndex = 0;
    uint32_t oldestTime = satellites_[0].lastSeenMs;
    for (size_t i = 1; i < satelliteCount_; ++i) {
      if (satellites_[i].lastSeenMs < oldestTime) {
        oldestTime = satellites_[i].lastSeenMs;
        oldestIndex = i;
      }
    }
    auto &sat = satellites_[oldestIndex];
    sat.constellation = constellation;
    sat.nmeaId = nmeaId;
    sat.cn0 = cn0;
    sat.usedForFix = false;
    sat.tracked = cn0 > 0;
    sat.lastSeenMs = nowMs;
    return;
  }

  auto &sat = satellites_[satelliteCount_++];
  sat.constellation = constellation;
  sat.nmeaId = nmeaId;
  sat.cn0 = cn0;
  sat.usedForFix = false;
  sat.tracked = cn0 > 0;
  sat.lastSeenMs = nowMs;
}

void GnssDriver::clearSatelliteUsage(app::GnssConstellation target, bool global) {
  for (size_t i = 0; i < satelliteCount_; ++i) {
    auto &sat = satellites_[i];
    if (global || sat.constellation == target || target == app::GnssConstellation::Unknown) {
      sat.usedForFix = false;
    }
  }
}

void GnssDriver::markSatelliteUsed(uint8_t nmeaId, app::GnssConstellation preferConstellation, bool global, uint32_t nowMs) {
  for (size_t i = 0; i < satelliteCount_; ++i) {
    auto &sat = satellites_[i];
    const bool idMatch = sat.nmeaId == nmeaId;
    const bool constellationMatch = global || sat.constellation == preferConstellation || preferConstellation == app::GnssConstellation::Unknown;
    if (idMatch && constellationMatch) {
      sat.usedForFix = true;
      sat.tracked = true;
      sat.lastSeenMs = nowMs;
      return;
    }
  }
}

void GnssDriver::pruneSatellites(uint32_t nowMs) {
  size_t writeIdx = 0;
  for (size_t readIdx = 0; readIdx < satelliteCount_; ++readIdx) {
    const auto &sat = satellites_[readIdx];
    if (nowMs - sat.lastSeenMs > kSatelliteStaleTimeoutMs) {
      continue;
    }
    if (writeIdx != readIdx) {
      satellites_[writeIdx] = satellites_[readIdx];
    }
    ++writeIdx;
  }
  satelliteCount_ = writeIdx;
}

void GnssDriver::copySatellitesToSnapshot(app::GnssSnapshot &snapshot) const {
  std::array<size_t, app::GnssSnapshot::kMaxSatellites> indices{};
  const size_t count = std::min(satelliteCount_, satellites_.size());
  for (size_t i = 0; i < count; ++i) {
    indices[i] = i;
  }

  std::sort(indices.begin(), indices.begin() + count, [&](size_t lhs, size_t rhs) {
    const auto &a = satellites_[lhs];
    const auto &b = satellites_[rhs];
    if (a.cn0 == b.cn0) {
      return a.nmeaId < b.nmeaId;
    }
    return a.cn0 > b.cn0;
  });

  snapshot.satelliteCount = count;
  for (size_t i = 0; i < snapshot.satelliteList.size(); ++i) {
    auto &dst = snapshot.satelliteList[i];
    if (i < count) {
      const auto &src = satellites_[indices[i]];
      dst.constellation = src.constellation;
      dst.nmeaId = src.nmeaId;
      dst.cn0 = src.cn0;
      dst.usedForFix = src.usedForFix;
      dst.tracked = src.tracked;
    } else {
      dst = {};
    }
  }

  if (snapshot.satelliteCount == 0) {
    snapshot.scrollRow = 0;
  } else {
    const int maxRow = static_cast<int>(snapshot.satelliteCount) - 1;
    if (snapshot.scrollRow > maxRow) {
      snapshot.scrollRow = maxRow;
    }
    if (snapshot.scrollRow < 0) {
      snapshot.scrollRow = 0;
    }
  }
}

bool GnssDriver::waitForAck(uint8_t cls, uint8_t id) {
  constexpr uint32_t kAckTimeoutMs = 500;
  const uint32_t start = millis();

  enum class State : uint8_t {
    kSync1,
    kSync2,
    kClass,
    kId,
    kLengthLsb,
    kLengthMsb,
    kPayloadClass,
    kPayloadId,
    kCkA,
    kCkB,
  };

  State state = State::kSync1;
  uint8_t ackType = 0x00;

  while (millis() - start < kAckTimeoutMs) {
    if (!serial_.available()) {
      delay(1);
      continue;
    }

    const uint8_t byte = static_cast<uint8_t>(serial_.read());

    switch (state) {
      case State::kSync1:
        if (byte == 0xB5) {
          state = State::kSync2;
        }
        if (byte >= 0x20 && byte <= 0x7E) {
          parser_.encode(static_cast<char>(byte));
        }
        break;

      case State::kSync2:
        if (byte == 0x62) {
          state = State::kClass;
        } else {
          state = (byte == 0xB5) ? State::kSync2 : State::kSync1;
        }
        break;

      case State::kClass:
        if (byte == 0x05) {
          state = State::kId;
        } else {
          if (byte >= 0x20 && byte <= 0x7E) {
            parser_.encode(static_cast<char>(byte));
          }
          state = State::kSync1;
        }
        break;

      case State::kId:
        if (byte == 0x01 || byte == 0x00) {
          ackType = byte;
          state = State::kLengthLsb;
        } else {
          state = State::kSync1;
        }
        break;

      case State::kLengthLsb:
        if (byte == 0x02) {
          state = State::kLengthMsb;
        } else {
          state = State::kSync1;
        }
        break;

      case State::kLengthMsb:
        if (byte == 0x00) {
          state = State::kPayloadClass;
        } else {
          state = State::kSync1;
        }
        break;

      case State::kPayloadClass:
        if (byte == cls) {
          state = State::kPayloadId;
        } else {
          if (byte >= 0x20 && byte <= 0x7E) {
            parser_.encode(static_cast<char>(byte));
          }
          state = State::kSync1;
        }
        break;

      case State::kPayloadId:
        if (byte == id) {
          state = State::kCkA;
        } else {
          if (byte >= 0x20 && byte <= 0x7E) {
            parser_.encode(static_cast<char>(byte));
          }
          state = State::kSync1;
        }
        break;

      case State::kCkA:
        state = State::kCkB;
        break;

      case State::kCkB:
        return ackType == 0x01;
    }
  }

  return false;
}

GnssDriver &gnss() {
  return g_gnss;
}

}  // namespace hardware
