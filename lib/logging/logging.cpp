#include "logging.h"
#include <Arduino.h>
#include "../display/display.h"
#include "../imu/imu.h"
#include "../mag/mag.h"
#include "../pressure/pressure.h"
#include "../gnss/gnss.h"
#include "../sd_card/sd_card.h"
#include "../calibration/calibration.h"

extern Display display;
extern IMU imu;
extern Mag mag;
extern Pressure pressure;
extern GNSS gnss;
extern SDCard sd_card;
extern Calibration calibration;

Logger::Logger()
{
}

void Logger::init()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Logger initialized");
}

void Logger::log_status()
{
    Serial.println("--- Hardware Status ---");

    display.init();
    Serial.println("Display: Initialized");

    if (imu.init())
    {
        Serial.println("IMU (LSM6DSR): OK");
    }
    else
    {
        Serial.println("IMU (LSM6DSR): Not found");
    }

    if (mag.init())
    {
        Serial.println("Magnetometer (LIS2MDL): OK");
    }
    else
    {
        Serial.println("Magnetometer (LIS2MDL): Not found");
    }

    if (pressure.init())
    {
        Serial.println("Pressure (BMP388): OK");
    }
    else
    {
        Serial.println("Pressure (BMP388): Not found");
    }

    gnss.init();
    Serial.println("GNSS: Initialized");

    if (sd_card.init())
    {
        Serial.println("SD Card: OK");
    }
    else
    {
        Serial.println("SD Card: Not found");
    }

    if (calibration.load_calibration())
    {
        Serial.println("Calibration: Loaded from NVS");
    }
    else
    {
        Serial.println("Calibration: Not found in NVS");
    }

    Serial.println("-----------------------");
}