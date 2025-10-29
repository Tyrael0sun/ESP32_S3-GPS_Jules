#include "gpx_logger.h"
#include "SD_MMC.h"
#include <Arduino.h>

GPXLogger::GPXLogger() : is_logging(false)
{
}

bool GPXLogger::start_logging()
{
    if (is_logging)
    {
        return true;
    }

    if (!SD_MMC.exists("/GPX"))
    {
        SD_MMC.mkdir("/GPX");
    }

    // Create a unique filename
    char filename[30];
    sprintf(filename, "/GPX/track_%ld.gpx", millis());

    file = SD_MMC.open(filename, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    is_logging = true;

    // Write GPX header
    file.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    file.println("<gpx version=\"1.1\" creator=\"ESP32 GPS Logger\">");
    file.println("  <trk>");
    file.println("    <name>Track</name>");
    file.println("    <trkseg>");

    return true;
}

void GPXLogger::stop_logging()
{
    if (!is_logging)
    {
        return;
    }

    // Write GPX footer
    file.println("    </trkseg>");
    file.println("  </trk>");
    file.println("</gpx>");
    file.close();

    is_logging = false;
}

void GPXLogger::log_point(GNSS &gnss)
{
    if (!is_logging || !gnss.gps.location.isValid())
    {
        return;
    }

    file.print("      <trkpt lat=\"");
    file.print(gnss.gps.location.lat(), 6);
    file.print("\" lon=\"");
    file.print(gnss.gps.location.lng(), 6);
    file.println("\">");

    if (gnss.gps.altitude.isValid())
    {
        file.print("        <ele>");
        file.print(gnss.gps.altitude.meters());
        file.println("</ele>");
    }

    if (gnss.gps.time.isValid())
    {
        char iso_time[30];
        sprintf(iso_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                gnss.gps.date.year(),
                gnss.gps.date.month(),
                gnss.gps.date.day(),
                gnss.gps.time.hour(),
                gnss.gps.time.minute(),
                gnss.gps.time.second());
        file.print("        <time>");
        file.print(iso_time);
        file.println("</time>");
    }

    file.println("      </trkpt>");
}