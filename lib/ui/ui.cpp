#include "ui.h"
#include "../display/display.h"
#include "../input/input.h"
#include "screens/settings_screen.h"
#include "screens/gps_logger_screen.h"
#include "screens/bicycle_computer_screen.h"
#include "screens/pbox_screen.h"

static Display display;
static Input input;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[240 * 240 / 10];

static lv_obj_t *main_screen;
static lv_obj_t *status_bar;

static void create_main_screen();
static void settings_button_event_handler(lv_event_t *e);
static void gps_logger_button_event_handler(lv_event_t *e);
static void bicycle_computer_button_event_handler(lv_event_t *e);
static void pbox_button_event_handler(lv_event_t *e);

UI::UI()
{
}

void UI::init()
{
    lv_init();

    display.init();

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 240 / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    create_main_screen();
}

void UI::update()
{
    lv_timer_handler();
}

void UI::display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    Display::lcd.pushImage(area->x1, area->y1, w, h, (lgfx::rgb565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

void UI::touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
    static int32_t last_encoder_val = 0;
    int32_t encoder_val = input.encoder.getCount();
    int32_t encoder_diff = encoder_val - last_encoder_val;
    last_encoder_val = encoder_val;

    data->enc_diff = encoder_diff;

    if (input.button.isPressed())
    {
        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void create_main_screen()
{
    main_screen = lv_obj_create(NULL);

    status_bar = lv_obj_create(main_screen);
    lv_obj_set_size(status_bar, 240, 30);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *sat_label = lv_label_create(status_bar);
    lv_label_set_text(sat_label, "Sats: 0");
    lv_obj_align(sat_label, LV_ALIGN_LEFT_MID, 5, 0);

    lv_obj_t *sd_label = lv_label_create(status_bar);
    lv_label_set_text(sd_label, "SD: OK");
    lv_obj_align(sd_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *bat_label = lv_label_create(status_bar);
    lv_label_set_text(bat_label, "Bat: 100%");
    lv_obj_align(bat_label, LV_ALIGN_RIGHT_MID, -5, 0);

    lv_obj_t *pbox_button = lv_btn_create(main_screen);
    lv_obj_add_event_cb(pbox_button, pbox_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(pbox_button, LV_ALIGN_CENTER, 0, -120);

    lv_obj_t *label = lv_label_create(pbox_button);
    lv_label_set_text(label, "P-Box");
    lv_obj_center(label);

    lv_obj_t *bicycle_computer_button = lv_btn_create(main_screen);
    lv_obj_add_event_cb(bicycle_computer_button, bicycle_computer_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(bicycle_computer_button, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(bicycle_computer_button);
    lv_label_set_text(label, "Bicycle Computer");
    lv_obj_center(label);

    lv_obj_t *gps_logger_button = lv_btn_create(main_screen);
    lv_obj_add_event_cb(gps_logger_button, gps_logger_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(gps_logger_button, LV_ALIGN_CENTER, 0, 40);

    label = lv_label_create(gps_logger_button);
    lv_label_set_text(label, "GPS Logger");
    lv_obj_center(label);

    lv_obj_t *settings_button = lv_btn_create(main_screen);
    lv_obj_add_event_cb(settings_button, settings_button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(settings_button, LV_ALIGN_CENTER, 0, 120);

    label = lv_label_create(settings_button);
    lv_label_set_text(label, "Settings");
    lv_obj_center(label);

    lv_scr_load(main_screen);
}

static void settings_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        create_settings_screen();
    }
}

static void gps_logger_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        create_gps_logger_screen();
    }
}

static void bicycle_computer_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        create_bicycle_computer_screen();
    }
}

static void pbox_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        create_pbox_screen();
    }
}
