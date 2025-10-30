#include <Arduino.h>
#include <lvgl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app/app_controller.h"
#include "config.h"
#include "hardware/hardware_stubs.h"
#include "ui/ui_bike_computer.h"
#include "ui/ui_logger.h"
#include "ui/ui_pbox.h"
#include "ui/ui_settings.h"
#include "ui/ui_lvgl_util.h"

namespace {
app::ApplicationController controller;
ui::BikeComputerScreen bikeScreen;
ui::PboxScreen pboxScreen;
ui::LoggerScreen loggerScreen;
ui::SettingsScreen settingsScreen;
ui::UIScreen *activeScreen = nullptr;

ui::UIScreen *screenForMode(app::Mode mode) {
  switch (mode) {
    case app::Mode::BikeComputer:
      return &bikeScreen;
    case app::Mode::PBox:
      return &pboxScreen;
    case app::Mode::Logger:
      return &loggerScreen;
    case app::Mode::Settings:
      return &settingsScreen;
  }
  return &bikeScreen;
}

void ensureScreenInitialized(ui::UIScreen *screen) {
  static bool initializedBike = false;
  static bool initializedPbox = false;
  static bool initializedLogger = false;
  static bool initializedSettings = false;

  if (screen == &bikeScreen && !initializedBike) {
    screen->init();
    initializedBike = true;
  } else if (screen == &pboxScreen && !initializedPbox) {
    screen->init();
    initializedPbox = true;
  } else if (screen == &loggerScreen && !initializedLogger) {
    screen->init();
    initializedLogger = true;
  } else if (screen == &settingsScreen && !initializedSettings) {
    screen->init();
    initializedSettings = true;
  }
}

void updateSensors() {
  hardware::pollGnss(controller.state().gnss);
  hardware::pollImu(controller.state().imu);
  hardware::pollMagnetometer(controller.state().mag);
  hardware::pollBarometer(controller.state().baro);
  hardware::pollBattery(controller.state().battery);
}

void renderActiveScreen() {
  ui::RenderContext ctx{controller.state()};
  ui::UIScreen *desired = screenForMode(controller.state().activeMode);
  if (desired != activeScreen) {
    ensureScreenInitialized(desired);
    if (desired && desired->root()) {
      lv_disp_load_scr(desired->root());
    }
    activeScreen = desired;
  }
  if (activeScreen) {
    activeScreen->render(ctx);
  }
}

}  // namespace

void setup() {
  hardware::initPeripherals();
  ui::ensureLvglInitialized();
  static TaskHandle_t lvglTaskHandle = nullptr;
  xTaskCreatePinnedToCore(
      [](void *) {
        while (true) {
          lv_timer_handler();
          vTaskDelay(pdMS_TO_TICKS(5));
        }
      },
      "lvgl", 4096, nullptr, 1, &lvglTaskHandle, 1);
  ensureScreenInitialized(screenForMode(controller.state().activeMode));
  activeScreen = screenForMode(controller.state().activeMode);
  if (activeScreen && activeScreen->root()) {
    lv_disp_load_scr(activeScreen->root());
  }
}

void loop() {
  const unsigned long now = millis();
  hardware::pollInput(controller);
  updateSensors();
  renderActiveScreen();
  hardware::flushDiagnostics(controller.state(), now);
  delay(10);
}
