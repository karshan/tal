#ifndef STATE_H
#define STATE_H

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
    uint8_t group;      // currently playing group
    ui_mode  mode;
} state_t;

uint8_t get_step(state_t *s, uint8_t i);
uint8_t get_step_v(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i);
void seq_state_init(state_t *s);

void toggle_step(state_t *s, uint8_t i);
void toggle_step_v(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i);

#endif // STATE_H
