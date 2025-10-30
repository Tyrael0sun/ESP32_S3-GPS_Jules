#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "app/app_state.h"

namespace ui {

struct RenderContext {
  const app::SystemState &state;
};

class UIScreen {
 public:
  virtual ~UIScreen() = default;
  virtual void init() = 0;
  virtual void render(const RenderContext &ctx) = 0;
  virtual lv_obj_t *root() const = 0;
};

}  // namespace ui
