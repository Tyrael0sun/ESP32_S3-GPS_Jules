#include "hardware_stubs.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "config.h"
#include "hardware/baro_driver.h"
#include "hardware/display_driver.h"
#include "hardware/gnss_driver.h"
#include "hardware/imu_driver.h"
#include "hardware/input_driver.h"
#include "hardware/mag_driver.h"
#include "hardware/power_driver.h"
#include "hardware/storage_driver.h"
#include "pin_config.h"
#include "util/serial_router.h"

namespace {
unsigned long lastDiagMs = 0;
bool highRatePhaseComplete = false;
TwoWire I2CBus = TwoWire(0);

float readMcuTemperatureC() {
#ifdef ARDUINO_ARCH_ESP32
  return temperatureRead();
#else
  return NAN;
#endif
}
}  // namespace

namespace hardware {

void initPeripherals() {
  serial_router::init();
  auto &dbg = serial_router::debug();
  dbg.println(F("[BOOT] Initializing peripherals"));

  initDisplay();
  dbg.println(F("[BOOT] Display ready"));

  I2CBus.begin(pins::kI2cSda, pins::kI2cScl, 400000);
  imu().begin(I2CBus);
  magnetometer().begin(I2CBus);
  barometer().begin(I2CBus);
  dbg.printf("[BOOT] IMU: %s\r\n", imu().available() ? "OK" : "MISSING");
  dbg.printf("[BOOT] MAG: %s\r\n", magnetometer().available() ? "OK" : "MISSING");
  dbg.printf("[BOOT] BARO: %s\r\n", barometer().available() ? "OK" : "MISSING");

  gnss().begin();
  initStorage();
  initPowerMonitoring();
  input().begin();

  dbg.println(F("[BOOT] Peripherals initialized"));
}

void pollGnss(app::GnssSnapshot &snapshot) {
  gnss().poll(snapshot);
}

void pollImu(app::ImuSnapshot &snapshot) {
  if (!imu().available()) {
    snapshot.accelX = snapshot.accelY = snapshot.accelZ = NAN;
    snapshot.gyroX = snapshot.gyroY = snapshot.gyroZ = NAN;
    snapshot.temperatureC = NAN;
    return;
  }
  imu().read(snapshot);
}

void pollMagnetometer(app::MagSnapshot &snapshot) {
  if (!magnetometer().available()) {
    snapshot.magX = snapshot.magY = snapshot.magZ = NAN;
    snapshot.temperatureC = NAN;
    return;
  }
  magnetometer().read(snapshot);
}

void pollBarometer(app::BaroSnapshot &snapshot) {
  if (!barometer().available()) {
    snapshot.pressureHpa = NAN;
    snapshot.altitudeM = NAN;
    snapshot.temperatureC = NAN;
    return;
  }
  barometer().read(snapshot);
}

void pollBattery(app::BatterySnapshot &snapshot) {
  readPower(snapshot);
}

void pollInput(app::ApplicationController &controller) {
  input().poll(controller);
}

void flushDiagnostics(const app::SystemState &state, unsigned long nowMs) {
  const unsigned long elapsed = nowMs;
  const bool highRate = !highRatePhaseComplete && elapsed <= config::kDiagHighRateDurationMs;

  if (highRate && (nowMs - lastDiagMs) < config::kDiagHighRateMs) {
    return;
  }

  if (!highRate && (nowMs - lastDiagMs) < 5000) {
    return;
  }

  lastDiagMs = nowMs;
  if (highRate && elapsed > config::kDiagHighRateDurationMs) {
    highRatePhaseComplete = true;
  }

  auto &dbg = serial_router::debug();
  const bool imuOk = !isnan(state.imu.accelX);
  const bool magOk = !isnan(state.mag.magX);
  const bool baroOk = !isnan(state.baro.pressureHpa);
  const float mcuTemp = readMcuTemperatureC();
  dbg.println(F("[DIAG] Snapshot"));
  auto printOptional = [&](float value) {
    if (isnan(value) || isinf(value)) {
      dbg.print(F("N/A"));
    } else {
      dbg.printf("%.2f", value);
    }
  };

  dbg.printf("GNSS: %s, %u sats, (%.6f, %.6f), speed %.2f km/h\r\n",
             state.gnss.fix ? "FIX" : "NO-FIX",
             state.gnss.satellites,
             state.gnss.latitude,
             state.gnss.longitude,
             state.gnss.speedKmh);
  if (imuOk) {
    dbg.printf("IMU: ACC(%.2f, %.2f, %.2f) GYRO(%.2f, %.2f, %.2f)\r\n",
               state.imu.accelX,
               state.imu.accelY,
               state.imu.accelZ,
               state.imu.gyroX,
               state.imu.gyroY,
               state.imu.gyroZ);
    dbg.printf("IMU_ACC_G: X=%.3f Y=%.3f Z=%.3f\r\n",
               state.imu.accelX,
               state.imu.accelY,
               state.imu.accelZ);
  } else {
    dbg.println(F("IMU: MISSING"));
  }
  if (magOk) {
    dbg.printf("MAG: (%.2f, %.2f, %.2f)\r\n",
               state.mag.magX,
               state.mag.magY,
               state.mag.magZ);
  } else {
    dbg.println(F("MAG: MISSING"));
  }
  if (baroOk) {
    dbg.printf("BARO: %.2f hPa, alt %.2f m\r\n",
               state.baro.pressureHpa,
               state.baro.altitudeM);
  } else {
    dbg.println(F("BARO: MISSING"));
  }
  dbg.printf("BAT: %.2f%%, %.2f V, charging=%s\r\n",
             state.battery.levelPercent,
             state.battery.voltage,
             state.battery.charging ? "yes" : "no");
  dbg.print(F("TEMP: MCU="));
  printOptional(mcuTemp);
  dbg.print(F("°C, IMU="));
  printOptional(state.imu.temperatureC);
  dbg.print(F("°C, BARO="));
  printOptional(state.baro.temperatureC);
  dbg.print(F("°C, MAG="));
  printOptional(state.mag.temperatureC);
  dbg.println(F("°C"));
}

}  // namespace hardware
