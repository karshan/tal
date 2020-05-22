#include "stm32f4xx_hal.h"
#include "input.h"
#include "util.h"

uint8_t buttons[NUM_BUTTONS/8 + (NUM_BUTTONS % 8 ? 1 : 0)];

#define rowPort GPIOA
uint16_t rowPins[NUM_ROWS] = { GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4 };

#define colPort GPIOB
uint16_t colPins[NUM_COLS] = { GPIO_PIN_0, GPIO_PIN_1 };

void (*cb)(struct input_evt *);

void input_init(void (*cb_init)(struct input_evt *)) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    init_pin(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    init_pin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    cb = cb_init;
}

void set_button_state(int row, int col, int s) {
    int i = (col * NUM_ROWS) + row;
    uint8_t mask = 1 << (i % 8);
    uint8_t current = (buttons[i / 8] & mask) >> (i % 8);
    if (current != s) {
        if (s) {
            buttons[i/8] |= (1 << (i % 8));
        } else {
            buttons[i/8] &= ~(1 << (i % 8));
        }
        struct input_evt e = { .row = row, .col = col, .val = s };
        cb(&e);
    }
}

uint32_t last_scan = 0;
void scan_buttons() {
    if (HAL_GetTick() <= last_scan + 100) return;
    last_scan = HAL_GetTick();
    for (int i = 0; i < NUM_COLS; i++) {
        HAL_GPIO_WritePin(colPort, colPins[i], GPIO_PIN_RESET);
    }

    for (int i = 0; i < NUM_COLS; i++) {
        HAL_GPIO_WritePin(colPort, colPins[i], GPIO_PIN_SET);
        HAL_Delay(2);
        for (int j = 0; j < NUM_ROWS; j++) {
            if (HAL_GPIO_ReadPin(rowPort, rowPins[j]) == GPIO_PIN_SET) {
                set_button_state(j, i, 1);
            } else {
                set_button_state(j, i, 0);
            }
        }
        HAL_GPIO_WritePin(colPort, colPins[i], GPIO_PIN_RESET);
    }
}
