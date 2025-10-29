#pragma once

#include "FS.h"
#include "SD_MMC.h"

class SDCard
{
public:
    SDCard();
    bool init();
    void deinit();

private:
};