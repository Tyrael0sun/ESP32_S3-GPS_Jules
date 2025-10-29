#include "data_manager.h"
#include "../imu/imu.h"
#include "../mag/mag.h"
#include "../pressure/pressure.h"
#include "../gnss/gnss.h"
#include "../input/input.h"

extern IMU imu;
extern Mag mag;
extern Pressure pressure;
extern GNSS gnss;
extern Input input;

DataManager::DataManager()
{
}

void DataManager::init()
{
}

void DataManager::update()
{
    imu.read();
    mag.read();
    pressure.read();
    gnss.update();
    input.update();
}