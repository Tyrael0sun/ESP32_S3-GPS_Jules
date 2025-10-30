#include "input_driver.h"

#include <Arduino.h>

#include "config.h"
#include "pin_config.h"
#include "util/serial_router.h"

namespace {
hardware::InputDriver g_input;
volatile int32_t g_encoderCount = 0;
volatile bool g_buttonPressed = false;
volatile uint32_t g_buttonPressedAt = 0;
volatile uint32_t g_buttonReleasedAt = 0;
volatile bool g_buttonQueued = false;
}  // namespace

namespace hardware {

void InputDriver::begin() {
  pinMode(pins::kEncA, INPUT_PULLUP);
  pinMode(pins::kEncB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pins::kEncA), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pins::kEncB), handleEncoderB, CHANGE);

  pinMode(pins::kKeyMain, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pins::kKeyMain), handleButton, CHANGE);
}

void InputDriver::poll(app::ApplicationController &controller) {
  const uint32_t now = millis();
  processPendingShort(controller, now);
  processEncoder(controller, now);
  processButton(controller, now);
}

void IRAM_ATTR InputDriver::handleEncoderA() {
  const bool a = digitalRead(pins::kEncA);
  const bool b = digitalRead(pins::kEncB);
  if (a == b) {
    ++g_encoderCount;
  } else {
    --g_encoderCount;
  }
}

void IRAM_ATTR InputDriver::handleEncoderB() {
  const bool a = digitalRead(pins::kEncA);
  const bool b = digitalRead(pins::kEncB);
  if (a != b) {
    ++g_encoderCount;
  } else {
    --g_encoderCount;
  }
}

void IRAM_ATTR InputDriver::handleButton() {
  const bool pressed = digitalRead(pins::kKeyMain) == LOW;
  const uint32_t now = millis();
  if (pressed) {
    g_buttonPressed = true;
    g_buttonPressedAt = now;
  } else if (g_buttonPressed) {
    g_buttonPressed = false;
    g_buttonReleasedAt = now;
    g_buttonQueued = true;
  }
}

void InputDriver::processEncoder(app::ApplicationController &controller, uint32_t nowMs) {
  int32_t count = 0;
  noInterrupts();
  count = g_encoderCount;
  interrupts();

  const int32_t delta = count - lastEncoderCount_;
  if (delta == 0) {
    return;
  }
  lastEncoderCount_ = count;
  encoderAccumulator_ += delta;

  const int32_t stepTicks = config::kEncoderTicksPerStep > 0 ? config::kEncoderTicksPerStep : 1;
  auto &dbg = serial_router::debug();

  while (encoderAccumulator_ >= stepTicks) {
    encoderAccumulator_ -= stepTicks;
    controller.cycleModeForward();
    dbg.printf("[INPUT] %lu ms: encoder CW (ticks=%ld)\r\n",
               static_cast<unsigned long>(nowMs),
               static_cast<long>(delta));
  }

  while (encoderAccumulator_ <= -stepTicks) {
    encoderAccumulator_ += stepTicks;
    controller.cycleModeBackward();
    dbg.printf("[INPUT] %lu ms: encoder CCW (ticks=%ld)\r\n",
               static_cast<unsigned long>(nowMs),
               static_cast<long>(delta));
  }
}

void InputDriver::processPendingShort(app::ApplicationController &controller, uint32_t nowMs) {
  if (!pendingShort_) {
    return;
  }
  if (static_cast<int32_t>(nowMs - doubleDeadlineMs_) < 0) {
    return;
  }
  pendingShort_ = false;
  awaitingDouble_ = false;
  dispatchButtonEvent(controller, ButtonEvent::ShortPress, pendingShortDurationMs_);
  pendingShortDurationMs_ = 0;
}

void InputDriver::processButton(app::ApplicationController &controller, uint32_t nowMs) {
  uint32_t pressedAt = 0;
  uint32_t releasedAt = 0;

  noInterrupts();
  const bool queued = g_buttonQueued;
  if (queued) {
    g_buttonQueued = false;
    pressedAt = g_buttonPressedAt;
    releasedAt = g_buttonReleasedAt;
  }
  interrupts();

  if (!queued) {
    return;
  }

  const uint32_t duration = releasedAt - pressedAt;
  if (duration < config::kButtonDebounceMs) {
    return;
  }

  if (duration < config::kButtonShortPressMaxMs) {
    if (awaitingDouble_ && static_cast<int32_t>(releasedAt - doubleDeadlineMs_) <= 0) {
      const uint32_t firstDuration = pendingShortDurationMs_;
      pendingShort_ = false;
      awaitingDouble_ = false;
      pendingShortDurationMs_ = 0;
      dispatchButtonEvent(controller, ButtonEvent::DoubleClick, firstDuration, duration);
    } else {
      awaitingDouble_ = true;
      pendingShort_ = true;
      pendingShortDurationMs_ = duration;
      doubleDeadlineMs_ = releasedAt + config::kButtonDoubleClickWindowMs;
    }
    return;
  }

  pendingShort_ = false;
  awaitingDouble_ = false;

  if (duration < config::kButtonMediumPressMaxMs) {
    dispatchButtonEvent(controller, ButtonEvent::MediumPress, duration);
  } else {
    dispatchButtonEvent(controller, ButtonEvent::LongPress, duration);
  }
}

void InputDriver::dispatchButtonEvent(app::ApplicationController &controller, ButtonEvent event, uint32_t primaryDurationMs, uint32_t secondaryDurationMs) {
  auto &dbg = serial_router::debug();
  switch (event) {
    case ButtonEvent::ShortPress:
      dbg.printf("[INPUT] %lu ms: button short (%lu ms)\r\n",
                 static_cast<unsigned long>(millis()),
                 static_cast<unsigned long>(primaryDurationMs));
      controller.cycleModeForward();
      break;
    case ButtonEvent::DoubleClick:
      dbg.printf("[INPUT] %lu ms: button double-click (%lu ms + %lu ms)\r\n",
                 static_cast<unsigned long>(millis()),
                 static_cast<unsigned long>(primaryDurationMs),
                 static_cast<unsigned long>(secondaryDurationMs));
      controller.startOrStopRecording();
      break;
    case ButtonEvent::MediumPress:
      dbg.printf("[INPUT] %lu ms: button medium (%lu ms)\r\n",
                 static_cast<unsigned long>(millis()),
                 static_cast<unsigned long>(primaryDurationMs));
      controller.startOrStopRecording();
      break;
    case ButtonEvent::LongPress:
      dbg.printf("[INPUT] %lu ms: button long (%lu ms)\r\n",
                 static_cast<unsigned long>(millis()),
                 static_cast<unsigned long>(primaryDurationMs));
      controller.enterSettings();
      break;
    case ButtonEvent::None:
      break;
  }
}

InputDriver &input() {
  return g_input;
}

}  // namespace hardware
