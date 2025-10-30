#include "ui_status_bar.h"

#include <Arduino.h>

#include "config.h"
#include "hardware/storage_driver.h"

namespace ui {

void StatusBar::init(lv_obj_t *parent) {
  if (container_) {
    return;
  }

  container_ = lv_obj_create(parent);
  lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
  const lv_coord_t width = lv_disp_get_hor_res(nullptr);
  lv_obj_set_size(container_, width, 32);
  lv_obj_set_pos(container_, 0, 0);
  lv_obj_align(container_, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(container_, lv_color_hex(0x202020), 0);
  lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(container_, 0, 0);
  lv_obj_set_style_pad_all(container_, 6, 0);

  satLabel_ = lv_label_create(container_);
  lv_obj_set_style_text_color(satLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(satLabel_, "GNSS 0 sats");
  lv_obj_align(satLabel_, LV_ALIGN_LEFT_MID, 0, 0);

  sdLabel_ = lv_label_create(container_);
  lv_obj_set_style_text_color(sdLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(sdLabel_, "SD --");
  lv_obj_align(sdLabel_, LV_ALIGN_CENTER, 0, 0);

  batteryLabel_ = lv_label_create(container_);
  lv_obj_set_style_text_color(batteryLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(batteryLabel_, "Batt --");
  lv_obj_align(batteryLabel_, LV_ALIGN_RIGHT_MID, 0, 0);
}

void StatusBar::update(const app::SystemState &state) {
  if (!container_) {
    return;
  }

  const uint32_t satColor = state.gnss.fix ? config::kSatCountLockedColor : config::kSatCountSearchingColor;
  lv_obj_set_style_text_color(satLabel_, lv_color_hex(satColor), 0);
  lv_label_set_text_fmt(satLabel_, "GNSS %u", state.gnss.satellites);

  const bool sdReady = hardware::storageReady();
  lv_label_set_text(sdLabel_, sdReady ? "SD OK" : "SD --");

  const bool charging = state.battery.charging;
  lv_label_set_text_fmt(batteryLabel_, "%s %.0f%%",
                        charging ? LV_SYMBOL_CHARGE : LV_SYMBOL_BATTERY_FULL,
                        state.battery.levelPercent);
}

}  // namespace ui
