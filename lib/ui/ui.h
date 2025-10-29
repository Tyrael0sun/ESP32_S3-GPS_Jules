#pragma once

#include "lvgl.h"

class UI
{
public:
    UI();
    void init();
    void update();

private:
    static void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static void touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
};