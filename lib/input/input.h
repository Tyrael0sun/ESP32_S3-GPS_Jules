#pragma once

#include <ESP32Encoder.h>
#include <Button2.h>
#include "../../src/config.h"

class Input
{
public:
    Input();
    void init();
    void update();

    ESP32Encoder encoder;
    Button2 button;

private:
};