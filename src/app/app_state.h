#pragma once

#include <Arduino.h>
#include <array>

namespace app {

enum class Mode {
  BikeComputer,
  PBox,
  Logger,
  GnssDebug,
  Settings
};

enum class GnssConstellation : uint8_t {
  Unknown,
  GPS,
  GLONASS,
  Galileo,
  BeiDou,
  QZSS,
  SBAS,
  Mixed
};

struct GnssSatellite {
  GnssConstellation constellation = GnssConstellation::Unknown;
  uint8_t nmeaId = 0;
  uint8_t cn0 = 0;
  bool usedForFix = false;
  bool tracked = false;
};

struct GnssSnapshot {
  static constexpr size_t kLogLines = 14;
  static constexpr size_t kLogLineLength = 96;
  static constexpr size_t kMaxSatellites = 32;

  bool fix = false;
  uint8_t satellites = 0;
  double latitude = 0.0;
  double longitude = 0.0;
  double altitudeM = 0.0;
  bool altitudeValid = false;
  float speedKmh = 0.0f;
  size_t satelliteCount = 0;
  std::array<GnssSatellite, kMaxSatellites> satelliteList{};
  bool scrollMode = false;
  int16_t scrollRow = 0;
  size_t logCount = 0;
  char logs[kLogLines][kLogLineLength] = {};
};

struct ImuSnapshot {
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
  float temperatureC = 0.0f;
};

struct MagSnapshot {
  float magX = 0.0f;
  float magY = 0.0f;
  float magZ = 0.0f;
  float temperatureC = 0.0f;
};

struct BaroSnapshot {
  float pressureHpa = 1013.25f;
  float altitudeM = 0.0f;
  float temperatureC = 0.0f;
};

struct BatterySnapshot {
  float voltage = 3.7f;
  float levelPercent = 50.0f;
  bool charging = false;
};

struct SystemState {
  Mode activeMode = Mode::BikeComputer;
  GnssSnapshot gnss;
  ImuSnapshot imu;
  MagSnapshot mag;
  BaroSnapshot baro;
  BatterySnapshot battery;
  bool gpxRecording = false;
};

}  // namespace app
