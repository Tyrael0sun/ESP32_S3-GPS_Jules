#include "ui_bike_computer.h"

#include <Arduino.h>

#include "ui_status_bar.h"
#include "ui_lvgl_util.h"

namespace ui {

void BikeComputerScreen::init() {
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
  const lv_coord_t statusHeight = StatusBar::kHeight;
  const lv_coord_t contentHeight = height > statusHeight ? height - statusHeight : height;

  content_ = lv_obj_create(root_);
  lv_obj_set_size(content_, width, contentHeight);
  lv_obj_set_pos(content_, 0, statusHeight);
  lv_obj_set_style_bg_opa(content_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(content_, 0, 0);
  lv_obj_set_style_pad_all(content_, 12, 0);
  lv_obj_clear_flag(content_, LV_OBJ_FLAG_SCROLLABLE);

  speedLabel_ = lv_label_create(content_);
  lv_obj_set_style_text_font(speedLabel_, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(speedLabel_, lv_color_hex(0x00FF00), 0);
  lv_label_set_text(speedLabel_, "0.0 km/h");
  lv_obj_align(speedLabel_, LV_ALIGN_TOP_LEFT, 0, 10);

  altitudeLabel_ = lv_label_create(content_);
  lv_obj_set_style_text_font(altitudeLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(altitudeLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(altitudeLabel_, "Alt 0 m");
  lv_obj_align_to(altitudeLabel_, speedLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  distanceLabel_ = lv_label_create(content_);
  lv_obj_set_style_text_font(distanceLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(distanceLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(distanceLabel_, "Dist 0.00 km");
  lv_obj_align_to(distanceLabel_, altitudeLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  rideTimeLabel_ = lv_label_create(content_);
  lv_obj_set_style_text_font(rideTimeLabel_, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(rideTimeLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(rideTimeLabel_, "Time 00:00:00");
  lv_obj_align_to(rideTimeLabel_, distanceLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  recordLabel_ = lv_label_create(content_);
  lv_obj_set_style_text_font(recordLabel_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(recordLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(recordLabel_, "REC");
  lv_obj_align(recordLabel_, LV_ALIGN_BOTTOM_LEFT, 0, -10);

  recordIndicator_ = lv_obj_create(content_);
  lv_obj_set_size(recordIndicator_, 24, 24);
  lv_obj_align_to(recordIndicator_, recordLabel_, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_obj_set_style_radius(recordIndicator_, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(recordIndicator_, lv_color_hex(0x00FF00), 0);
  lv_obj_set_style_bg_opa(recordIndicator_, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(recordIndicator_, 0, 0);
}

void BikeComputerScreen::render(const RenderContext &ctx) {
  if (!root_) {
    return;
  }

  statusBar_.update(ctx.state);

  const auto &gnss = ctx.state.gnss;
  const auto &baro = ctx.state.baro;

  lv_label_set_text_fmt(speedLabel_, "%.1f km/h", gnss.speedKmh);
  lv_label_set_text_fmt(altitudeLabel_, "Alt %.0f m", baro.altitudeM);
  lv_label_set_text_fmt(distanceLabel_, "Dist %.2f km", 0.0f);

  const uint32_t elapsed = millis() / 1000UL;
  const uint32_t hours = elapsed / 3600UL;
  const uint32_t minutes = (elapsed % 3600UL) / 60UL;
  const uint32_t seconds = elapsed % 60UL;
  lv_label_set_text_fmt(rideTimeLabel_, "Time %02lu:%02lu:%02lu",
                        static_cast<unsigned long>(hours),
                        static_cast<unsigned long>(minutes),
                        static_cast<unsigned long>(seconds));

  const bool recording = ctx.state.gpxRecording;
  lv_obj_set_style_bg_color(recordIndicator_, recording ? lv_color_hex(0xFF0000) : lv_color_hex(0x00FF00), 0);
}

}  // namespace ui
