#include "stm32f4xx_hal.h"
#include "input.h"
#include "leds.h"

uint8_t state[NUM_ROWS][NUM_COLS];
void ui_init() {
}

void ui_handle_input(struct input_evt *e) {
    if (e->val == 0) {
        state[e->row][e->col] = state[e->row][e->col] ? 0:1;
    }
    leds_set(e->row, e->col, state[e->row][e->col] ? red : off);
}
