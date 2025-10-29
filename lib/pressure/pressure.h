#pragma once

#include <Adafruit_BMP3XX.h>

class Pressure
{
public:
    Pressure();
    bool init();
    void read();

    double temperature;
    double pressure;
    double altitude;

private:
    Adafruit_BMP3XX bmp;
};