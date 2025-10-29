#include "gnss.h"
#include "../../src/config.h"

GNSS::GNSS() : gps_serial(1)
{
}

void GNSS::init()
{
    gps_serial.begin(115200, SERIAL_8N1, GNSS_RX_PIN, GNSS_TX_PIN);
}

void GNSS::update()
{
    while (gps_serial.available() > 0)
    {
        gps.encode(gps_serial.read());
    }
}