#include "stm32f4xx_hal.h"

typedef enum { VERTICAL = 0, _8x8, _1x64 } ui_mode;

typedef struct {
        uint8_t steps[8];
} chan_t;

typedef struct {
        chan_t chans[8];
} preset_t;

typedef struct {
        preset_t presets[8];
} group_t;

typedef struct {
        group_t groups[8];
        uint8_t mutes[8];
        uint8_t chan;       // selected chan in vertical mode
        uint8_t preset;     // currently playing preset
        ui_mode  mode;
} state_t;

inline uint8_t get_step(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i) {
        return  s->groups[group].presets[preset].chans[chan].steps[i];
}

inline void set_mode(state_t *s, ui_mode mode) {
	s->mode = mode;
}
