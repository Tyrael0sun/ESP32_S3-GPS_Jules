#include "ui_bike_computer.h"

#include <Arduino.h>

#include "ui_status_bar.h"
#include "util/serial_router.h"

namespace ui {

void BikeComputerScreen::init() {
  serial_router::printlnUsb(F("[UI] Bike computer screen init"));
}

void BikeComputerScreen::render(const RenderContext &ctx) {
  renderStatusBar(ctx.state);
  const auto &gnss = ctx.state.gnss;
  const auto &baro = ctx.state.baro;
  serial_router::printfUsb("[UI] Bike\tspd=%.1fkm/h\talt=%.1fm\tdist=%.2fkm\ttime=%lu\trecording=%s\r\n",
                           gnss.speedKmh,
                           baro.altitudeM,
                           0.0f,
                           millis() / 1000UL,
                           ctx.state.gpxRecording ? "yes" : "no");
}

}  // namespace ui
