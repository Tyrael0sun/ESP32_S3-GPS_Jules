#include "display_driver.h"

#include <Arduino.h>

#include "pin_config.h"

namespace {
class DisplayDevice : public lgfx::LGFX_Device {
 public:
  DisplayDevice() {
    auto busCfg = _bus_instance.config();
    busCfg.spi_host = SPI3_HOST;
    busCfg.spi_mode = 0;
    busCfg.freq_write = 40000000;
    busCfg.freq_read = 16000000;
    busCfg.dma_channel = 1;
    busCfg.pin_sclk = pins::kDispSck;
    busCfg.pin_mosi = pins::kDispMosi;
    busCfg.pin_miso = -1;
    busCfg.pin_dc = pins::kDispDc;
    _bus_instance.config(busCfg);
    _panel_instance.setBus(&_bus_instance);

    auto panelCfg = _panel_instance.config();
    panelCfg.pin_cs = pins::kDispCs;
    panelCfg.pin_rst = pins::kDispRst;
    panelCfg.pin_busy = -1;
    panelCfg.panel_width = 240;
    panelCfg.panel_height = 320;
    panelCfg.offset_x = 0;
    panelCfg.offset_y = 0;
    panelCfg.offset_rotation = 2;
    panelCfg.memory_width = 240;
    panelCfg.memory_height = 320;
    panelCfg.readable = false;
    panelCfg.invert = true;
    panelCfg.rgb_order = false;
    panelCfg.dlen_16bit = false;
    panelCfg.bus_shared = true;
    _panel_instance.config(panelCfg);

    auto lightCfg = _light_instance.config();
    lightCfg.pin_bl = pins::kDispBl;
    lightCfg.invert = false;
    lightCfg.freq = 2000;
    lightCfg.pwm_channel = 7;
    _light_instance.config(lightCfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
  }

 private:
  lgfx::Bus_SPI _bus_instance;
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Light_PWM _light_instance;
};

DisplayDevice g_display;
}  // namespace

namespace hardware {

lgfx::LGFX_Device &display() {
  return g_display;
}

void initDisplay() {
  g_display.init();
  g_display.setRotation(2);
  g_display.setSwapBytes(true);
  setBacklight(0.5f);
}

void setBacklight(float percent) {
  const float clamped = constrain(percent, 0.0f, 1.0f);
  g_display.setBrightness(static_cast<uint8_t>(clamped * 255.0f));
}

}  // namespace hardware
