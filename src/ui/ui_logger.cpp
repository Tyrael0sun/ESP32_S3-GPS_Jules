#include "ui_logger.h"

#include <Arduino.h>

#include "ui_status_bar.h"
#include "ui_lvgl_util.h"

namespace ui {

void LoggerScreen::init() {
  if (root_) {
    return;
  }

  ensureLvglInitialized();
  const lv_coord_t width = lv_disp_get_hor_res(nullptr);
  const lv_coord_t height = lv_disp_get_ver_res(nullptr);
  root_ = lv_obj_create(nullptr);
  lv_obj_set_size(root_, width, height);
  lv_obj_set_style_bg_color(root_, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);

  statusBar_.init(root_);
  const lv_obj_t *statusContainer = statusBar_.container();
  const lv_coord_t statusHeight = statusContainer ? lv_obj_get_height(statusContainer) : 0;
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

  altLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(altLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(altLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(altLabel_, "Alt 0 m");
  lv_obj_align_to(altLabel_, speedLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

  durationLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(durationLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(durationLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(durationLabel_, "Duration 00:00:00");
  lv_obj_align_to(durationLabel_, altLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

  recordLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(recordLabel_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(recordLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(recordLabel_, "Recording: OFF");
  lv_obj_align(recordLabel_, LV_ALIGN_BOTTOM_LEFT, 0, -10);
}

void LoggerScreen::render(const RenderContext &ctx) {
  if (!root_) {
    return;
  }

  statusBar_.update(ctx.state);

  lv_label_set_text_fmt(speedLabel_, "Speed %.1f km/h", ctx.state.gnss.speedKmh);
  lv_label_set_text_fmt(altLabel_, "Alt %.0f m", ctx.state.baro.altitudeM);

  const uint32_t elapsed = millis() / 1000UL;
  const uint32_t hours = elapsed / 3600UL;
  const uint32_t minutes = (elapsed % 3600UL) / 60UL;
  const uint32_t seconds = elapsed % 60UL;
  lv_label_set_text_fmt(durationLabel_, "Duration %02lu:%02lu:%02lu",
                        static_cast<unsigned long>(hours),
                        static_cast<unsigned long>(minutes),
                        static_cast<unsigned long>(seconds));

  lv_label_set_text_fmt(recordLabel_, "Recording: %s",
                        ctx.state.gpxRecording ? "ON" : "OFF");
}

}  // namespace ui
