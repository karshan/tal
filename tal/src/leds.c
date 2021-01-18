#include "string.h"
#include "stm32f4xx_hal.h"
#include "ws2812b/ws2812b.h"
#include "leds.h"

rgb red = { 70, 0, 0 };
rgb green = { 0, 70, 0 };
rgb blue = { 0, 0, 70 };
rgb off = { 0, 0, 0 };

uint8_t fb[3 * NUM_LEDS];

void leds_clear() {
    memset(fb, 0, 3 * NUM_LEDS);
}

inline void leds_set(uint8_t row, uint8_t col, rgb c) {
    uint8_t i = row * 2 + (col % 2) + (col/2) * 16;
    fb[i * 3] = c.r;
    fb[i * 3 + 1] = c.g;
    fb[i * 3 + 2] = c.b;
}

void leds_init() {
    ws2812b.item[0].frameBufferPointer = fb;
    ws2812b.item[0].frameBufferSize = sizeof(fb);
    ws2812b.item[0].channel = 0;
    ws2812b_init();

    for (int i = 0; i < 3 * NUM_LEDS; i++) {
        fb[i] = 0;
    }
}


