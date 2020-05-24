#include "string.h"
#include "stm32f4xx_hal.h"
#include "input.h"
#include "leds.h"

#define WritePin(p, v) HAL_GPIO_WritePin(GPIOD, p, v);

#define BAR_LENGTH (12*8)

#define CLOCK_PIN GPIO_PIN_0
#define RESET_PIN GPIO_PIN_1
#define CHAN1_PIN GPIO_PIN_2

typedef struct {
    uint8_t row;
    uint8_t col;
} pos;

pos vertical_btn = { 7, 9 };
pos _8x8_btn = { 6, 9 };

// vertical mode btns
#define PRESET_COL 2
#define STEPS_COL 3
#define CHAN_COL 4
pos reset_btn = { 3, 7 };

typedef enum { VERTICAL = 0, _8x8 } ui_mode;

uint16_t tick = 0;
uint8_t reset = 0;

ui_mode mode = VERTICAL;
uint8_t chan = 0;
uint8_t preset = 0;
uint8_t steps[8][8][8]; // [preset][chan][step]

void vertical_mode_init() {
    mode = VERTICAL;

    leds_clear();
    leds_set(vertical_btn.row, vertical_btn.col, blue);
    leds_set(chan, CHAN_COL, green);
    leds_set(preset, PRESET_COL, green);
}

void ui_init() {
    tick = 0;

    vertical_mode_init();
}

void ui_handle_input(struct input_evt *e) {
    pos i = { e->row, e->col };
    if (e->val == 0) { // key release
        if (memcmp(&i, &vertical_btn, sizeof(pos)) == 0) {
            vertical_mode_init();
        }

        if (memcmp(&i, &_8x8_btn, sizeof(pos)) == 0) {
            mode = _8x8;
            leds_clear();
            leds_set(_8x8_btn.row, _8x8_btn.col, blue);
        }

        if (mode == VERTICAL) {
            if (memcmp(&i, &reset_btn, sizeof(pos)) == 0) {
                reset = 1;
                leds_set(reset_btn.row, reset_btn.col, red);
            }

            if (e->col == STEPS_COL) {
                steps[preset][chan][e->row] ^= 1;
                leds_set(e->row, e->col, steps[preset][chan][e->row] ? green : off);
            }

            if (e->col == CHAN_COL) {
                leds_set(chan, CHAN_COL, off);
                chan = e->row;
                for (int i = 0; i < 8; i++)
                    leds_set(i, STEPS_COL, steps[preset][chan][i] ? green : off);
                leds_set(chan, CHAN_COL, green);
            }

            if (e->col == PRESET_COL) {
                leds_set(preset, PRESET_COL, off);
                preset = e->row;
                for (int i = 0; i < 8; i++)
                    leds_set(i, STEPS_COL, steps[preset][chan][i] ? green : off);
                leds_set(preset, PRESET_COL, green);
            }
        }
    }
}

void ui_tick() {
    if (reset == 1) {
        WritePin(RESET_PIN, 1);
        tick = 0;
        reset = 2;
    } else if (reset == 2) {
        reset = 0;
        WritePin(RESET_PIN, 0);
        leds_set(reset_btn.row, reset_btn.col, off);
    }

    if (steps[preset][0][tick/12] && (tick % 12) == 0) {
        WritePin(CHAN1_PIN, 1);
    } else {
        WritePin(CHAN1_PIN, 0);
    }

    // cursor
    if (mode == VERTICAL) {
        for (int i = 0; i < 8; i++) {
            fb[LED_INDEX(i, STEPS_COL)*3] = 0;
        }
        fb[LED_INDEX(tick/12, STEPS_COL)*3] = 70;
    }

    tick = (tick + 1) % BAR_LENGTH;
}
