#include "pressure.h"

Pressure::Pressure()
{
}

bool Pressure::init()
{
    return bmp.begin_I2C();
}

void Pressure::read()
{
    bmp.performReading();
    temperature = bmp.temperature;
    pressure = bmp.pressure;
}