#pragma once

#include <Adafruit_LIS2MDL.h>

class Mag
{
public:
    Mag();
    bool init();
    void read();

    sensors_event_t mag;

private:
    Adafruit_LIS2MDL lis2mdl;
};