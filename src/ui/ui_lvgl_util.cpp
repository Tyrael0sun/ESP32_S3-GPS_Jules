#include "ui_lvgl_util.h"

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
#include <esp_heap_caps.h>
#endif

#include "hardware/display_driver.h"

namespace ui {

namespace {
bool lvglReady = false;
lv_disp_t *display = nullptr;
lv_indev_t *encoder = nullptr;
lv_disp_draw_buf_t drawBuf;
lv_color_t *drawBuffer = nullptr;
}  // namespace

static void flushCallback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  auto &panel = hardware::display();
  panel.startWrite();
  const uint16_t w = area->x2 - area->x1 + 1;
  const uint16_t h = area->y2 - area->y1 + 1;
  panel.setAddrWindow(area->x1, area->y1, w, h);
  panel.pushPixels(reinterpret_cast<const uint16_t *>(color_p), w * h, true);
  panel.endWrite();
  lv_disp_flush_ready(drv);
}

void ensureLvglInitialized() {
  if (lvglReady) {
    return;
  }

  lv_init();

  const size_t bufPixels = 240 * 40;
#ifdef ARDUINO_ARCH_ESP32
  drawBuffer = static_cast<lv_color_t *>(heap_caps_malloc(sizeof(lv_color_t) * bufPixels, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
#endif
  if (!drawBuffer) {
    drawBuffer = static_cast<lv_color_t *>(malloc(sizeof(lv_color_t) * bufPixels));
  }
  if (!drawBuffer) {
    return;
  }
  lv_disp_draw_buf_init(&drawBuf, drawBuffer, nullptr, bufPixels);

  static lv_disp_drv_t dispDrv;
  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = 240;
  dispDrv.ver_res = 320;
  dispDrv.flush_cb = flushCallback;
  dispDrv.draw_buf = &drawBuf;
  display = lv_disp_drv_register(&dispDrv);
  lv_disp_set_rotation(display, LV_DISP_ROT_180);
  lv_disp_set_default(display);

  static lv_indev_drv_t indevDrv;
  lv_indev_drv_init(&indevDrv);
  indevDrv.type = LV_INDEV_TYPE_ENCODER;
  indevDrv.read_cb = [](lv_indev_drv_t *, lv_indev_data_t *data) {
    data->enc_diff = 0;
    data->state = LV_INDEV_STATE_REL;
  };
  encoder = lv_indev_drv_register(&indevDrv);

  lvglReady = true;
}

}  // namespace ui
