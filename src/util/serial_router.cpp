#include "serial_router.h"

#include <Arduino.h>

namespace {
HardwareSerial debugSerial(0);
}  // namespace

namespace serial_router {

void init() {
  Serial.begin(115200);
  const unsigned long start = millis();
  while (!Serial && millis() - start < 2000) {
    delay(10);
  }

  constexpr int kDebugRxPin = 44;
  constexpr int kDebugTxPin = 43;
  debugSerial.begin(115200, SERIAL_8N1, kDebugRxPin, kDebugTxPin);
}

HardwareSerial &debug() {
  return debugSerial;
}

}  // namespace serial_router
