#ifndef INPUT_H
#define INPUT_H

#define NUM_ROWS 8
#define NUM_COLS 10
#define NUM_BUTTONS (NUM_ROWS*NUM_COLS)

extern void input_init();
extern void scan_buttons();
struct input_evt {
    uint8_t row;
    uint8_t col;
    uint8_t val;
};

#endif
