#pragma once

namespace pins {
// Display (SPI3)
constexpr int kDispSck = 5;
constexpr int kDispMosi = 8;
constexpr int kDispCs = 7;
constexpr int kDispDc = 6;
constexpr int kDispRst = 4;
constexpr int kDispBl = 9;

// GNSS UART1
constexpr int kGnssTx = 17;
constexpr int kGnssRx = 18;
constexpr int kGnssLdoEn = 14;

// I2C bus
constexpr int kI2cScl = 39;
constexpr int kI2cSda = 40;

// SDIO
constexpr int kSdCmd = 35;
constexpr int kSdClk = 36;
constexpr int kSdD0 = 37;
constexpr int kSdD1 = 38;
constexpr int kSdD2 = 34;
constexpr int kSdD3 = 33;

// Encoder and button
constexpr int kEncA = 1;
constexpr int kEncB = 3;
constexpr int kKeyMain = 2;

// Battery
constexpr int kBatAdc = 12;
constexpr int kChargeStatus = 21;

// IMU interrupts (unused but reserved)
constexpr int kAccGyroInt = 41;
constexpr int kMagInt = 42;
constexpr int kPressInt = 13;

// Debug UART (USB CDC is Serial)
constexpr int kDebugTx = 43;
constexpr int kDebugRx = 44;
}  // namespace pins
