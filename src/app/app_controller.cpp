#include "app_controller.h"

namespace app {

ApplicationController::ApplicationController() {
  state_.activeMode = Mode::BikeComputer;
}

void ApplicationController::cycleModeForward() {
  switch (state_.activeMode) {
    case Mode::BikeComputer:
      state_.activeMode = Mode::PBox;
      break;
    case Mode::PBox:
      state_.activeMode = Mode::Logger;
      break;
    case Mode::Logger:
      state_.activeMode = Mode::BikeComputer;
      break;
    case Mode::Settings:
      state_.activeMode = Mode::BikeComputer;
      break;
  }
}

void ApplicationController::cycleModeBackward() {
  switch (state_.activeMode) {
    case Mode::BikeComputer:
      state_.activeMode = Mode::Logger;
      break;
    case Mode::PBox:
      state_.activeMode = Mode::BikeComputer;
      break;
    case Mode::Logger:
      state_.activeMode = Mode::PBox;
      break;
    case Mode::Settings:
      state_.activeMode = Mode::BikeComputer;
      break;
  }
}

void ApplicationController::startOrStopRecording() {
  const bool newState = !state_.gpxRecording;
  updateRecordingFlag(newState);
}

void ApplicationController::enterSettings() {
  state_.activeMode = Mode::Settings;
}

void ApplicationController::updateRecordingFlag(bool recording) {
  state_.gpxRecording = recording;
}

}  // namespace app
