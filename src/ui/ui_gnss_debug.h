#pragma once

#include <array>

#include "ui_common.h"
#include "ui_status_bar.h"

namespace ui {

class GnssDebugScreen : public UIScreen {
 public:
  void init() override;
  void render(const RenderContext &ctx) override;
  lv_obj_t *root() const override { return root_; }

 private:
  struct RowWidgets {
    lv_obj_t *container = nullptr;
    lv_obj_t *systemLabel = nullptr;
    lv_obj_t *idLabel = nullptr;
    lv_obj_t *cn0Label = nullptr;
    lv_obj_t *stateLabel = nullptr;
  };

  void updateRow(RowWidgets &row, const app::GnssSatellite &sat, bool highlight);
  const char *constellationToString(app::GnssConstellation constellation) const;
  const char *statusToString(const app::GnssSatellite &sat) const;
  void ensureScrollMode(const app::GnssSnapshot &snapshot);

  static constexpr lv_coord_t kRowHeight = 20;

  lv_obj_t *root_ = nullptr;
  lv_obj_t *content_ = nullptr;
  lv_obj_t *summaryContainer_ = nullptr;
  lv_obj_t *positionLabel_ = nullptr;
  lv_obj_t *altitudeLabel_ = nullptr;
  lv_obj_t *speedLabel_ = nullptr;
  lv_obj_t *statusLabel_ = nullptr;
  lv_obj_t *listContainer_ = nullptr;
  lv_obj_t *emptyLabel_ = nullptr;
  StatusBar statusBar_;
  std::array<RowWidgets, app::GnssSnapshot::kMaxSatellites> rows_{};
  bool scrollModeEnabled_ = false;
};

}  // namespace ui
