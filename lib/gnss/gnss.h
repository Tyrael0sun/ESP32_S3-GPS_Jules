#pragma once

#include <TinyGPS++.h>
#include "config.h"

class GNSS
{
public:
    GNSS();
    void init();
    void update();

    TinyGPSPlus gps;

private:
    HardwareSerial gps_serial;
};