#ifndef UI_H
#define UI_H

#include "stm32f4xx_hal.h"
#include "input.h"

extern void ui_handle_input(struct input_evt *e);
extern void ui_init();
extern void ui_tick();

#endif
