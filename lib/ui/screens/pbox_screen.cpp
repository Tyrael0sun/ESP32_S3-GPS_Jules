#include "pbox_screen.h"
#include "../../gnss/gnss.h"
#include "../../imu/imu.h"

extern GNSS gnss;
extern IMU imu;

static lv_obj_t *status_label;
static lv_obj_t *time_label;
static lv_obj_t *speed_label;

enum TestState {
    IDLE,
    ARMED,
    TIMING,
    FINISHED
};

static TestState test_state = IDLE;
static unsigned long start_time = 0;
static float result_time = 0;

static void reset_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        test_state = ARMED;
        lv_label_set_text(status_label, "Status: Armed");
        lv_label_set_text(time_label, "Time: 0.00s");
    }
}

static void update_task(lv_timer_t *timer)
{
    float speed = gnss.gps.speed.kmph();
    float accel_g = imu.accel.acceleration.x / 9.81; // Assuming X is longitudinal axis

    char buf[32];
    sprintf(buf, "Speed: %.2f km/h", speed);
    lv_label_set_text(speed_label, buf);

    switch (test_state)
    {
    case IDLE:
        lv_label_set_text(status_label, "Status: Press Reset");
        break;
    case ARMED:
        if (speed < 1.0 && accel_g > 0.15)
        {
            start_time = millis();
            test_state = TIMING;
            lv_label_set_text(status_label, "Status: Timing...");
        }
        break;
    case TIMING:
    {
        float current_time = (millis() - start_time) / 1000.0f;
        sprintf(buf, "Time: %.2fs", current_time);
        lv_label_set_text(time_label, buf);

        if (speed >= 100.0)
        {
            result_time = current_time;
            test_state = FINISHED;
        }
        break;
    }
    case FINISHED:
        lv_label_set_text(status_label, "Status: Finished!");
        sprintf(buf, "Time: %.2fs", result_time);
        lv_label_set_text(time_label, buf);
        test_state = IDLE; // Auto-reset to idle state, user must press reset to arm again
        break;
    }
}

void create_pbox_screen()
{
    lv_obj_t *screen = lv_obj_create(NULL);

    status_label = lv_label_create(screen);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 40);

    time_label = lv_label_create(screen);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, 0);

    speed_label = lv_label_create(screen);
    lv_obj_align(speed_label, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t *reset_button = lv_btn_create(screen);
    lv_obj_add_event_cb(reset_button, reset_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(reset_button, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_obj_t *label = lv_label_create(reset_button);
    lv_label_set_text(label, "Reset");
    lv_obj_center(label);

    lv_timer_create(update_task, 10, NULL); // High frequency timer for accurate measurement

    lv_scr_load(screen);
}