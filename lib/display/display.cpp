#include "display.h"

LGFX Display::lcd;

Display::Display()
{
}

void Display::init()
{
    lcd.begin();
}

void Display::update()
{
}
