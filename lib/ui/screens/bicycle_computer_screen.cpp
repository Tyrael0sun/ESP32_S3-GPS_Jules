#include "bicycle_computer_screen.h"
#include "../../gnss/gnss.h"

extern GNSS gnss;

static lv_obj_t *speed_label;
static lv_obj_t *alt_label;
static lv_obj_t *dist_label;

static float total_distance = 0;
static TinyGPSLocation last_location;

static void update_task(lv_timer_t *timer)
{
    if (gnss.gps.location.isValid() && gnss.gps.location.age() < 1000)
    {
        if (last_location.isValid())
        {
            float distance_moved = TinyGPSPlus::distanceBetween(
                gnss.gps.location.lat(),
                gnss.gps.location.lng(),
                last_location.lat(),
                last_location.lng());

            if (distance_moved > 0.002) // Ignore small movements
            {
                total_distance += distance_moved / 1000.0; // convert to km
            }
        }
        last_location = gnss.gps.location;
    }

    char buf[32];
    sprintf(buf, "Speed: %.2f km/h", gnss.gps.speed.kmph());
    lv_label_set_text(speed_label, buf);

    sprintf(buf, "Alt: %.2f m", gnss.gps.altitude.meters());
    lv_label_set_text(alt_label, buf);

    sprintf(buf, "Dist: %.2f km", total_distance);
    lv_label_set_text(dist_label, buf);
}

void create_bicycle_computer_screen()
{
    lv_obj_t *screen = lv_obj_create(NULL);

    speed_label = lv_label_create(screen);
    lv_obj_align(speed_label, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_text_font(speed_label, &lv_font_montserrat_24, 0);

    alt_label = lv_label_create(screen);
    lv_obj_align(alt_label, LV_ALIGN_CENTER, 0, 0);

    dist_label = lv_label_create(screen);
    lv_obj_align(dist_label, LV_ALIGN_CENTER, 0, 40);

    lv_timer_create(update_task, 200, NULL); // Slower update rate for display

    lv_scr_load(screen);
}
