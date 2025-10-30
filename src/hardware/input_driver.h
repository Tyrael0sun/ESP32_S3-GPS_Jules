#pragma once

#include <functional>

#include "app/app_controller.h"

namespace hardware {

enum class ButtonEvent {
  None,
  ShortPress,
  DoubleClick,
  MediumPress,
  LongPress
};

class InputDriver {
 public:
  void begin();
  void poll(app::ApplicationController &controller);

 private:
  static void IRAM_ATTR handleEncoderA();
  static void IRAM_ATTR handleEncoderB();
  static void IRAM_ATTR handleButton();

  void processPendingShort(app::ApplicationController &controller, uint32_t nowMs);
  void processEncoder(app::ApplicationController &controller, uint32_t nowMs);
  void processButton(app::ApplicationController &controller, uint32_t nowMs);
  void dispatchButtonEvent(app::ApplicationController &controller, ButtonEvent event, uint32_t primaryDurationMs, uint32_t secondaryDurationMs = 0);

  int32_t lastEncoderCount_ = 0;
  int32_t encoderAccumulator_ = 0;
  bool awaitingDouble_ = false;
  bool pendingShort_ = false;
  uint32_t doubleDeadlineMs_ = 0;
  uint32_t pendingShortDurationMs_ = 0;
};

InputDriver &input();

}  // namespace hardware
