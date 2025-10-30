#include "power_driver.h"

#include <Arduino.h>

#include "pin_config.h"

namespace hardware {

void initPowerMonitoring() {
  analogReadResolution(12);
  pinMode(pins::kChargeStatus, INPUT_PULLUP);
}

void readPower(app::BatterySnapshot &snapshot) {
  const int raw = analogRead(pins::kBatAdc);
  const float voltage = raw * 3.3f / 4095.0f * 2.0f;
  snapshot.voltage = voltage;
  snapshot.levelPercent = constrain((voltage - 3.3f) * 100.0f / 0.9f, 0.0f, 100.0f);
  snapshot.charging = digitalRead(pins::kChargeStatus) == LOW;
}

}  // namespace hardware
