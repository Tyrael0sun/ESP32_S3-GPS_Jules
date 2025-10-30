#pragma once

#include "app/app_controller.h"
#include "app/app_state.h"

namespace hardware {

void initPeripherals();
void pollGnss(app::GnssSnapshot &snapshot);
void pollImu(app::ImuSnapshot &snapshot);
void pollMagnetometer(app::MagSnapshot &snapshot);
void pollBarometer(app::BaroSnapshot &snapshot);
void pollBattery(app::BatterySnapshot &snapshot);
void flushDiagnostics(const app::SystemState &state, unsigned long nowMs);
void pollInput(app::ApplicationController &controller);

}  // namespace hardware
