typedef state_t struct {
        group_t groups[8];
        uint8_t mutes[8];
        uint8_t chan;       // selected chan in vertical mode
        uint8_t preset;     // currently playing preset
        uimode  mode;
};

typedef group_t struct {
        preset_t presets[8];
}

typedef preset_t struct {
        chan_t chans[8];
}

typedef chan_t struct {
        uint8_t steps[8];
}

inline uint8_t get_step(state_t *s, uint8_t group, uint8_t preset, uint8_t chan, uint8_t i) {
        return  s->groups[group].presets[preset].chans[chan].step[i];
}
