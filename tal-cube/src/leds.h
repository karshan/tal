#ifndef LEDS_H
#define LEDS_H

#define NUM_LEDS 64

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

extern rgb red;
extern rgb blue;
extern rgb off;
extern void leds_init();
extern void leds_set(uint8_t row, uint8_t col, rgb c);

#endif // LEDS_H
