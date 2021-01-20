#include "string.h"
#include "stm32f4xx_hal.h"
#include "input.h"
#include "leds.h"

#define WritePin(p, v) HAL_GPIO_WritePin(GPIOD, p, v);

#define BAR_LENGTH (12*8)

#define CLOCK_PIN GPIO_PIN_0
#define RESET_PIN GPIO_PIN_1

uint16_t chan_pin[8] = { GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11 };

typedef struct {
    uint8_t row;
    uint8_t col;
} pos;

pos vertical_btn = { 7, 9 };
pos _8x8_btn = { 6, 9 };
pos _1x64_btn = { 3, 9 };

// vertical mode btns
#define GROUP_COL 2
#define PRESET_COL 2
#define STEPS_COL 3
#define CHAN_COL 4
#define MUTE_COL 5
pos reset_btn = { 3, 7 };
pos pause_btn = { 7, 8 };
pos preset_loop_btn = { 0, 8 };

typedef enum { VERTICAL = 0, _8x8, _1x64 } ui_mode;

uint16_t tick = 0;
uint8_t reset = 0;
uint8_t pause = 0;
uint8_t preset_loop = 0;

ui_mode mode = VERTICAL;
uint8_t chan = 0;
uint8_t preset = 0;
// TODO don't hardcode 8
uint8_t steps[8][8][8]; // [preset][chan][step]
uint8_t mutes[8]; // [chan]

void render_steps_vertical() {
    for (int i = 0; i < 8; i++)
        leds_set(i, STEPS_COL, steps[preset][chan][i] ? green : off);
}

void vertical_mode_init() {
    mode = VERTICAL;

    leds_clear();
    leds_set(vertical_btn.row, vertical_btn.col, blue);
    leds_set(chan, CHAN_COL, green);
    leds_set(preset, PRESET_COL, green);
    render_steps_vertical();
    for (int i = 0; i < 8; i++)
        leds_set(i, MUTE_COL, mutes[i] ? green : off);
    leds_set(preset_loop_btn.row, preset_loop_btn.col, preset_loop ? green : off);
    leds_set(pause_btn.row, pause_btn.col, pause ? red : green);
}

void _1x64_mode_init() {
    mode = _1x64;
    leds_clear();
    leds_set(_1x64_btn.row, _1x64_btn.col, blue);

    for (int row = 0; row < 8; row++)  {
        for (int col = 0; col < 8; col++)  {
            leds_set(row, col + 1, steps[row][chan][col] ? green : off);
        }
    }
}

void _8x8_mode_init() {
    mode = _8x8;
    leds_clear();
    leds_set(_8x8_btn.row, _8x8_btn.col, blue);

    for (int c = 0; c < 8; c++) {
        for (int s = 0; s < 8; s++) {
            leds_set(c, s + 1, steps[preset][c][s] ? green : off);
        }
    }
}

void ui_init() {
    tick = 0;

    vertical_mode_init();
}

void ui_handle_input(struct input_evt *e) {
    pos i = { e->row, e->col };
#define IS(a) (memcmp(&i, &a, sizeof(pos)) == 0)
    if (e->val == 0) { // key release
        if (IS(vertical_btn)) {
            vertical_mode_init();
        }

        if (memcmp(&i, &_8x8_btn, sizeof(pos)) == 0) {
            _8x8_mode_init();
        }

        if (memcmp(&i, &_1x64_btn, sizeof(pos)) == 0) {
            _1x64_mode_init();
        }

        if (mode == VERTICAL) {
            if (memcmp(&i, &reset_btn, sizeof(pos)) == 0) {
                reset = 1;
                leds_set(reset_btn.row, reset_btn.col, green);
            }

            if (memcmp(&i, &preset_loop_btn, sizeof(pos)) == 0) {
                preset_loop ^= 1;
                leds_set(preset_loop_btn.row, preset_loop_btn.col, preset_loop ? green : off);
            }

            if (memcmp(&i, &pause_btn, sizeof(pos)) == 0) {
                pause ^= 1;
                leds_set(pause_btn.row, pause_btn.col, pause ? red : green);
            }

            if (e->col == STEPS_COL) {
                steps[preset][chan][e->row] ^= 1;
                leds_set(e->row, e->col, steps[preset][chan][e->row] ? green : off);
            }

            if (e->col == CHAN_COL) {
                leds_set(chan, CHAN_COL, off);
                chan = e->row;
                render_steps_vertical();
                leds_set(chan, CHAN_COL, green);
            }

            if (e->col == PRESET_COL) {
                leds_set(preset, PRESET_COL, off);
                preset = e->row;
                render_steps_vertical();
                leds_set(preset, PRESET_COL, green);
            }

            if (e->col == MUTE_COL) {
                mutes[e->row] ^= 1;
                leds_set(e->row, e->col, mutes[e->row] ? red : off);
            }

        } else if (mode == _8x8) {
            if (e->col >= 1 && e->col <= 8) {
                steps[preset][e->row][e->col - 1] ^= 1;
                leds_set(e->row, e->col, steps[preset][e->row][e->col -1] ? green : off);
            }
        } else if (mode == _1x64) {
            if (e->col >= 1 && e->col <= 8) {
                steps[e->row][chan][e->col - 1] ^= 1;
                leds_set(e->row, e->col, steps[e->row][chan][e->col -1] ? green : off);
            }
        }
    }
}

void ui_tick() {
    if (reset == 1) {
        WritePin(RESET_PIN, 1);
        if (mode == VERTICAL) leds_set(preset, PRESET_COL, off);

        if (preset_loop) preset = 0;

        if (mode == VERTICAL) {
            leds_set(preset, PRESET_COL, green);
            render_steps_vertical();
        } else if (mode == _8x8) {
            _8x8_mode_init();
        }
        tick = 0;
        reset = 2;
    } else if (reset == 2) {
        reset = 0;
        WritePin(RESET_PIN, 0);
        leds_set(reset_btn.row, reset_btn.col, off);
    }

    for (int c = 0; c < 8; c++) {
        if (mutes[c] == 0 && steps[preset][c][tick/12] && (tick % 12) == 0) {
            WritePin(chan_pin[c], 1);
        } else {
            WritePin(chan_pin[c], 0);
        }
    }

    // cursor
    if (mode == VERTICAL) {
        for (int i = 0; i < 8; i++) {
            fb[LED_INDEX(i, STEPS_COL)*3] = 0;
        }
        fb[LED_INDEX(tick/12, STEPS_COL)*3] = 70;
    } else if (mode == _8x8) {
        for (int s = 0; s < 8; s++) {
            uint8_t v = s == tick/12 ? 70 : 0;
            for (int r = 0; r < 8; r++) {
                fb[LED_INDEX(r, s + 1)*3] = v;
            }
        }
    } else if (mode == _1x64) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                uint8_t v = row == preset && col == tick/12 ? 70 : 0;
                fb[LED_INDEX(row, col + 1)*3] = v;
            }
        }
    }

    if (pause) return;
    tick = (tick + 1) % BAR_LENGTH;
    if (tick == 0 && preset_loop) {
        if (mode == VERTICAL) leds_set(preset, PRESET_COL, off);
        preset = (preset + 1) % 8;
        if (mode == VERTICAL) {
            leds_set(preset, PRESET_COL, green);
            render_steps_vertical();
        } else if (mode == _8x8) {
            _8x8_mode_init();
        }
    }
}
