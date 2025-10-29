#include "sd_card.h"
#include "../../src/config.h"

SDCard::SDCard()
{
}

bool SDCard::init()
{
    return SD_MMC.begin();
}

void SDCard::deinit()
{
    SD_MMC.end();
}