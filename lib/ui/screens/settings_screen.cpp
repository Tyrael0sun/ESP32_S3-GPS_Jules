#include "settings_screen.h"
#include "../calibration/calibration.h"

extern Calibration calibration;

static void calibration_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        calibration.start_accel_mag_calibration();
    }
}

static void gnss_config_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        LV_LOG_USER("GNSS config button clicked");
    }
}

void create_settings_screen()
{
    lv_obj_t *settings_screen = lv_obj_create(NULL);

    lv_obj_t *calibration_button = lv_btn_create(settings_screen);
    lv_obj_add_event_cb(calibration_button, calibration_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(calibration_button, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t *label = lv_label_create(calibration_button);
    lv_label_set_text(label, "Calibrate Sensors");
    lv_obj_center(label);

    lv_obj_t *gnss_config_button = lv_btn_create(settings_screen);
    lv_obj_add_event_cb(gnss_config_button, gnss_config_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(gnss_config_button, LV_ALIGN_CENTER, 0, 40);

    label = lv_label_create(gnss_config_button);
    lv_label_set_text(label, "GNSS Config");
    lv_obj_center(label);

    lv_scr_load(settings_screen);
}