#pragma once

#include "ui_common.h"
#include "ui_status_bar.h"

namespace ui {

class LoggerScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
  lv_obj_t *root() const override { return root_; }

 private:
  lv_obj_t *root_ = nullptr;
  lv_obj_t *speedLabel_ = nullptr;
  lv_obj_t *altLabel_ = nullptr;
  lv_obj_t *durationLabel_ = nullptr;
  lv_obj_t *recordLabel_ = nullptr;
  StatusBar statusBar_;
};

}  // namespace ui
