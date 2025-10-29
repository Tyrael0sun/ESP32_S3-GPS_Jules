#pragma once

#include <Adafruit_LSM6DS.h>

class IMU
{
public:
    IMU();
    bool init();
    void read();

    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

private:
    Adafruit_LSM6DS lsm6ds;
};