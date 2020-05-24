#include "stm32f4xx_hal.h"
#include "input.h"
#include "leds.h"

#define BAR_LENGTH (12*8)

static uint8_t state[NUM_ROWS][NUM_COLS];
static uint16_t tick = 0;
static uint8_t reset = 0;

void ui_init() {
    tick = 0;
}

#define RESET_BUTTON(row, col) ((col) == 1 && (row) == 1)
void ui_handle_input(struct input_evt *e) {
    if (e->val == 0 && e->col > 1) {
        state[e->row][e->col] = state[e->row][e->col] ? 0:1;
        leds_set(e->row, e->col, state[e->row][e->col] ? red : off);
    } if (e->val == 0 && RESET_BUTTON(e->row, e->col)) {
        reset = 1;
        leds_set(e->row, e->col, blue);
    }
}

void ui_tick() {
    if (reset == 1) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
        tick = 0;
        reset = 2;
    } else if (reset == 2) {
        reset = 0;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
        leds_set(1, 1, off);
    }


    if (state[6][2 + ((tick/12) % 8)] && (tick % 12) == 0) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    }

    for (int i = 0; i < 8; i++) {
        fb[LED_INDEX(6,2 + i)*3 + 1] = 0;
    }
    int i = (tick/12) % 8;
    fb[LED_INDEX(6,2 + i)*3 + 1] = 70;

    tick = (tick + 1) % BAR_LENGTH;
}
