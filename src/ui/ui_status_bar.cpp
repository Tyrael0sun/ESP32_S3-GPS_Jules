#include "ui_status_bar.h"

#include <Arduino.h>

#include "config.h"
#include "util/serial_router.h"

namespace ui {

void renderStatusBar(const app::SystemState &state) {
  const uint32_t color = state.gnss.fix ? config::kSatCountLockedColor : config::kSatCountSearchingColor;
  serial_router::printfUsb("[STATUS] sats=%u color=0x%06lx sd=%s batt=%.1f%% charging=%s\r\n",
                           state.gnss.satellites,
                           static_cast<unsigned long>(color),
                           "ok",
                           state.battery.levelPercent,
                           state.battery.charging ? "yes" : "no");
}

}  // namespace ui
