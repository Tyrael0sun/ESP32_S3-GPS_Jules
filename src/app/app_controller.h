#pragma once

#include "app_state.h"

namespace app {

class ApplicationController {
 public:
  ApplicationController();

  void cycleModeForward();
  void cycleModeBackward();
  void startOrStopRecording();
  void enterSettings();
  void toggleGnssScrollMode();
  void adjustGnssScroll(int16_t steps);

  SystemState &state() { return state_; }
  const SystemState &state() const { return state_; }

 private:
  void updateRecordingFlag(bool recording);

  SystemState state_{};
};

}  // namespace app
