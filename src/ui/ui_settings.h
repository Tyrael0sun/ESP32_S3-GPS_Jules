#pragma once

#include "ui_common.h"

namespace ui {

class SettingsScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
};

}  // namespace ui
