#pragma once

#include <stdint.h>
#include <cstddef>

namespace config {
// GNSS configuration defaults
constexpr uint8_t kDefaultGnssRateHz = 5;
constexpr uint8_t kGnssRateOptions[] = {1, 5, 10, 25};
constexpr uint32_t kGnssStartupBaud = 9600;
constexpr uint32_t kGnssTargetBaud = 115200;
constexpr size_t kGnssSerialRxBufferSize = 1536; // enlarge GNSS UART RX buffer
constexpr uint32_t kGnssSerialWatchdogMs = 1500; // restart GNSS UART after this idle window (ms)
constexpr uint32_t kGnssSerialRecoveryDelayMs = 60; // settle time before reading after restart (ms)
constexpr uint32_t kGnssSerialRecoveryDrainMs = 120; // drain window to re-sync NMEA stream (ms)

// Input timing thresholds in milliseconds
constexpr uint16_t kButtonDebounceMs = 100;
constexpr uint16_t kButtonShortPressMaxMs = 1000;
constexpr uint16_t kButtonMediumPressMinMs = 1000;
constexpr uint16_t kButtonMediumPressMaxMs = 3000;
constexpr uint16_t kButtonLongPressMinMs = 3000;
constexpr uint16_t kButtonDoubleClickWindowMs = 400;

// Encoder smoothing
constexpr int32_t kEncoderTicksPerStep = 12;

// Performance thresholds
constexpr float kPboxStartSpeedKmh = 1.0f;
constexpr float kPboxStartAccelG = 0.15f;

// Status bar color codes (placeholder values)
constexpr uint32_t kSatCountLockedColor = 0x00FF00; // green when locked
constexpr uint32_t kSatCountSearchingColor = 0xFF0000; // red while searching

// Logging
constexpr uint32_t kDiagHighRateMs = 1000;
constexpr uint32_t kDiagHighRateDurationMs = 5000;
constexpr uint32_t kUsbSerialInitTimeoutMs = 250;
} // namespace config
