#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

namespace serial_router {

// Initializes USB CDC and UART0 debug ports.
void init();

// Returns the dedicated UART0 debug interface.
HardwareSerial &debug();

// Helper utilities to emit data over USB CDC (Serial).
template <typename... Args>
inline void printfUsb(const char *fmt, Args... args) {
	Serial.printf(fmt, args...);
}

inline void printlnUsb(const __FlashStringHelper *message) {
	Serial.println(message);
}

inline void printlnUsb(const char *message) {
	Serial.println(message);
}

}  // namespace serial_router
