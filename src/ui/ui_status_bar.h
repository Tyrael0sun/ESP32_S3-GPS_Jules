#pragma once

#include <lvgl.h>

#include "app/app_state.h"

namespace ui {

class StatusBar {
 public:
  void init(lv_obj_t *parent);
  void update(const app::SystemState &state);
  lv_obj_t *container() const { return container_; }

 private:
  lv_obj_t *container_ = nullptr;
  lv_obj_t *satLabel_ = nullptr;
  lv_obj_t *sdLabel_ = nullptr;
  lv_obj_t *batteryLabel_ = nullptr;
};

}  // namespace ui
