#include "ui_gnss_debug.h"

#include <algorithm>

#include "ui_lvgl_util.h"

namespace ui {

void GnssDebugScreen::init() {
  if (root_) {
    return;
  }

  ensureLvglInitialized();
  const lv_coord_t width = lv_disp_get_hor_res(nullptr);
  const lv_coord_t height = lv_disp_get_ver_res(nullptr);

  root_ = lv_obj_create(nullptr);
  lv_obj_set_size(root_, width, height);
  lv_obj_set_style_bg_color(root_, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
  lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);

  statusBar_.init(root_);
  const lv_coord_t statusHeight = StatusBar::kHeight;
  const lv_coord_t contentHeight = height > statusHeight ? height - statusHeight : height;

  content_ = lv_obj_create(root_);
  lv_obj_set_size(content_, width, contentHeight);
  lv_obj_set_pos(content_, 0, statusHeight);
  lv_obj_set_style_bg_opa(content_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(content_, 0, 0);
  lv_obj_set_style_pad_all(content_, 6, 0);
  lv_obj_set_style_pad_gap(content_, 0, 0);
  lv_obj_clear_flag(content_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_layout(content_, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  summaryContainer_ = lv_obj_create(content_);
  lv_obj_set_style_bg_opa(summaryContainer_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(summaryContainer_, 0, 0);
  lv_obj_set_style_pad_all(summaryContainer_, 0, 0);
  lv_obj_set_style_pad_gap(summaryContainer_, 2, 0);
  lv_obj_set_width(summaryContainer_, LV_PCT(100));
  lv_obj_clear_flag(summaryContainer_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_layout(summaryContainer_, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(summaryContainer_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(summaryContainer_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  positionLabel_ = lv_label_create(summaryContainer_);
  lv_obj_set_style_text_color(positionLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(positionLabel_, &lv_font_montserrat_14, 0);
  lv_label_set_text(positionLabel_, "Lat --  Lon --");

  altitudeLabel_ = lv_label_create(summaryContainer_);
  lv_obj_set_style_text_color(altitudeLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(altitudeLabel_, &lv_font_montserrat_14, 0);
  lv_label_set_text(altitudeLabel_, "Alt -- m");

  speedLabel_ = lv_label_create(summaryContainer_);
  lv_obj_set_style_text_color(speedLabel_, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(speedLabel_, &lv_font_montserrat_14, 0);
  lv_label_set_text(speedLabel_, "Speed -- km/h");

  statusLabel_ = lv_label_create(summaryContainer_);
  lv_obj_set_style_text_color(statusLabel_, lv_color_hex(0xA0A0A0), 0);
  lv_obj_set_style_text_font(statusLabel_, &lv_font_montserrat_14, 0);
  lv_label_set_text(statusLabel_, "Fix: -- | Used 0 | Tracked 0");

  lv_obj_set_style_pad_bottom(summaryContainer_, 0, 0);

  listContainer_ = lv_obj_create(content_);
  lv_obj_set_style_bg_opa(listContainer_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(listContainer_, 0, 0);
  lv_obj_set_style_pad_all(listContainer_, 0, 0);
  lv_obj_set_style_pad_row(listContainer_, 1, 0);
  lv_obj_set_style_pad_column(listContainer_, 0, 0);
  lv_obj_set_width(listContainer_, LV_PCT(100));
  lv_obj_set_flex_grow(listContainer_, 1);
  lv_obj_set_layout(listContainer_, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(listContainer_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(listContainer_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(listContainer_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(listContainer_, LV_SCROLLBAR_MODE_AUTO);

  emptyLabel_ = lv_label_create(listContainer_);
  lv_obj_set_style_text_color(emptyLabel_, lv_color_hex(0x606060), 0);
  lv_obj_set_style_text_font(emptyLabel_, &lv_font_montserrat_14, 0);
  lv_label_set_text(emptyLabel_, "No satellite data");

  for (auto &row : rows_) {
    row.container = lv_obj_create(listContainer_);
    lv_obj_set_size(row.container, LV_PCT(100), kRowHeight);
    lv_obj_set_style_bg_opa(row.container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row.container, 0, 0);
    lv_obj_set_style_pad_all(row.container, 1, 0);
    lv_obj_set_style_pad_gap(row.container, 4, 0);
    lv_obj_clear_flag(row.container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(row.container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row.container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row.container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(row.container, LV_OBJ_FLAG_HIDDEN);

    row.systemLabel = lv_label_create(row.container);
    lv_obj_set_style_text_font(row.systemLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(row.systemLabel, lv_color_hex(0x87CEEB), 0);
    lv_label_set_text(row.systemLabel, "--");

    row.idLabel = lv_label_create(row.container);
    lv_obj_set_style_text_font(row.idLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(row.idLabel, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(row.idLabel, "--");

    row.cn0Label = lv_label_create(row.container);
    lv_obj_set_style_text_font(row.cn0Label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(row.cn0Label, lv_color_hex(0xFFD700), 0);
    lv_label_set_text(row.cn0Label, "-- dB");

    row.stateLabel = lv_label_create(row.container);
    lv_obj_set_style_text_font(row.stateLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(row.stateLabel, lv_color_hex(0xA0FFA0), 0);
    lv_label_set_text(row.stateLabel, "--");
  }
}

void GnssDebugScreen::render(const RenderContext &ctx) {
  if (!root_) {
    return;
  }

  statusBar_.update(ctx.state);
  const auto &gnss = ctx.state.gnss;

  if (positionLabel_) {
    if (gnss.fix) {
      lv_label_set_text_fmt(positionLabel_, "Lat %.6f  Lon %.6f", gnss.latitude, gnss.longitude);
    } else {
      lv_label_set_text(positionLabel_, "Lat --  Lon --");
    }
  }

  if (altitudeLabel_) {
    if (gnss.altitudeValid) {
      lv_label_set_text_fmt(altitudeLabel_, "Alt %.1f m", gnss.altitudeM);
    } else {
      lv_label_set_text(altitudeLabel_, "Alt -- m");
    }
  }

  if (speedLabel_) {
    lv_label_set_text_fmt(speedLabel_, "Speed %.1f km/h", gnss.speedKmh);
  }

  size_t usedCount = 0;
  size_t trackedCount = 0;
  for (size_t i = 0; i < gnss.satelliteCount; ++i) {
    const auto &sat = gnss.satelliteList[i];
    usedCount += sat.usedForFix ? 1 : 0;
    trackedCount += sat.tracked ? 1 : 0;
  }

  if (statusLabel_) {
    lv_label_set_text_fmt(statusLabel_, "Fix: %s | Used %u | Tracked %u",
                          gnss.fix ? "YES" : "NO",
                          static_cast<unsigned>(usedCount),
                          static_cast<unsigned>(trackedCount));
  }

  ensureScrollMode(gnss);

  if (gnss.satelliteCount == 0) {
    lv_obj_set_style_pad_top(listContainer_, 4, 0);
    lv_obj_clear_flag(emptyLabel_, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_set_style_pad_top(listContainer_, 0, 0);
    lv_obj_add_flag(emptyLabel_, LV_OBJ_FLAG_HIDDEN);
  }

  std::array<size_t, app::GnssSnapshot::kMaxSatellites> order{};
  const size_t sortedCount = gnss.satelliteCount;
  for (size_t i = 0; i < sortedCount; ++i) {
    order[i] = i;
  }

  const auto statusRank = [](const app::GnssSatellite &sat) {
    if (sat.usedForFix) {
      return 0;
    }
    if (sat.tracked) {
      return 1;
    }
    return 2;
  };

  std::sort(order.begin(), order.begin() + sortedCount,
            [&](size_t lhs, size_t rhs) {
              const auto &a = gnss.satelliteList[lhs];
              const auto &b = gnss.satelliteList[rhs];
              const int rankA = statusRank(a);
              const int rankB = statusRank(b);
              if (rankA != rankB) {
                return rankA < rankB;
              }
              if (a.cn0 != b.cn0) {
                return a.cn0 > b.cn0;
              }
              return a.nmeaId < b.nmeaId;
            });

  for (size_t i = 0; i < rows_.size(); ++i) {
    auto &row = rows_[i];
    if (!row.container) {
      continue;
    }
    if (i < sortedCount) {
      lv_obj_clear_flag(row.container, LV_OBJ_FLAG_HIDDEN);
      const bool highlight = gnss.scrollMode && static_cast<int>(i) == gnss.scrollRow;
      updateRow(row, gnss.satelliteList[order[i]], highlight);
    } else {
      lv_obj_add_flag(row.container, LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (gnss.satelliteCount > 0) {
    const lv_coord_t target = static_cast<lv_coord_t>(gnss.scrollRow) * kRowHeight;
    lv_obj_scroll_to_y(listContainer_, target, LV_ANIM_OFF);
  } else {
    lv_obj_scroll_to_y(listContainer_, 0, LV_ANIM_OFF);
  }
}

void GnssDebugScreen::updateRow(RowWidgets &row, const app::GnssSatellite &sat, bool highlight) {
  if (!row.container) {
    return;
  }

  lv_obj_set_style_bg_color(row.container,
                            highlight ? lv_color_hex(0x203A6B) : lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(row.container, highlight ? LV_OPA_40 : LV_OPA_TRANSP, 0);

  if (row.systemLabel) {
    lv_label_set_text(row.systemLabel, constellationToString(sat.constellation));
  }
  if (row.idLabel) {
    lv_label_set_text_fmt(row.idLabel, "#%02u", static_cast<unsigned>(sat.nmeaId));
  }
  if (row.cn0Label) {
    if (sat.cn0 > 0) {
      lv_label_set_text_fmt(row.cn0Label, "%u dB", static_cast<unsigned>(sat.cn0));
    } else {
      lv_label_set_text(row.cn0Label, "-- dB");
    }
  }
  if (row.stateLabel) {
    lv_label_set_text(row.stateLabel, statusToString(sat));
  }
}

const char *GnssDebugScreen::constellationToString(app::GnssConstellation constellation) const {
  switch (constellation) {
    case app::GnssConstellation::GPS:
      return "GPS";
    case app::GnssConstellation::GLONASS:
      return "GLON";
    case app::GnssConstellation::Galileo:
      return "GAL";
    case app::GnssConstellation::BeiDou:
      return "BDS";
    case app::GnssConstellation::QZSS:
      return "QZSS";
    case app::GnssConstellation::SBAS:
      return "SBAS";
    case app::GnssConstellation::Mixed:
      return "MIX";
    case app::GnssConstellation::Unknown:
    default:
      return "UNK";
  }
}

const char *GnssDebugScreen::statusToString(const app::GnssSatellite &sat) const {
  if (sat.usedForFix) {
    return "USED";
  }
  if (sat.tracked) {
    return "TRACK";
  }
  return "SEARCH";
}

void GnssDebugScreen::ensureScrollMode(const app::GnssSnapshot &snapshot) {
  if (!listContainer_) {
    return;
  }

  const bool desired = snapshot.scrollMode && snapshot.satelliteCount > 0;
  if (desired == scrollModeEnabled_) {
    return;
  }

  scrollModeEnabled_ = desired;
  if (desired) {
    lv_obj_add_flag(listContainer_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(listContainer_, LV_DIR_VER);
  } else {
    lv_obj_clear_flag(listContainer_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to_y(listContainer_, 0, LV_ANIM_OFF);
  }
}

}  // namespace ui
