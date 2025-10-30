#pragma once

#include "ui_common.h"
#include "ui_status_bar.h"

namespace ui {

class BikeComputerScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
  lv_obj_t *root() const override { return root_; }

 private:
  lv_obj_t *root_ = nullptr;
  lv_obj_t *content_ = nullptr;
  lv_obj_t *speedLabel_ = nullptr;
  lv_obj_t *altitudeLabel_ = nullptr;
  lv_obj_t *distanceLabel_ = nullptr;
  lv_obj_t *rideTimeLabel_ = nullptr;
  lv_obj_t *recordLabel_ = nullptr;
  lv_obj_t *recordIndicator_ = nullptr;
  StatusBar statusBar_;
};

}  // namespace ui
