#pragma once

#include <lvgl.h>
#include "freertos/FreeRTOS.h"

namespace ui {

void ensureLvglInitialized();
bool lockLvgl(TickType_t timeout = portMAX_DELAY);
void unlockLvgl();

class LvglGuard {
 public:
	explicit LvglGuard(TickType_t timeout = portMAX_DELAY);
	~LvglGuard();
	bool locked() const { return locked_; }

 private:
	bool locked_ = false;
};

}  // namespace ui
