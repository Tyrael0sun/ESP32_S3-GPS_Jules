#pragma once

#include <Arduino.h>

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
};

}  // namespace ui
