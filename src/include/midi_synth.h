#ifndef UAE_MIDI_SYNTH_H
#define UAE_MIDI_SYNTH_H

#include <stdint.h>

#if defined(__PSP2__) || defined(__SWITCH__)

#ifdef __cplusplus
extern "C" {
#endif

void midi_synth_init(void);
void midi_synth_reset(void);
void midi_synth_set_enabled(int enabled);
int  midi_synth_enabled(void);
void midi_synth_all_notes_off(void);
void midi_synth_feed_byte(uint8_t byte);
void midi_synth_mix(int16_t *samples, int frames, int channels, int output_rate);

#ifdef __cplusplus
}
#endif

#endif

#endif