#pragma once

#include <LovyanGFX.hpp>

namespace hardware {

lgfx::LGFX_Device &display();
void initDisplay();
void setBacklight(float percent);

}  // namespace hardware
