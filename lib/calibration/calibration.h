#pragma once

#include <Adafruit_Sensor_Calibration.h>
#include <Preferences.h>

class Calibration
{
public:
    Calibration();
    void start_accel_mag_calibration();
    bool load_calibration();
    bool save_calibration();

private:
    Preferences preferences;
};