#include "stm32f4xx_hal.h"
#include "input.h"
#include "leds.h"

#define output_port GPIOD
#define clock_pin GPIO_PIN_0

#define BAR_LENGTH (12*8)

static uint8_t state[NUM_ROWS][NUM_COLS];
static uint16_t tick = 0;

void ui_init() {
    tick = 0;
}

void ui_handle_input(struct input_evt *e) {
    if (e->val == 0) {
        state[e->row][e->col] = state[e->row][e->col] ? 0:1;
    }
    rgb on = e->col > 1 ? red : blue;
    leds_set(e->row, e->col, state[e->row][e->col] ? on : off);
}

void ui_tick() {
    HAL_GPIO_TogglePin(output_port, clock_pin);

    tick = (tick + 1) % BAR_LENGTH;

    if (state[6][2 + ((tick/12) % 8)] && (tick % 12) == 0) {
        HAL_GPIO_WritePin(output_port, GPIO_PIN_1, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(output_port, GPIO_PIN_1, GPIO_PIN_RESET);
    }

    for (int i = 0; i < 8; i++) {
        fb[LED_INDEX(6,2 + i)*3 + 1] = 0;
    }
    int i = (tick/12) % 8;
    fb[LED_INDEX(6,2 + i)*3 + 1] = 70;
}
