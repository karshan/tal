#ifndef LEDS_H
#define LEDS_H

#define NUM_LEDS 80
#define LED_INDEX(row, col) ((row) * 2 + ((col) % 2) + ((col)/2) * 16)

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

extern rgb red;
extern rgb green;
extern rgb blue;
extern rgb off;
extern void leds_init();
extern void leds_set(uint8_t row, uint8_t col, rgb c);
extern uint8_t fb[3 * NUM_LEDS];

#endif // LEDS_H
