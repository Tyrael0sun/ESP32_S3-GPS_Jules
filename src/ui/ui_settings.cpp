#include "ui_settings.h"

#include <Arduino.h>

#include "config.h"
#include "ui_status_bar.h"
#include "util/serial_router.h"

namespace ui {

void SettingsScreen::init() {
  serial_router::printlnUsb(F("[UI] Settings screen init"));
}

void SettingsScreen::render(const RenderContext &ctx) {
  renderStatusBar(ctx.state);
  serial_router::printfUsb("[UI] Settings\tGNSS rate=%uhz\tPBOX accel=%.2fg\r\n",
                           config::kDefaultGnssRateHz,
                           config::kPboxStartAccelG);
}

}  // namespace ui
