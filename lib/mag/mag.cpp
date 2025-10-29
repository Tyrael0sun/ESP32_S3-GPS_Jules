#include "mag.h"

Mag::Mag()
{
}

bool Mag::init()
{
    return lis2mdl.begin();
}

void Mag::read()
{
    lis2mdl.getEvent(&mag);
}