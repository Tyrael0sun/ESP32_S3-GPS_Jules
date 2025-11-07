#include "ui_settings.h"

#include <Arduino.h>

#include "config.h"
#include "ui_status_bar.h"
#include "ui_lvgl_util.h"

namespace ui {

void SettingsScreen::init() {
  if (root_) {
    return;
  }

  ensureLvglInitialized();
  const lv_coord_t width = lv_disp_get_hor_res(nullptr);
  const lv_coord_t height = lv_disp_get_ver_res(nullptr);
  root_ = lv_obj_create(nullptr);
  lv_obj_set_size(root_, width, height);
  lv_obj_set_style_bg_color(root_, lv_color_hex(0x101820), 0);
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
  lv_obj_set_style_pad_all(content, 14, 0);
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

  titleLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(titleLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text(titleLabel_, "Settings");
  lv_obj_align(titleLabel_, LV_ALIGN_TOP_LEFT, 0, 20);

  gnssLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(gnssLabel_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(gnssLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text_fmt(gnssLabel_, "GNSS Rate: %u Hz", config::kDefaultGnssRateHz);
  lv_obj_align_to(gnssLabel_, titleLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  pboxLabel_ = lv_label_create(content);
  lv_obj_set_style_text_font(pboxLabel_, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(pboxLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_label_set_text_fmt(pboxLabel_, "P-Box Trigger: %.2f g", config::kPboxStartAccelG);
  lv_obj_align_to(pboxLabel_, gnssLabel_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
}

void SettingsScreen::render(const RenderContext &ctx) {
  if (!root_) {
    return;
  }

  statusBar_.update(ctx.state);
}

}  // namespace ui
