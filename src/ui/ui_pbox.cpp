#include "ui_pbox.h"

#include <Arduino.h>

#include "config.h"
#include "ui_status_bar.h"
#include "ui_lvgl_util.h"

namespace ui {

void PboxScreen::init() {
  if (root_) {
    return;
  }

  ensureLvglInitialized();
  const lv_coord_t width = lv_disp_get_hor_res(nullptr);
  const lv_coord_t height = lv_disp_get_ver_res(nullptr);
  root_ = lv_obj_create(nullptr);
  lv_obj_set_size(root_, width, height);
  lv_obj_set_style_bg_color(root_, lv_color_hex(0x101010), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);

  statusBar_.init(root_);
  const lv_coord_t statusHeight = StatusBar::kHeight;
  const lv_coord_t contentHeight = height > statusHeight ? height - statusHeight : height;

  lv_obj_t *content = lv_obj_create(root_);
  lv_obj_set_size(content, width, contentHeight);
  lv_obj_set_pos(content, 0, statusHeight);
  lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(content, 0, 0);
  lv_obj_set_style_pad_all(content, 12, 0);
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

  speedLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(speedLabel_, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(speedLabel_, lv_color_hex(0x00FF00), 0);
  lv_label_set_text(speedLabel_, "Speed 0.0 km/h");
  lv_obj_align(speedLabel_, LV_ALIGN_TOP_LEFT, 0, 10);

  statusLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(statusLabel_, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(statusLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(statusLabel_, "Status: Idle");
  lv_obj_align_to(statusLabel_, speedLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

  timerLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(timerLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(timerLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(timerLabel_, "Timer 00:00.000");
  lv_obj_align_to(timerLabel_, statusLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

void PboxScreen::render(const RenderContext &ctx) {
  if (!root_) {
    return;
  }

  statusBar_.update(ctx.state);

  const auto &state = ctx.state;
  const bool ready = state.gnss.speedKmh < config::kPboxStartSpeedKmh &&
                     state.imu.accelX > config::kPboxStartAccelG;

  lv_label_set_text_fmt(speedLabel_, "Speed %.1f km/h", state.gnss.speedKmh);
  lv_label_set_text(statusLabel_, ready ? "Status: Armed" : "Status: Idle");

  const uint32_t elapsed = millis();
  const uint32_t seconds = (elapsed / 1000UL) % 60UL;
  const uint32_t minutes = (elapsed / 60000UL) % 60UL;
  const uint32_t hours = elapsed / 3600000UL;
  const uint32_t millisPart = elapsed % 1000UL;
  lv_label_set_text_fmt(timerLabel_, "Timer %02lu:%02lu:%02lu.%03lu",
                        static_cast<unsigned long>(hours),
                        static_cast<unsigned long>(minutes),
                        static_cast<unsigned long>(seconds),
                        static_cast<unsigned long>(millisPart));
}

}  // namespace ui
