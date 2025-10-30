#pragma once

#include "app/app_state.h"

namespace hardware {

void initPowerMonitoring();
void readPower(app::BatterySnapshot &snapshot);

}  // namespace hardware
