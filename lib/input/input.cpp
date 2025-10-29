#include "input.h"

Input::Input()
{
}

void Input::init()
{
    encoder.attachHalfQuad(ENC_A_PIN, ENC_B_PIN);
    button.begin(KEY_MAIN_PIN, INPUT_PULLUP);
}

void Input::update()
{
    encoder.getCount();
    button.loop();
}