#pragma once

#include "FS.h"
#include "../gnss/gnss.h"

class GPXLogger
{
public:
    GPXLogger();
    bool start_logging();
    void stop_logging();
    void log_point(GNSS &gnss);

private:
    File file;
    bool is_logging;
};