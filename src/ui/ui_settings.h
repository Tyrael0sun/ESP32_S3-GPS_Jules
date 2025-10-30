#pragma once

#include "ui_common.h"
#include "ui_status_bar.h"

namespace ui {

class SettingsScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
  lv_obj_t *root() const override { return root_; }

 private:
  lv_obj_t *root_ = nullptr;
  lv_obj_t *titleLabel_ = nullptr;
  lv_obj_t *gnssLabel_ = nullptr;
  lv_obj_t *pboxLabel_ = nullptr;
  StatusBar statusBar_;
};

}  // namespace ui
