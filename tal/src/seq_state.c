#include "string.h"
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

inline void copy_chan(state_t *s, uint8_t dest, uint8_t src) {
    memcpy(s->groups[s->group].presets[s->preset].chans[dest].steps,
            s->groups[s->copy.group].presets[s->copy.preset].chans[src].steps,
            sizeof(chan_t));
}

inline void copy_preset(state_t *s, uint8_t dest, uint8_t src) {
    memcpy(s->groups[s->group].presets[s->preset].chans,
            s->groups[s->copy.group].presets[s->copy.preset].chans,
            sizeof(preset_t));
}

inline void copy_group(state_t *s, uint8_t dest, uint8_t src) {
    memcpy(s->groups[s->group].presets,
            s->groups[s->copy.group].presets,
            sizeof(group_t));
}

inline void clear_chan(state_t *s, uint8_t i) {
    memset(s->groups[s->group].presets[s->preset].chans[i].steps,
            0,
            sizeof(chan_t));
}

inline void clear_preset(state_t *s, uint8_t i) {
    memset(s->groups[s->group].presets[i].chans,
            0,
            sizeof(preset_t));
}

inline void clear_group(state_t *s, uint8_t i) {
    memset(s->groups[i].presets,
            0,
            sizeof(group_t));
}

void seq_state_init(state_t *s) {
    // since mem is inited to 0 this is unnecessary
    copy_state_t init_copy = { CP_NO, 0, CP_CHAN, 0 };
    s->copy = init_copy;
    s->set_loop = 0;
}
