#include <Arduino.h>
#include "config.h"
#include "../lib/logging/logging.h"
#include "../lib/ui/ui.h"
#include "../lib/display/display.h"
#include "../lib/imu/imu.h"
#include "../lib/mag/mag.h"
#include "../lib/pressure/pressure.h"
#include "../lib/gnss/gnss.h"
#include "../lib/sd_card/sd_card.h"
#include "../lib/input/input.h"
#include "../lib/calibration/calibration.h"
#include "../lib/data_manager/data_manager.h"

// Centralized hardware driver instances
Display display;
IMU imu;
Mag mag;
Pressure pressure;
GNSS gnss;
SDCard sd_card;
Input input;
Calibration calibration;
DataManager data_manager;

Logger logger;
UI ui;

void setup() {
  logger.init();
  logger.log_status();
  ui.init();
  data_manager.init();
}

void loop() {
  data_manager.update();
  ui.update();
  delay(5);
}
