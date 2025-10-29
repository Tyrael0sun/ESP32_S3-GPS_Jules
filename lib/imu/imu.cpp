#include "imu.h"

IMU::IMU()
{
}

bool IMU::init()
{
    return lsm6ds.begin_I2C();
}

void IMU::read()
{
    lsm6ds.getEvent(&accel, &gyro, &temp);
}