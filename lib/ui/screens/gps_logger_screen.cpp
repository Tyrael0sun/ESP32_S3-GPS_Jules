#include "gps_logger_screen.h"
#include "../../gpx_logger/gpx_logger.h"
#include "../../gnss/gnss.h"

static GPXLogger gpx_logger;
extern GNSS gnss;

static lv_obj_t *lat_label;
static lv_obj_t *lon_label;
static lv_obj_t *alt_label;
static lv_obj_t *speed_label;
static lv_obj_t *log_button_label;

static bool is_logging = false;

static void log_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        is_logging = !is_logging;
        if (is_logging)
        {
            if (gpx_logger.start_logging())
            {
                lv_label_set_text(log_button_label, "Stop Logging");
            }
            else
            {
                is_logging = false; // Failed to start
            }
        }
        else
        {
            gpx_logger.stop_logging();
            lv_label_set_text(log_button_label, "Start Logging");
        }
    }
}

static void update_task(lv_timer_t *timer)
{
    if (is_logging)
    {
        gpx_logger.log_point(gnss);
    }

    char buf[32];
    sprintf(buf, "Lat: %.6f", gnss.gps.location.lat());
    lv_label_set_text(lat_label, buf);

    sprintf(buf, "Lon: %.6f", gnss.gps.location.lng());
    lv_label_set_text(lon_label, buf);

    sprintf(buf, "Alt: %.2f m", gnss.gps.altitude.meters());
    lv_label_set_text(alt_label, buf);

    sprintf(buf, "Speed: %.2f km/h", gnss.gps.speed.kmph());
    lv_label_set_text(speed_label, buf);
}

void create_gps_logger_screen()
{
    lv_obj_t *screen = lv_obj_create(NULL);

    lat_label = lv_label_create(screen);
    lv_obj_align(lat_label, LV_ALIGN_TOP_LEFT, 10, 40);

    lon_label = lv_label_create(screen);
    lv_obj_align(lon_label, LV_ALIGN_TOP_LEFT, 10, 70);

    alt_label = lv_label_create(screen);
    lv_obj_align(alt_label, LV_ALIGN_TOP_LEFT, 10, 100);

    speed_label = lv_label_create(screen);
    lv_obj_align(speed_label, LV_ALIGN_TOP_LEFT, 10, 130);

    lv_obj_t *log_button = lv_btn_create(screen);
    lv_obj_add_event_cb(log_button, log_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(log_button, LV_ALIGN_BOTTOM_MID, 0, -20);

    log_button_label = lv_label_create(log_button);
    lv_label_set_text(log_button_label, "Start Logging");
    lv_obj_center(log_button_label);

    lv_timer_create(update_task, 200, NULL);

    lv_scr_load(screen);
}