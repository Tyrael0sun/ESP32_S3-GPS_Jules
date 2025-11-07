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
      state_.activeMode = Mode::GnssDebug;
      break;
    case Mode::GnssDebug:
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
      state_.activeMode = Mode::GnssDebug;
      break;
    case Mode::PBox:
      state_.activeMode = Mode::BikeComputer;
      break;
    case Mode::Logger:
      state_.activeMode = Mode::PBox;
      break;
    case Mode::GnssDebug:
      state_.activeMode = Mode::Logger;
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

void ApplicationController::toggleGnssScrollMode() {
  state_.gnss.scrollMode = !state_.gnss.scrollMode;
  if (!state_.gnss.scrollMode) {
    state_.gnss.scrollRow = 0;
  }
}

void ApplicationController::adjustGnssScroll(int16_t steps) {
  if (steps == 0) {
    return;
  }
  auto &gnss = state_.gnss;
  if (gnss.satelliteCount == 0) {
    gnss.scrollRow = 0;
    return;
  }
  const int maxRow = static_cast<int>(gnss.satelliteCount) - 1;
  int nextRow = static_cast<int>(gnss.scrollRow) + steps;
  if (nextRow < 0) {
    nextRow = 0;
  } else if (nextRow > maxRow) {
    nextRow = maxRow;
  }
  gnss.scrollRow = static_cast<int16_t>(nextRow);
}

}  // namespace app
