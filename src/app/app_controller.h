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

  SystemState &state() { return state_; }
  const SystemState &state() const { return state_; }

 private:
  void updateRecordingFlag(bool recording);

  SystemState state_{};
};

}  // namespace app
