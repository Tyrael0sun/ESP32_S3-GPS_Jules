#pragma once

#include <array>

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
  void startSerial(uint32_t baud);
  bool drainSerial(uint32_t durationMs, bool detectSentence = false);
  void recoverSerial();
  bool configureSerialPort(uint32_t baud);
  bool sendUbloxCommand(uint8_t cls, uint8_t id, const uint8_t *payload, size_t len);
  bool waitForAck(uint8_t cls, uint8_t id);
  void appendLogChar(char c);
  void finalizeLogLine();
  void copyLogsToSnapshot(app::GnssSnapshot &snapshot) const;
  void parseNmeaSentence(const char *line);
  void handleGsvSentence(const char *systemId, char *&ctx);
  void handleGsaSentence(const char *systemId, char *&ctx);
  void updateSatellite(app::GnssConstellation constellation, uint8_t nmeaId, uint8_t cn0, uint32_t nowMs);
  void clearSatelliteUsage(app::GnssConstellation target, bool global);
  void markSatelliteUsed(uint8_t nmeaId, app::GnssConstellation preferConstellation, bool global, uint32_t nowMs);
  void pruneSatellites(uint32_t nowMs);
  void copySatellitesToSnapshot(app::GnssSnapshot &snapshot) const;

  TinyGPSPlus parser_;
  HardwareSerial serial_{1};
  uint32_t currentBaud_ = 0;
  uint8_t currentRateHz_ = 0;
  std::array<std::array<char, app::GnssSnapshot::kLogLineLength>, app::GnssSnapshot::kLogLines> logLines_{};
  std::array<char, app::GnssSnapshot::kLogLineLength> currentLine_{};
  size_t logWriteIndex_ = 0;
  size_t logCount_ = 0;
  size_t currentLinePos_ = 0;
  uint32_t lastSerialActivityMs_ = 0;
  uint32_t lastParserCharCount_ = 0;
  struct SatelliteRecord {
    app::GnssConstellation constellation = app::GnssConstellation::Unknown;
    uint8_t nmeaId = 0;
    uint8_t cn0 = 0;
    bool usedForFix = false;
    bool tracked = false;
    uint32_t lastSeenMs = 0;
  };
  std::array<SatelliteRecord, app::GnssSnapshot::kMaxSatellites> satellites_{};
  size_t satelliteCount_ = 0;
};

GnssDriver &gnss();

}  // namespace hardware
