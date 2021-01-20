#include "stm32f4xx_hal.h"
#include "seq_state.h"

inline uint8_t get_step(state_t *s, uint8_t i) {
    return  s->groups[s->group].presets[s->preset].chans[s->chan].steps[i];
}

inline uint8_t get_step_v(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i) {
    return  s->groups[group].presets[preset].chans[chan].steps[i];
}

inline void set_step_v(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i, uint8_t v) {
    s->groups[group].presets[preset].chans[chan].steps[i] = v;
}

inline void toggle_step_v(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i) {
    s->groups[group].presets[preset].chans[chan].steps[i] ^= 1;
}

inline void toggle_step(state_t *s, uint8_t i) {
    s->groups[s->group].presets[s->preset].chans[s->chan].steps[i] ^= 1;
}

void seq_state_init(state_t *s) {
}