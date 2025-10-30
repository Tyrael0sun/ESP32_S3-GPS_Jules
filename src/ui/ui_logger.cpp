#include "ui_logger.h"

#include <Arduino.h>

#include "ui_status_bar.h"
#include "util/serial_router.h"

namespace ui {

void LoggerScreen::init() {
  serial_router::printlnUsb(F("[UI] Logger screen init"));
}

void LoggerScreen::render(const RenderContext &ctx) {
  renderStatusBar(ctx.state);
  serial_router::printfUsb("[UI] Logger\tspd=%.1fkm/h\talt=%.1fm\tdur=%lu\trecording=%s\r\n",
                           ctx.state.gnss.speedKmh,
                           ctx.state.baro.altitudeM,
                           millis() / 1000UL,
                           ctx.state.gpxRecording ? "yes" : "no");
}

}  // namespace ui
