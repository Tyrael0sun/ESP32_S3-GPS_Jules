#pragma once

#include "ui_common.h"
#include "ui_status_bar.h"

namespace ui {

class PboxScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
  lv_obj_t *root() const override { return root_; }

 private:
  lv_obj_t *root_ = nullptr;
  lv_obj_t *speedLabel_ = nullptr;
  lv_obj_t *statusLabel_ = nullptr;
  lv_obj_t *timerLabel_ = nullptr;
  StatusBar statusBar_;
};

}  // namespace ui
