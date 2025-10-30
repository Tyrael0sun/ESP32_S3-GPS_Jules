#include "ui_pbox.h"

#include <Arduino.h>

#include "config.h"
#include "ui_status_bar.h"
#include "util/serial_router.h"

namespace ui {

void PboxScreen::init() {
  serial_router::printlnUsb(F("[UI] P-Box screen init"));
}

void PboxScreen::render(const RenderContext &ctx) {
  renderStatusBar(ctx.state);
  const auto &state = ctx.state;
  const bool ready = state.gnss.speedKmh < config::kPboxStartSpeedKmh &&
                     state.imu.accelX > config::kPboxStartAccelG;
  serial_router::printfUsb("[UI] PBOX\tspd=%.1fkm/h\ttime=%lu\ttrigger=%s\r\n",
                           state.gnss.speedKmh,
                           millis() / 1000UL,
                           ready ? "armed" : "idle");
}

}  // namespace ui
