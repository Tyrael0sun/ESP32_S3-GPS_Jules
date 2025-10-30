#pragma once

#include <Arduino.h>

namespace app {

enum class Mode {
  BikeComputer,
  PBox,
  Logger,
  Settings
};

struct GnssSnapshot {
  bool fix = false;
  uint8_t satellites = 0;
  double latitude = 0.0;
  double longitude = 0.0;
  float speedKmh = 0.0f;
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
